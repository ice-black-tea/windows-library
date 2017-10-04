#include "InjectUtil.h"
#include <tlhelp32.h>


CInjectUtil::CInjectUtil(DWORD dwPid)
    : m_dwPid(dwPid)
    , m_hProcess(NULL)
{
    if (dwPid != 0)
    {
        m_hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, m_dwPid);
    }
}


CInjectUtil::~CInjectUtil()
{
    if (m_hProcess != NULL)
    {
        CloseHandle(m_hProcess);
    }
}


//向目标进程加载DLL
DWORD CInjectUtil::LoadInjectDLL(WCHAR *pszDllPath, SIZE_T nSize, DWORD dwWaitTime)
{
    return ExecInjectFuction(L"kernel32", "LoadLibraryW", pszDllPath, nSize, dwWaitTime);
}


//远程执行函数
DWORD CInjectUtil::ExecInjectFuction(WCHAR *pszDllPath, CHAR *pszProcName, LPVOID pBuff, SIZE_T nSize, DWORD dwWaitTime)
{
    if (m_hProcess == NULL)
    {
        return 0;
    }

    LPVOID pBaseAddr = GetBaseAddress(pszDllPath);
    LPVOID pFucOffset = GetFuctionOffset(pszDllPath, pszProcName);
    LPVOID pFucAddr = (CHAR*)pBaseAddr + (SIZE_T)pFucOffset;

    return ExecRemoteFuction(pFucAddr, pBuff, nSize, dwWaitTime);
}


//是否包含字符串（不区分大小写）
static WCHAR* wcsistr(
    const WCHAR * str1,
    const WCHAR * str2
    )
{
    WCHAR *cp = (WCHAR *)str1;
    WCHAR *s1, *s2;

    if (!*str2)
        return((WCHAR *)str1);

    while (*cp)
    {
        s1 = cp;
        s2 = (WCHAR *)str2;

        while (*s1 && *s2)
        {
            WCHAR ch1 = *s1, ch2 = *s2;
            if (iswascii(*s1) && iswupper(*s1)) ch1 = towlower(*s1);
            if (iswascii(*s2) && iswupper(*s2)) ch2 = towlower(*s2);

            if (ch1 - ch2 == 0) s1++, s2++;
            else break;
        }

        if (!*s2)
            return(cp);

        cp++;
    }


    return(NULL);
}


//获取模块基地址
LPVOID CInjectUtil::GetBaseAddress(WCHAR *pszDllPath)
{
    LPVOID pAddr = NULL;

    HANDLE hModuleSnap = NULL;
    MODULEENTRY32 me32 = { 0 };

    // Take a snapshot of all modules in the specified process. 

    hModuleSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, m_dwPid);

    if (hModuleSnap == INVALID_HANDLE_VALUE)
    {
        goto EXIT_LABLE;
    }

    // Fill the size of the structure before using it. 

    me32.dwSize = sizeof(MODULEENTRY32W);

    // Walk the module list of the process, and find the module of 
    // interest. Then copy the information to the buffer pointed 
    // to by lpMe32 so that it can be returned to the caller. 

    if (Module32First(hModuleSnap, &me32))
    {
        do
        {
            if (wcsistr(me32.szExePath, pszDllPath) != NULL)
            {
                pAddr = me32.modBaseAddr;
                goto EXIT_LABLE;
            }

        } while (Module32Next(hModuleSnap, &me32));
    }

EXIT_LABLE:
    // Do not forget to clean up the snapshot object. 
    if (hModuleSnap != INVALID_HANDLE_VALUE)
    {
        CloseHandle(hModuleSnap);
    }

    return pAddr;
}


//获取函数对模块偏移
LPVOID CInjectUtil::GetFuctionOffset(WCHAR *pszDllPath, CHAR *pszProcName)
{
    LPVOID pOffset = NULL;
    HMODULE hDll = LoadLibraryW(pszDllPath);

    if (hDll != NULL)
    {
        pOffset = (LPSTR)GetProcAddress(hDll, pszProcName) - (SIZE_T)hDll;
        FreeLibrary(hDll);
    }

    return pOffset;
}


//执行远程线程
DWORD CInjectUtil::ExecRemoteFuction(LPVOID pFucAddr, LPVOID pBuff, SIZE_T nSize, DWORD dwWaitTime)
{
    DWORD dwRet = 0;
    LPVOID pAddr = NULL;
    HANDLE hThread = NULL;

    if (pBuff != NULL && nSize != 0)
    {
        //向目标进程写入DLL地址
        pAddr = VirtualAllocEx(
            m_hProcess,
            NULL,
            nSize,
            MEM_COMMIT,
            PAGE_EXECUTE_READWRITE);

        if (pAddr == NULL)
        {
            goto EXIT_LABLE;
        }

        SIZE_T nWriteBytes = 0;

        BOOL bRet = WriteProcessMemory(
            m_hProcess,
            pAddr,
            pBuff,
            nSize,
            &nWriteBytes);

        if (!bRet || nSize != nWriteBytes)
        {
            goto EXIT_LABLE;
        }
    }

    //创建远程线程, 让目标进程执行代码
    hThread = CreateRemoteThread(
        m_hProcess,
        NULL,
        0,
        (LPTHREAD_START_ROUTINE)pFucAddr,
        pAddr,//目标进程中指定的Dll路径的地址
        0,
        NULL);

    WaitForSingleObject(hThread, dwWaitTime);

    GetExitCodeThread(hThread, &dwRet);

EXIT_LABLE:

    if (pAddr != NULL)
    {
        VirtualFreeEx(m_hProcess, pAddr, 0, MEM_RELEASE);
    }

    if (hThread != NULL)
    {
        CloseHandle(hThread);
    }

    return dwRet;
}