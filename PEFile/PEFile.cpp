#include <stddef.h>
#include "PEFile.h"

#define VALID_OFFSET(offset, len) \
    ((offset) < GetTotalSize() && \
     ((len) == 0 || \
      (offset) + (len) <= GetTotalSize()))

typedef VOID(*PFNExeEntry)();

typedef BOOL(WINAPI *PFNDllEntry)(
    HINSTANCE hinstDLL,
    DWORD fdwReason,
    LPVOID lpvReserved);

CPEFile::CPEFile()
    : m_pDosHeader(this)
    , m_pNtHeaders(this)
    , m_aryDataDirectory(this)
    , m_arySectionHeader(this)
{

}


CPEFile::CPEFile(LPCSTR pszFileName)
    : m_pDosHeader(this)
    , m_pNtHeaders(this)
    , m_aryDataDirectory(this)
    , m_arySectionHeader(this)
{
    SetFileName(pszFileName);
}


CPEFile::CPEFile(LPCWSTR pszFileName)
    : m_pDosHeader(this)
    , m_pNtHeaders(this)
    , m_aryDataDirectory(this)
    , m_arySectionHeader(this)
{
    SetFileName(pszFileName);
}


CPEFile::~CPEFile()
{

}

//加载文件
HMODULE CPEFile::Load(HMODULE hModule, bool bCallEntry)
{
    assert(!m_Buffer.IsEmpty());

    if (!ParseHeader()) //解析pe头失败
    {
        return NULL;
    }

    DWORD dwImageSize = m_pNtHeaders->OptionalHeader.SizeOfImage;

    //模块基址
    LPVOID lpImageBase = hModule;
    
    if (lpImageBase == NULL)
    {
        //默认地址为空就自己申请吧
        lpImageBase = VirtualAlloc(
            NULL,
            dwImageSize,
            MEM_COMMIT,
            PAGE_EXECUTE_READWRITE);
    }

    if (lpImageBase == NULL)
    {
        return NULL;
    }

    DWORD dwRet = 0;

    //修改内存属性
    BOOL bRet = VirtualProtect(
        lpImageBase,
        dwImageSize,
        PAGE_EXECUTE_READWRITE,
        &dwRet);

    if (!bRet)
    {
        return NULL;
    }

    //////////////////////////////////////////////////////////////////////////
    // 拷贝头部信息
    //////////////////////////////////////////////////////////////////////////
    memcpy(lpImageBase, m_pDosHeader, m_pNtHeaders->OptionalHeader.SizeOfHeaders);

    //////////////////////////////////////////////////////////////////////////
    // 拷贝节表信息
    //////////////////////////////////////////////////////////////////////////
    for (int i = 0; i < m_arySectionHeader.GetCount(); i++)
    {
        memcpy(
            (PBYTE)lpImageBase + m_arySectionHeader[i].VirtualAddress,
            GetBuffer(m_arySectionHeader[i].PointerToRawData),
            m_arySectionHeader[i].SizeOfRawData);
    }

    //////////////////////////////////////////////////////////////////////////
    // 填写iat
    //////////////////////////////////////////////////////////////////////////
    CImageImportDescriptorAry aryImport = ParseImportTable();

    if (aryImport != NULL)
    {
        //遍历导入表
        for (int i = 0; i < aryImport.GetCount() - 1; i++)
        {
            if (aryImport[i].Name == 0)
            {
                continue;
            }

            //加载指定模块
            HMODULE hModule = LoadLibraryA(
                (LPSTR)lpImageBase + aryImport[i].Name);

            if (hModule == NULL)
            {
                continue;
            }

            //这个就是地址表的数组
            CImageThunkDataAry aryThunk = GetThunkAry(aryImport[i].FirstThunk);

            //地址表的数组首地址
            DWORD_PTR *pThunk = (DWORD_PTR*)
                ((PBYTE)lpImageBase + OffsetToRva(aryThunk.GetOffset()));

            //判断是要FirstThunk还是OriginalFirstThunk
            int nFirstThunk = aryImport[i].OriginalFirstThunk == 0 ?
                aryImport[i].FirstThunk : aryImport[i].OriginalFirstThunk;;

            //这个就是要遍历的导入表数组了
            CImageThunkDataAry aryOrigThunk = GetThunkAry(nFirstThunk);

            //为填入每项IAT
            for (int j = 0; j < aryOrigThunk.GetCount() - 1; j++)
            {
                if (aryThunk[j].u1.AddressOfData != 0)
                {
                    CImageThunkData thunkData = aryOrigThunk.GetAt(j);

                    //拿到的是序号或者函数名
                    LPCSTR pszProcName = thunkData.IsOrdinal() ?
                        (LPCSTR)thunkData.GetOrdinal() :
                        (LPCSTR)thunkData.GetForwarderString()->Name;

                    //填入导入地址表内容
                    pThunk[j] = (DWORD_PTR)GetProcAddress(hModule, pszProcName);
                }
            }

            FreeLibrary(hModule);
        }
    }

    //////////////////////////////////////////////////////////////////////////
    // 填写重定位表
    //////////////////////////////////////////////////////////////////////////
    CImageBaseRelocationPtr pReloc = ParseBaseRelocation();

    if (pReloc != NULL)
    {
        while (!pReloc.IsEnd())
        {
            SIZE_T nOffset = (SIZE_T)lpImageBase - m_pNtHeaders->OptionalHeader.ImageBase;
            PBYTE pBase = (PBYTE)lpImageBase + pReloc->VirtualAddress;
            CRelocTypeOffsetAry aryOffset = pReloc.GetTypeOffsetAry();

            for (int i = 0; i < aryOffset.GetCount(); i++)
            {
                if (aryOffset[i].GetType() != 0)
                {
                    PBYTE* pAddr = (PBYTE*)(pBase + aryOffset[i].GetOffset());
                    *pAddr = *pAddr + nOffset;
                }
            }

            pReloc = pReloc.GetNext();
        }
    }

    //如果不需要执行EntryPoint则直接返回
    if (!bCallEntry)
    {
        return hModule;
    }

    //文件属性
    WORD wChar = m_pNtHeaders->FileHeader.Characteristics;

    if ((wChar & IMAGE_FILE_DLL) == IMAGE_FILE_DLL) //如果是dll
    {
        //////////////////////////////////////////////////////////////////////////
        // 调用DllMain
        //////////////////////////////////////////////////////////////////////////
        DWORD dwEntryPoint = m_pNtHeaders->OptionalHeader.AddressOfEntryPoint;

        if (dwEntryPoint != 0)
        {
            PFNDllEntry pfnEntry = (PFNDllEntry)((PBYTE)lpImageBase + dwEntryPoint);

            if (!pfnEntry((HINSTANCE)lpImageBase, DLL_PROCESS_ATTACH, NULL))
            {
                if (hModule == NULL) //说明是自己申请的空间，需要释放
                {
                    VirtualFree(lpImageBase, 0, MEM_RELEASE);
                    lpImageBase = NULL;
                }
            }
        }
    }
    else if ((wChar & IMAGE_FILE_EXECUTABLE_IMAGE) == IMAGE_FILE_EXECUTABLE_IMAGE) //如果是exe
    {
        //////////////////////////////////////////////////////////////////////////
        // 调用mainCRTStartup
        //////////////////////////////////////////////////////////////////////////
        DWORD dwEntryPoint = m_pNtHeaders->OptionalHeader.AddressOfEntryPoint;

        if (dwEntryPoint != 0)
        {
            ((PFNExeEntry)((PBYTE)lpImageBase + dwEntryPoint))();
        }
    }

    return (HMODULE)lpImageBase;
}

