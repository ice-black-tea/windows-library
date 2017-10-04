#include "SoftBreakPoint.h"


CSoftBreakPoint::CSoftBreakPoint(LPVOID pAddr, int nStatus)
    : m_nFlag(BREAKPOINT_STATUS(nStatus))
    , m_bSet(FALSE)
    , m_pAddr(pAddr)
    , m_btOrigCode(0)
    , m_btBpCode(0xCC)
{

}


CSoftBreakPoint::~CSoftBreakPoint()
{

}


int CSoftBreakPoint::GetType()
{
    return BREAKPOINT_SOFTWARE;
}


int CSoftBreakPoint::GetStatus()
{
    return BREAKPOINT_STATUS(m_nFlag);
}


int CSoftBreakPoint::GetOperate()
{
    return BREAKPOINT_EXECUTE;
}


LPVOID CSoftBreakPoint::GetAddress()
{
    return m_pAddr;
}


bool CSoftBreakPoint::Set(DWORD dwPid)
{
    //打开进程
    HPROCESS hProcess(dwPid);

    if (hProcess == NULL)
    {
        return false;
    }

    SIZE_T dwBytes = 0;

    //保存原指令
    BOOL bRet = ReadProcessMemory(
        hProcess,
        m_pAddr,
        &m_btOrigCode,
        sizeof(m_btOrigCode),
        &dwBytes);

    if (!bRet || dwBytes != sizeof(m_btOrigCode))
    {
        return false;
    }

    //写入断点
    bRet = WriteProcessMemory(
        hProcess,
        m_pAddr,
        &m_btBpCode,
        sizeof(m_btBpCode),
        &dwBytes);

    if (!bRet || dwBytes != sizeof(m_btBpCode))
    {
        return false;
    }

    m_bSet = TRUE;

    return true;
}


bool CSoftBreakPoint::Reset(DWORD dwPid)
{
    //打开进程
    HPROCESS hProcess(dwPid);

    if (hProcess == NULL)
    {
        return false;
    }

    BYTE btCode = 0;
    SIZE_T dwBytes = 0;

    //读现在的指令
    BOOL bRet = ReadProcessMemory(
        hProcess,
        m_pAddr,
        &btCode,
        sizeof(btCode),
        &dwBytes);

    if (!bRet || dwBytes != sizeof(btCode))
    {
        SET_BREAKPOINT_STATUS(m_nFlag, BREAKPOINT_FORBIDDEN);
        return false;
    }

    //内存不对，就把这个断点列为禁用的吧
    if (btCode != m_btBpCode)
    {
        SET_BREAKPOINT_STATUS(m_nFlag, BREAKPOINT_FORBIDDEN);
        return false;
    }

    //恢复断点
    bRet = WriteProcessMemory(
        hProcess,
        m_pAddr,
        &m_btOrigCode,
        sizeof(m_btOrigCode),
        &dwBytes);

    if (!bRet || dwBytes != sizeof(m_btOrigCode))
    {
        SET_BREAKPOINT_STATUS(m_nFlag, BREAKPOINT_FORBIDDEN);
        return false;
    }

    m_bSet = FALSE;

    return true;
}


bool CSoftBreakPoint::IsSet()
{
    return !IS_BREAKPOINT_FORBIDDEN(m_nFlag) && m_bSet == TRUE;
}


bool CSoftBreakPoint::IsMatch(PEXCEPTION_RECORD pExcpRec)
{
    return IsSet() && m_pAddr == pExcpRec->ExceptionAddress;
}


BYTE CSoftBreakPoint::GetBpCode()
{
    return m_btBpCode;
}


BYTE CSoftBreakPoint::GetOrigCode()
{
    return m_btOrigCode;
}