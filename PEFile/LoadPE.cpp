#include "PEFile.h"
#include "LoadPE.h"

HMODULE LoadDLL(CPEFile *pFile);

/************************************************************************/
/*                      加载PE文件，同LoadLibrary                       */
/************************************************************************/

HMODULE LoadDLL(LPCTSTR lpFileName)
{
    CPEFile file(lpFileName);

    //打开并读取pe文件内容
    if (file.ParseFile())
    {
        return file.Load();
    }

    return NULL;
}


/************************************************************************/
/*             获取函数地址（没有绝对地址），同GetProcAddress           */
/************************************************************************/
extern "C" FARPROC GetProcAddr(
    HMODULE hModule,    // handle to DLL module
    LPCSTR lpProcName   // function name
    )
{
    IMAGE_DOS_HEADER *pDosHeader = (IMAGE_DOS_HEADER*)hModule;
    IMAGE_NT_HEADERS *pNtHeader = (IMAGE_NT_HEADERS*)((PBYTE)hModule + pDosHeader->e_lfanew);
    IMAGE_DATA_DIRECTORY *pDataDir = &pNtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];
    IMAGE_EXPORT_DIRECTORY *pExport = (IMAGE_EXPORT_DIRECTORY*)((PBYTE)hModule + pDataDir->VirtualAddress);
    
    DWORD* pFuctions = (DWORD*)((PBYTE)hModule + pExport->AddressOfFunctions);
    DWORD* pNames = (DWORD*)((PBYTE)hModule + pExport->AddressOfNames);
    WORD* pOrdinals = (WORD*)((PBYTE)hModule + pExport->AddressOfNameOrdinals);

    DWORD dwOrdinal = -1;

    //如果是函数名
    if (lpProcName > (LPCSTR)0xFFFF)
    {
        DWORD i = 0, j = 0;

        for (; i < pExport->NumberOfNames; i++)
        {
            LPSTR pszName = (LPSTR)hModule + pNames[i];

            //strcmp
            for (j = 0; lpProcName[j] != '\0' && pszName[j] != '\0'; j++)
            {
                if (lpProcName[j] != pszName[j])
                {
                    break;
                }
            }

            //名称匹配上
            if (lpProcName[j] == '\0' && pszName[j] == '\0')
            {
                dwOrdinal = pOrdinals[i];
                break;
            }
        }
    }
    else
    {
        dwOrdinal = (DWORD)lpProcName - pExport->Base;
    }

    //如果序号大于项数
    if (dwOrdinal >= pExport->NumberOfFunctions)
    {
        return NULL;
    }

    DWORD dwFuction = pFuctions[dwOrdinal];

    //为0那就说明不存在
    if (dwFuction == 0)
    {
        return NULL;
    }

    PBYTE pFuction = (PBYTE)hModule + dwFuction;

    //如果不在转导出表内，直接返回函数地址，否则就是转发表
    if (pFuction < (PBYTE)pExport || pFuction >= (PBYTE)pExport + pDataDir->Size)
    {
        return (FARPROC)pFuction;
    }

    PLDR_DATA_TABLE_ENTRY pLdtEntryHead = NULL;

#ifndef _WIN64
    //获取模块链表
    __asm mov eax, fs:[0x18]; //获取_TEB
    __asm mov eax, [eax + 0x30]; //获取_PEB
    __asm mov eax, [eax + 0xC]; //获取_PEB_LDR_DATA
    __asm mov eax, [eax + 0xC]; //获取InLoadOrderModuleList（加载模块链表）
    __asm mov pLdtEntryHead, eax;
#else
    // mov rax, qword ptr gs : [30] //获取_TEB
    // mov rax, qword ptr ds : [rax + 60] //获取_PEB
    // mov rax, qword ptr ds : [rax + 18] //获取_PEB_LDR_DATA
    // mov rax, qword ptr ds : [rax + 10] //InLoadOrderModuleList
    PBYTE pTeb = (PBYTE)__readgsqword(0x30);
    PBYTE pPeb = *(PBYTE*)(pTeb + 0x60);
    PBYTE pLdr = *(PBYTE*)(pPeb + 0x18);
    pLdtEntryHead = *(PLDR_DATA_TABLE_ENTRY*)(pLdr + 0x10);
#endif

    PLDR_DATA_TABLE_ENTRY pLdtEntry = pLdtEntryHead;

    do
    {
        if (pLdtEntry->BaseDllName.Length >= 24) //wcslen(L"kernel32.dll");
        {
            LPWSTR pszFileName = pLdtEntry->BaseDllName.Buffer;

            //找到kernel32.dll
            if ((pszFileName[0] == L'K' || pszFileName[0] == L'k') &&
                //(pszFileName[1] == L'E' || pszFileName[1] == L'e') &&
                (pszFileName[2] == L'R' || pszFileName[2] == L'r') &&
                //(pszFileName[3] == L'N' || pszFileName[3] == L'n') &&
                (pszFileName[4] == L'E' || pszFileName[4] == L'e') &&
                //(pszFileName[5] == L'L' || pszFileName[5] == L'l') &&
                (pszFileName[6] == L'3' || pszFileName[6] == L'3') &&
                //(pszFileName[7] == L'2' || pszFileName[7] == L'2') &&
                (pszFileName[8] == L'.' || pszFileName[8] == L'.') &&
                //(pszFileName[9] == L'D' || pszFileName[9] == L'd') &&
                //(pszFileName[10] == L'L' || pszFileName[10] == L'l') &&
                (pszFileName[11] == L'L' || pszFileName[11] == L'l'))
            {
                CHAR szLoadLibrary[] = {
                    'L', 'o', 'a', 'd', 'L', 'i', 'b', 'r', 'a', 'r', 'y', 'A', '\0'
                };

                //拿到LoadLibraryA
                PFNLoadLibraryA pfnLoadLibrary = (PFNLoadLibraryA)
                    GetProcAddr((HMODULE)pLdtEntry->DllBase, szLoadLibrary);

                if (pfnLoadLibrary == NULL)
                {
                    return NULL;
                }

                CHAR szDllName[MAX_PATH];

                int i = 0;

                //拿到dll名
                while (((LPCSTR)pFuction)[i] != '.')
                {
                    szDllName[i] = ((LPCSTR)pFuction)[i];
                    i++;
                }

                szDllName[i] = '\0';

                HMODULE hModule = pfnLoadLibrary(szDllName);

                //递归查找函数地址
                return GetProcAddr(hModule, (LPCSTR)(pFuction + i + 1));
            }
        }

        //遍历下一个
        pLdtEntry = (PLDR_DATA_TABLE_ENTRY)pLdtEntry->InLoadOrderLinks.Flink;

    } while (pLdtEntry != NULL && pLdtEntry != pLdtEntryHead);

    return NULL;
}