//读取文件数据并解析pe头
bool CPEFile::ParseFile()
{
    bool bRet = false;

    if ((IsOpen() || Open()) && Read() && ParseHeader())
    {
        bRet = true;
        Close();
    }

    return bRet;
}

//解析pe头
bool CPEFile::ParseHeader()
{
    assert(!m_Buffer.IsEmpty());

    int nCount = 0;
    size_t nOffset = 0;

    //设置NT头
    if (!VALID_OFFSET(nOffset, sizeof(IMAGE_DOS_HEADER)))
    {
        return false;
    }

    m_pDosHeader.SetOffset(nOffset);

    if (m_pDosHeader->e_magic != 0x5A4D)
    {
        return false;
    }

    //设置NT头
    nOffset = m_pDosHeader->e_lfanew;

    if (!VALID_OFFSET(nOffset, sizeof(IMAGE_NT_HEADERS)))
    {
        return false;
    }

    m_pNtHeaders.SetOffset(nOffset);

    if (m_pNtHeaders->Signature != 0x00004550)
    {
        return false;
    }

#ifndef _WIN64
    if (m_pNtHeaders->FileHeader.Machine != IMAGE_FILE_MACHINE_I386)
    {
        return false;
    }
#else
    if (m_pNtHeaders->FileHeader.Machine != IMAGE_FILE_MACHINE_IA64 &&
        m_pNtHeaders->FileHeader.Machine != IMAGE_FILE_MACHINE_AMD64)
    {
        return false;
    }
#endif

    //设置数据目录
    nOffset = m_pNtHeaders.GetOffset() +
        offsetof(IMAGE_NT_HEADERS, OptionalHeader.DataDirectory[0]);
    nCount = m_pNtHeaders->OptionalHeader.NumberOfRvaAndSizes;

    if (!VALID_OFFSET(nOffset, nCount * sizeof(IMAGE_DATA_DIRECTORY)))
    {
        return false;
    }

    m_aryDataDirectory.SetOffset(nOffset);
    m_aryDataDirectory.SetCount(nCount);

    //设置节表头
    nOffset = m_pNtHeaders.GetOffset() +
        offsetof(IMAGE_NT_HEADERS, OptionalHeader) +
        m_pNtHeaders->FileHeader.SizeOfOptionalHeader;
    nCount = m_pNtHeaders->FileHeader.NumberOfSections;

    if (!VALID_OFFSET(nOffset, nCount * sizeof(IMAGE_SECTION_HEADER)))
    {
        return false;
    }

    m_arySectionHeader.SetOffset(nOffset);
    m_arySectionHeader.SetCount(nCount);

    return true;
}

