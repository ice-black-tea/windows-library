#pragma once

#include "PEDef.h"

/************************************************************************/
/*                               PE文件                                 */
/************************************************************************/

class CPEFile : public CFileEdit
{
public:
    CPEFile();
    CPEFile(LPCSTR pszFileName);
    CPEFile(LPCWSTR pszFileName);
    virtual ~CPEFile();

public:
    HMODULE Load(HMODULE hModule = NULL, bool bCallEntry = true); //指定地址加载pe文件

    bool ParseFile(); //读取文件数据并解析pe头
    bool ParseHeader(); //解析pe头

    bool AppendSection(); //添加节
    bool DeleteSection(int nIndex); //删除节

    CImageExportDirectoryPtr ParseExportTable(); //解析导出表
    CDwordAry GetExportFunctions(PIMAGE_EXPORT_DIRECTORY pExport); //导出函数地址（RVA）
    CDwordAry GetExportNames(PIMAGE_EXPORT_DIRECTORY pExport); //导出名（下标与导出序号相同）
    CWordAry GetExportNameOrdinals(PIMAGE_EXPORT_DIRECTORY pExport); //导出序号

    CImageImportDescriptorAry ParseImportTable(); //解析导入表
    CImageThunkDataAry GetThunkAry(size_t nFirstThunk); //获取导入表Thunk数组

    CImageBaseRelocationPtr ParseBaseRelocation(); //解析重定位表
    CRelocTypeOffsetAry GetTypeOffsetAry(PIMAGE_BASE_RELOCATION pReloc); //获取重定位表偏移数组

    CImageTlsDirectoryPtr ParseTlsDirectory(); //解析Tls表

    CImageResourceDirectoryPtr ParseResourceDirectory(); //解析资源目录
    size_t GetResourceOffset(size_t nOffset); //获取资源文件偏移(参数为偏移)
    void *GetResourceData(PIMAGE_RESOURCE_DATA_ENTRY pEntry); //获取资源数据

    size_t OffsetToRva(size_t nOffset); //获取相对虚拟地址
    size_t RvaToOffset(size_t nRva); //获取文件偏移

    void *GetBufferByRva(size_t nRva, size_t nLen = 0); //获取可读缓冲区，nLen为0不指定长度
    void *SetBufferByRva(size_t nRva, size_t nLen); //获取可写缓冲区

public:
    CImageDosHeaderPtr m_pDosHeader;
    CImageNtHeadersPtr m_pNtHeaders;
    CImageDataDirectoryAry m_aryDataDirectory;
    CImageSectionHeaderAry m_arySectionHeader;
};

#include "PEImp.h"
