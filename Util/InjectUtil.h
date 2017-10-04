#pragma once

#include <windows.h>

/************************************************************************/
/*                             注入工具类                               */
/************************************************************************/
class CInjectUtil
{
public:
    CInjectUtil(DWORD dwPid = 0);
    ~CInjectUtil();

public:
    //设置进程ID
    inline void SetPid(DWORD dwPid);
    //获取进程ID
    inline DWORD GetPid() const;
    //进程是否有有效
    inline BOOL IsValidProcess() const;

    //获取模块基地址
    LPVOID GetBaseAddress(WCHAR *pszModuleName);
    //获取函数对模块偏移
    LPVOID GetFuctionOffset(WCHAR *pszDllPath, CHAR *pszProcName);
    //执行远程线程
    DWORD ExecRemoteFuction(LPVOID pFucAddr, LPVOID pBuff, SIZE_T nSize, DWORD dwWaitTime = INFINITE);
    
    //向目标进程加载DLL
    DWORD LoadInjectDLL(WCHAR *pszDllPath, SIZE_T nSize, DWORD dwWaitTime = INFINITE);
    //远程执行函数
    DWORD ExecInjectFuction(WCHAR *pszDllPath, CHAR *pszProcName, LPVOID pBuff, SIZE_T nSize, DWORD dwWaitTime = INFINITE);

protected:
    DWORD  m_dwPid;
    HANDLE m_hProcess;
};


//设置进程ID
inline void CInjectUtil::SetPid(DWORD dwPid)
{
    if (m_hProcess != NULL)
    {
        CloseHandle(m_hProcess);
    }

    m_dwPid = dwPid;
    m_hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, m_dwPid);
}


//获取进程ID
inline DWORD CInjectUtil::GetPid() const
{
    return m_dwPid;
}


//进程是否有有效
inline BOOL CInjectUtil::IsValidProcess() const
{
    return m_hProcess != NULL;
}