//添加节
bool CPEFile::AppendSection()
{
    assert(!m_Buffer.IsEmpty());

    IMAGE_SECTION_HEADER Header = { 0 };

    int nCount = m_arySectionHeader.GetCount();
    IMAGE_SECTION_HEADER *pHeader = m_arySectionHeader + nCount;

    //看看后面是不是全为0
    if (memcmp(pHeader, &Header, sizeof(IMAGE_SECTION_HEADER)) != 0)
    {
        return false;
    }

    //添加一项
    m_arySectionHeader.SetCount(nCount + 1);
    m_pNtHeaders->FileHeader.NumberOfSections = nCount + 1;

    //设置头部大小
    if (m_arySectionHeader.GetEndOffset() > 
        m_pNtHeaders->OptionalHeader.SizeOfHeaders)
    {
        m_pNtHeaders->OptionalHeader.SizeOfHeaders =
            (DWORD)m_arySectionHeader.GetEndOffset();
    }

    if (nCount == 0)
    {
        return true;
    }

    DWORD dwSectionAlignment = m_pNtHeaders->OptionalHeader.SectionAlignment;
    DWORD dwFileAlignment = m_pNtHeaders->OptionalHeader.FileAlignment;

    IMAGE_SECTION_HEADER *pLastHeader = &m_arySectionHeader[nCount - 1];

    //先对其文件大小
    DWORD dwAddr = pLastHeader->PointerToRawData + 
        pLastHeader->SizeOfRawData;
    DWORD dwSize = dwAddr % dwFileAlignment;
    if (dwSize != 0)
    {
        dwAddr = dwAddr - dwSize + dwFileAlignment;
    }

    pHeader->PointerToRawData = dwAddr;

    //对其节大小
    dwAddr = pLastHeader->VirtualAddress + 
        max(pLastHeader->SizeOfRawData, pLastHeader->Misc.VirtualSize);
    dwSize = dwAddr % dwSectionAlignment;
    if (dwSize != 0)
    {
        dwAddr = dwAddr - dwSize + dwSectionAlignment;
    }

    pHeader->VirtualAddress = dwAddr;

    return true;
}


//删除节
bool CPEFile::DeleteSection(int nIndex)
{
    assert(!m_Buffer.IsEmpty());

    if (nIndex < 0 || nIndex >= m_arySectionHeader.GetCount())
    {
        return false;
    }

    int nCount = m_arySectionHeader.GetCount() - 1;

    for (int i = nIndex; i < nCount; i++)
    {
        m_arySectionHeader[i] = m_arySectionHeader[i + 1];
    }

    memset(&m_arySectionHeader[nCount], 0, sizeof(IMAGE_SECTION_HEADER));

    m_arySectionHeader.SetCount(nCount);
    m_pNtHeaders->FileHeader.NumberOfSections = nCount;

    return true;
}


//解析导出表
CImageExportDirectoryPtr CPEFile::ParseExportTable()
{
    assert(!m_Buffer.IsEmpty());

    CImageExportDirectoryPtr pExport(this);

    //如果没有导入表项
    if (m_aryDataDirectory.GetCount() <= IMAGE_DIRECTORY_ENTRY_EXPORT)
    {
        return pExport;
    }

    //设置导出表偏移
    size_t nRva = m_aryDataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;

    if (nRva == 0)
    {
        return pExport;
    }

    size_t nOffset = RvaToOffset(nRva);

    if (!VALID_OFFSET(nOffset, 0))
    {
        return pExport;
    }

    pExport.SetOffset(nOffset);

    return pExport;
}


//导出函数地址
CDwordAry CPEFile::GetExportFunctions(PIMAGE_EXPORT_DIRECTORY pExport)
{
    assert(!m_Buffer.IsEmpty());

    CDwordAry aryFuctions(this);

    if (pExport == NULL)
    {
        return aryFuctions;
    }

    size_t nOffset = RvaToOffset(pExport->AddressOfFunctions);
    int nCount = pExport->NumberOfFunctions;

    if (!VALID_OFFSET(nOffset, nCount * sizeof(DWORD)))
    {
        return aryFuctions;
    }

    aryFuctions.SetOffset(nOffset);
    aryFuctions.SetCount(nCount);

    return aryFuctions;
}

//导出名
CDwordAry CPEFile::GetExportNames(PIMAGE_EXPORT_DIRECTORY pExport)
{
    assert(!m_Buffer.IsEmpty());

    CDwordAry aryNames(this);

    if (pExport == NULL)
    {
        return aryNames;
    }

    size_t nOffset = RvaToOffset(pExport->AddressOfNames);
    int nCount = pExport->NumberOfNames;

    if (!VALID_OFFSET(nOffset, nCount * sizeof(DWORD)))
    {
        return aryNames;
    }

    aryNames.SetOffset(nOffset);
    aryNames.SetCount(nCount);

    return aryNames;
}


//导出序号
CWordAry CPEFile::GetExportNameOrdinals(PIMAGE_EXPORT_DIRECTORY pExport)
{
    assert(!m_Buffer.IsEmpty());

    CWordAry aryNameOrdinals(this);

    if (pExport == NULL)
    {
        return aryNameOrdinals;
    }

    size_t nOffset = RvaToOffset(pExport->AddressOfNameOrdinals);
    int nCount = pExport->NumberOfNames;

    if (!VALID_OFFSET(nOffset, nCount * sizeof(DWORD)))
    {
        return aryNameOrdinals;
    }

    aryNameOrdinals.SetOffset(nOffset);
    aryNameOrdinals.SetCount(nCount);

    return aryNameOrdinals;
}


CImageImportDescriptorAry CPEFile::ParseImportTable()
{
    assert(!m_Buffer.IsEmpty());

    CImageImportDescriptorAry aryImport(this);

    //如果没有导入表项
    if (m_aryDataDirectory.GetCount() <= IMAGE_DIRECTORY_ENTRY_IMPORT)
    {
        return aryImport;
    }

    //设置导入表偏移
    size_t nRva = m_aryDataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;

    if (nRva == 0)
    {
        return aryImport;
    }

    size_t nOffset = RvaToOffset(nRva);

    if (!VALID_OFFSET(nOffset, 0))
    {
        return aryImport;
    }

    aryImport.SetOffset(nOffset);

    //设置导入表项数
    int nCount = 0;

    while (true)
    {
        if (!VALID_OFFSET(nOffset, sizeof(IMAGE_IMPORT_DESCRIPTOR)))
        {
            break;
        }

        if (aryImport[nCount++].FirstThunk == 0)
        {
            break;
        }

        nOffset += sizeof(IMAGE_IMPORT_DESCRIPTOR);
    }

    return aryImport;
}


CImageThunkDataAry CPEFile::GetThunkAry(size_t nFirstThunk)
{
    CImageThunkDataAry aryTrunkData(this);

    if (nFirstThunk == 0)
    {
        return aryTrunkData;
    }

    size_t nOffset = RvaToOffset(nFirstThunk);

    if (!VALID_OFFSET(nOffset, 0))
    {
        return aryTrunkData;
    }

    aryTrunkData.SetOffset(nOffset);

    int nCount = 0;

    while (true)
    {
        if (!VALID_OFFSET(nOffset, sizeof(IMAGE_THUNK_DATA)))
        {
            return aryTrunkData;
        }

        if (aryTrunkData[nCount++].u1.AddressOfData == 0)
        {
            return aryTrunkData;
        }

        nOffset += sizeof(IMAGE_THUNK_DATA);
    }
}


//解析重定位表
CImageBaseRelocationPtr CPEFile::ParseBaseRelocation()
{
    CImageBaseRelocationPtr pBaseReloc(this);

    //如果没有导入表项
    if (m_aryDataDirectory.GetCount() <= IMAGE_DIRECTORY_ENTRY_BASERELOC)
    {
        return pBaseReloc;
    }

    //设置导入表偏移
    size_t nRva = m_aryDataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress;

    if (nRva == 0)
    {
        return pBaseReloc;
    }

    size_t nOffset = RvaToOffset(nRva);

    if (!VALID_OFFSET(nOffset, sizeof(IMAGE_BASE_RELOCATION)))
    {
        return pBaseReloc;
    }

    pBaseReloc.SetOffset(nOffset);

    return pBaseReloc;
}


//获取重定位表偏移数组
CRelocTypeOffsetAry CPEFile::GetTypeOffsetAry(PIMAGE_BASE_RELOCATION pReloc)
{
    const int RELOC_SIZE = sizeof(IMAGE_BASE_RELOCATION);
    CRelocTypeOffsetAry aryOffset(this);

    aryOffset.SetOffset(GetOffset(pReloc) + RELOC_SIZE);
    aryOffset.SetCount((pReloc->SizeOfBlock - RELOC_SIZE) / sizeof(WORD));

    return aryOffset;
}


//解析Tls表
CImageTlsDirectoryPtr CPEFile::ParseTlsDirectory()
{
    CImageTlsDirectoryPtr pTls(this);

    //如果没有导入表项
    if (m_aryDataDirectory.GetCount() <= IMAGE_DIRECTORY_ENTRY_TLS)
    {
        return pTls;
    }

    //设置导入表偏移
    size_t nRva = m_aryDataDirectory[IMAGE_DIRECTORY_ENTRY_TLS].VirtualAddress;

    if (nRva == 0)
    {
        return pTls;
    }

    size_t nOffset = RvaToOffset(nRva);

    if (!VALID_OFFSET(nOffset, sizeof(IMAGE_TLS_DIRECTORY)))
    {
        return pTls;
    }

    pTls.SetOffset(nOffset);

    return pTls;
}


//解析资源目录
CImageResourceDirectoryPtr CPEFile::ParseResourceDirectory()
{
    CImageResourceDirectoryPtr pRes(this);

    //如果没有导入表项
    if (m_aryDataDirectory.GetCount() <= IMAGE_DIRECTORY_ENTRY_RESOURCE)
    {
        return pRes;
    }

    //设置导入表偏移
    size_t nRva = m_aryDataDirectory[IMAGE_DIRECTORY_ENTRY_RESOURCE].VirtualAddress;

    if (nRva == 0)
    {
        return pRes;
    }

    size_t nOffset = RvaToOffset(nRva);

    if (!VALID_OFFSET(nOffset, sizeof(IMAGE_RESOURCE_DIRECTORY)))
    {
        return pRes;
    }

    pRes.SetOffset(nOffset);

    return pRes;
}


//获取资源文件偏移
size_t CPEFile::GetResourceOffset(size_t nOffset)
{
    //如果没有导入表项
    if (m_aryDataDirectory.GetCount() <= IMAGE_DIRECTORY_ENTRY_RESOURCE)
    {
        return -1;
    }

    //设置导入表偏移
    size_t nRva = m_aryDataDirectory[IMAGE_DIRECTORY_ENTRY_RESOURCE].VirtualAddress;

    if (nRva == 0)
    {
        return -1;
    }

    return RvaToOffset(nRva) + nOffset;
}


//获取资源数据
void *CPEFile::GetResourceData(PIMAGE_RESOURCE_DATA_ENTRY pEntry)
{
    return GetBuffer(GetResourceOffset(pEntry->OffsetToData), pEntry->Size);
}


//获取相对虚拟地址
size_t CPEFile::OffsetToRva(size_t nOffset)
{
    int nCount = m_arySectionHeader.GetCount();

    //如果节数为0 或者 在第一节之前
    if (nCount == 0 || nOffset < m_pNtHeaders->OptionalHeader.SizeOfHeaders)
    {
        return nOffset;
    }

    for (int i = 0; i < nCount; i++)
    {
        if (nOffset < m_arySectionHeader[i].PointerToRawData)
        {
            return -1;
        }

        size_t nRva = nOffset - m_arySectionHeader[i].PointerToRawData;

        if (nRva < (int)m_arySectionHeader[i].SizeOfRawData)
        {
            return nRva + m_arySectionHeader[i].VirtualAddress;
        }
    }

    return -1;
}


//获取文件偏移
size_t CPEFile::RvaToOffset(size_t nRva)
{
    int nCount = m_arySectionHeader.GetCount();

    //如果节数为0 或者 在第一节之前
    if (nCount == 0 || nRva < m_arySectionHeader[0].VirtualAddress)
    {
        if (nRva < m_pNtHeaders->OptionalHeader.SizeOfHeaders)
        {
            return nRva;
        }

        return -1;
    }

    for (int i = 0; i < nCount; i++)
    {
        if (nRva < m_arySectionHeader[i].VirtualAddress)
        {
            return -1;
        }

        size_t nOffset = nRva - m_arySectionHeader[i].VirtualAddress;

        if (nOffset < m_arySectionHeader[i].SizeOfRawData)
        {
            return nOffset + m_arySectionHeader[i].PointerToRawData;
        }
    }

    return -1;
}


//获取可读缓冲区，nLen为0不指定长度
void *CPEFile::GetBufferByRva(size_t nRva, size_t nLen)
{
    return GetBuffer(RvaToOffset(nRva), nLen);
}


//获取可写缓冲区
void *CPEFile::SetBufferByRva(size_t nRva, size_t nLen)
{
    return SetBuffer(RvaToOffset(nRva), nLen);
}