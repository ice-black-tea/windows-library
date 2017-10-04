#include "Debugger.h"
#include <stdio.h>
#include <process.h>
#include <assert.h>
#include "../include/Disasm/Decode2Asm.h"


#ifndef _WIN64
#define ip Eip
#else
#define ip Rip
#endif

CDebugger::CDebugger()
    : m_hDbgThread(INVALID_HANDLE_VALUE)
    , m_hExitEvent(NULL)
    , m_bSingleStep(FALSE)
    , m_dwProcessId(0)
{

}


CDebugger::~CDebugger()
{
    ShutDown();
}

//调试程序是否运行
bool CDebugger::IsRunning()
{
    if (m_dwProcessId == 0)
    {
        return false;
    }

    HANDLE aryHandle[] = { 
        m_hExitEvent,
        m_hDbgThread
    };

    DWORD dwRet = WaitForMultipleObjects(
        sizeof(aryHandle) / sizeof(aryHandle[0]),
        aryHandle,
        false,
        0);

    return dwRet == WAIT_TIMEOUT;
}

//初始化
bool CDebugger::Start(PFNDebugNotify pfnNotify)
{
    assert(!IsRunning());
    assert(pfnNotify != NULL);

    m_pfnNotify = pfnNotify;

    //退出事件
    m_hExitEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    if (m_hExitEvent == NULL)
    {
        goto FAIL_LABLE;
    }

    //创建调试线程
    m_hDbgThread = (HANDLE)_beginthreadex(
        NULL,
        0,
        DebugThreadProc,
        this,
        0,
        NULL);

    if (m_hDbgThread == INVALID_HANDLE_VALUE)
    {
        goto FAIL_LABLE;
    }

    return true;

FAIL_LABLE:

    ShutDown();

    return false;
}


//暂停
bool CDebugger::Suspend()
{
    HPROCESS hProcess(m_dwProcessId);

    if (hProcess == NULL)
    {
        return false;
    }

    return DebugBreakProcess(hProcess) == TRUE;
}

//终止
void CDebugger::ShutDown()
{
    //如果还在运行，先通知
    if (m_hExitEvent != NULL)
    {
        SetEvent(m_hExitEvent);
    }

    //关闭线程
    if (m_dwProcessId != 0)
    {
        DebugActiveProcessStop(m_dwProcessId);
    }

    //等待调试进程退出
    if (m_hDbgThread != INVALID_HANDLE_VALUE)
    {
        while (WaitForSingleObject(m_hDbgThread, 0) == WAIT_TIMEOUT)
        {
            Sleep(100);
        }

        CloseHandle(m_hDbgThread);
        m_hDbgThread = INVALID_HANDLE_VALUE;
    }

    //关闭事件句柄
    if (m_hExitEvent != NULL)
    {
        CloseHandle(m_hExitEvent);
        m_hExitEvent = NULL;
    }

    m_bSingleStep = false;
}

//读内存
bool CDebugger::ReadMemeory(
    LPVOID pAddr,
    LPVOID pBuff,
    SIZE_T dwLen)
{
    //打开进程
    HPROCESS hProcess(m_dwProcessId);

    if (hProcess == NULL)
    {
        return false;
    }

    SIZE_T dwBytes = 0;
    BOOL bRet = ReadProcessMemory(
        hProcess,
        pAddr,
        pBuff,
        dwLen,
        &dwBytes);

    if (dwBytes != dwLen)
    {
        return false;
    }

    return true;
}

//写内存
bool CDebugger::WriteMemeory(
    LPVOID pAddr,
    LPVOID pBuff,
    SIZE_T dwLen)
{
    //打开进程
    HPROCESS hProcess(m_dwProcessId);

    if (hProcess == NULL)
    {
        return false;
    }

    SIZE_T dwBytes = 0;
    BOOL bRet = WriteProcessMemory(
        hProcess,
        pAddr,
        pBuff,
        dwLen,
        &dwBytes);

    if (dwBytes != dwLen)
    {
        return false;
    }

    return true;
}

//拿到第一个模块之间返回
ENUM_RET_TYPE GetModuleHandle(MODULEENTRY32 *pModuleEntry, LPVOID pParam)
{
    *(HMODULE*)pParam = pModuleEntry->hModule;

    return ENUM_RET_TRUE;
}

//获取程序入口点
LPVOID CDebugger::GetEntryPoint()
{
    HMODULE hModule = NULL;

    //获取主模块
    BOOL bRet = CSnapshoot::EnumModule(
        GetModuleHandle,
        m_dwProcessId,
        &hModule);

    if (!bRet)
    {
        return NULL;
    }

    //打开进程
    HPROCESS hProcess(m_dwProcessId);

    if (hProcess == NULL)
    {
        return false;
    }

    SIZE_T dwBytes = 0;

    IMAGE_DOS_HEADER dosHeader = { 0 };

    //读Dos头
    bRet = ReadProcessMemory(
        hProcess,
        hModule,
        &dosHeader,
        sizeof(dosHeader),
        &dwBytes);

    if (dwBytes != sizeof(dosHeader) ||
        dosHeader.e_magic != 0x5A4D)
    {
        return false;
    }

    IMAGE_NT_HEADERS ntHeaders = { 0 };

    //读Nt头
    bRet = ReadProcessMemory(
        hProcess,
        (PBYTE)hModule + dosHeader.e_lfanew,
        &ntHeaders,
        sizeof(ntHeaders),
        &dwBytes);

    if (dwBytes != sizeof(ntHeaders) ||
        ntHeaders.Signature != 0x00004550)
    {
        return NULL;
    }

    return (PBYTE)hModule + ntHeaders.OptionalHeader.AddressOfEntryPoint;
}

//获得汇编指令
bool CDebugger::GetAsmCode(
    LPVOID pAddr,
    char *pszAsmCode,
    char *pszOpcode,
    DWORD *pdwCodeLen)
{
    //打开进程
    HPROCESS hProcess(m_dwProcessId);

    if (hProcess == NULL)
    {
        return false;
    }

    //读二进制数据
    SIZE_T dwBytes = 0;
    BYTE szCode[MAX_BIN_CODE] = { 0 };

    BOOL bRet = ReadProcessMemory(
        hProcess,
        pAddr,
        szCode,
        sizeof(szCode),
        &dwBytes);

    //如果读失败了，试试改一下内存属性
    if (dwBytes == 0)
    {
        DWORD dwProtect = 0;

        //改成可读的
        bRet = VirtualProtectEx(
            hProcess,
            pAddr,
            sizeof(szCode),
            PAGE_READWRITE,
            &dwProtect);

        if (!bRet)
        {
            return false;
        }

        //读内容
        ReadProcessMemory(
            hProcess,
            pAddr,
            szCode,
            sizeof(szCode),
            &dwBytes);

        //还原原来的保护属性
        bRet = VirtualProtectEx(
            hProcess,
            pAddr,
            sizeof(szCode),
            dwProtect,
            &dwProtect);

        if (!bRet || dwBytes == 0)
        {
            return false;
        }
    }

    //修正二进制指令
    AdjustBinary(szCode, pAddr, dwBytes);

    //显示反汇编指令
    Decode2AsmOpcode(
        szCode,
        pszAsmCode,
        pszOpcode,
        (UINT*)pdwCodeLen,
        (UINT)pAddr);

    return true;
}

//下一般断点
bool CDebugger::SetSoftBreakPoint(LPVOID pAddr, int nType)
{
    CBreakPointPtr pBp = new CSoftBreakPoint(pAddr, nType);

    if (pBp == NULL)
    {
        return false;
    }

    if (!pBp->Set(m_dwProcessId))
    {
        return false;
    }

    m_lstBp.AddTail(pBp);

    return true;
}

//设置硬件断点
bool CDebugger::SetHardBreakPoint(
    LPVOID pAddr,
    SIZE_T dwSize,
    int nOperate,
    int nStatus)
{
    DWORD dwIndex = CHardBreakPoint::FindFreeReg(&m_lstHbp);

    if (dwIndex >= 4)
    {
        return false;
    }

    CBreakPointPtr pHbp = new CHardBreakPoint(
        dwIndex, pAddr, dwSize, nOperate, nStatus);

    if (pHbp == NULL)
    {
        return false;
    }

    if (!pHbp->Set(m_dwProcessId))
    {
        return false;
    }

    m_lstHbp.AddTail(pHbp);

    return true;
}

//设置内存断点
bool CDebugger::SetMemBreakPoint(
    LPVOID pAddr,
    SIZE_T dwLen,
    int nOperate,
    int nStatus)
{
    CBreakPointList lstMbp;
    
    CBreakPointList::iterator itBp = m_lstMbp.begin();

    //先重置一下原来的保护属性
    for (; itBp != m_lstMbp.end(); itBp++)
    {
        CBreakPointPtr pBp = *itBp;

        if (pBp->IsSet())
        {
            pBp->Reset(m_dwProcessId);
            lstMbp.AddTail(pBp);
        }
    }

    //刷新一下内存属性
    RefreshMemInfo();

    //还原原有的内存断点
    lstMbp.Set(m_dwProcessId);

    CBreakPointPtr pMbp = new CMemBreakPoint(
        pAddr, dwLen, &m_lstMemInfo, nOperate, nStatus);

    if (pMbp == NULL)
    {
        return false;
    }

    //设置内存断点
    if (!pMbp->Set(m_dwProcessId))
    {
        return false;
    }

    //添加到内存断点列表中
    m_lstMbp.AddTail(pMbp);

    return true;
}

//移除断点
bool CDebugger::RemoveBreakPoint(CBreakPointList& lstBp, int nIndex)
{
    //拿到指定断点
    POSITION pos = lstBp.GetPosition(nIndex);

    if (pos != NULL)
    {
        CBreakPointPtr pBp = lstBp.GetAt(pos);

        if (pBp->IsSet())
        {
            pBp->Reset(m_dwProcessId);
        }

        //移除恢复链表
        m_lstRecover.Remove(pBp);
        lstBp.RemoveAt(pos);

        return true;
    }

    return false;
}

//移除断点
bool CDebugger::RemoveBreakPoint(CBreakPointList& lstBp, IBreakPoint *pBp)
{
    if (pBp != NULL)
    {
        if (pBp->IsSet())
        {
            pBp->Reset(m_dwProcessId);
        }

        //移除恢复链表
        m_lstRecover.Remove(pBp);

        return lstBp.Remove(pBp);
    }

    return false;
}


//设置单步步入
bool CDebugger::SetStepInto(LPDEBUG_EVENT pDbgEvt, bool bNotify)
{
    //打开线程
    HTHREAD hThread(pDbgEvt->dwThreadId);

    if (hThread == NULL)
    {
        return false;
    }

    //获取线程环境
    CONTEXT Context = { 0 };
    Context.ContextFlags |= CONTEXT_ALL;
    BOOL bRet = GetThreadContext(hThread, &Context);

    if (!bRet)
    {
        return false;
    }

    //设置单步步入
    SetStepInto(&Context, bNotify);

    bRet = SetThreadContext(hThread, &Context);

    return bRet != FALSE;
}


//设置单步步入
bool CDebugger::SetStepInto(CONTEXT *pContext, bool bNotify)
{
    //直接置单步
    pContext->EFlags |= 0x100;

    if (bNotify)
    {
        m_bSingleStep = TRUE;
    }

    return true;
}


//设置单步步过
bool CDebugger::SetStepOver(LPDEBUG_EVENT pDbgEvt)
{
    //打开线程
    HTHREAD hThread(pDbgEvt->dwThreadId);

    if (hThread == NULL)
    {
        return false;
    }

    //获取线程环境
    CONTEXT Context = { 0 };
    Context.ContextFlags |= CONTEXT_ALL;
    BOOL bRet = GetThreadContext(hThread, &Context);

    if (!bRet)
    {
        return false;
    }

    //设置单步步过
    SetStepOver(&Context);

    bRet = SetThreadContext(hThread, &Context);

    return bRet != FALSE;
}


bool CDebugger::SetStepOver(CONTEXT *pContext)
{
    //判断是单步步入还是单步步过
    DWORD dwCodeLen = 0;

    if (IsStepOver(m_dwProcessId, (LPVOID)pContext->ip, &dwCodeLen))
    {
        //步过就在下一条指令下断点
        SetSoftBreakPoint((LPVOID)(pContext->ip + dwCodeLen), BREAKPOINT_ONCE);
    }
    else
    {
        //直接置单步
        pContext->EFlags |= 0x100;
        m_bSingleStep = TRUE;
    }

    return true;
}


//是否需要单步步过
bool CDebugger::IsStepOver(DWORD dwPid, LPVOID pAddr, PDWORD pdwCodeLen)
{
    //打开进程
    HPROCESS hProcess(dwPid);

    if (hProcess == NULL)
    {
        return false;
    }

    //读二进制数据
    SIZE_T dwBytes = 0;
    BYTE szCode[MAX_BIN_CODE] = { 0 };

    BOOL bRet = ReadProcessMemory(
        hProcess,
        pAddr,
        szCode,
        sizeof(szCode),
        &dwBytes);

    if (dwBytes == 0)
    {
        return false;
    }

    //修正二进制指令
    AdjustBinary(szCode, pAddr, dwBytes);

    char szAsmCode[MAX_ASM_CODE] = { 0 };
    char szOpcode[MAX_ASM_CODE] = { 0 };

    //显示反汇编指令
    Decode2AsmOpcode(
        szCode,
        szAsmCode,
        szOpcode,
        (UINT*)pdwCodeLen,
        (UINT)pAddr);

    //判断当前指令是否为call
    return (strstr(szAsmCode, "call") != NULL);
}


//修正二进制指令
void CDebugger::AdjustBinary(PBYTE pbtCode, LPVOID pAddr, SIZE_T nCount)
{
    CBreakPointList::iterator itBp = m_lstBp.begin();

    for (; itBp != m_lstBp.end(); itBp++)
    {
        CSoftBreakPoint *pBp = (CSoftBreakPoint*)(IBreakPoint*)*itBp;

        //如果在范围内
        if (pBp->IsSet() &&
            pBp->GetAddress() >= pAddr &&
            pBp->GetAddress() < (PBYTE)pAddr + nCount)
        {
            int nIndex = (int)((PBYTE)pBp->GetAddress() - (PBYTE)pAddr);

            //如果现指令相同
            if (pbtCode[nIndex] == pBp->GetBpCode())
            {
                pbtCode[nIndex] = pBp->GetOrigCode();
            }
        }
    }

    return;
}


//刷新内存信息
void CDebugger::RefreshMemInfo()
{
    m_lstMemInfo.RemoveAll();

    HPROCESS hProcess(m_dwProcessId);

    if (hProcess == NULL)
    {
        return;
    }

    PBYTE pAddr = NULL;

    while (TRUE)
    {
        MEMORY_BASIC_INFORMATION memInfo;

        SIZE_T dwSize = VirtualQueryEx(
            hProcess,
            pAddr,
            &memInfo,
            sizeof(MEMORY_BASIC_INFORMATION));

        if (dwSize != sizeof(MEMORY_BASIC_INFORMATION))
        {
            break;
        }
        
        pAddr = (PBYTE)memInfo.BaseAddress + memInfo.RegionSize;

        if (memInfo.State != MEM_COMMIT)
        {
            continue;
        }

        m_lstMemInfo.AddTail(memInfo);
    }

    return;
}


//处理调试事件
unsigned __stdcall CDebugger::DebugThreadProc(LPVOID lpParam)
{
    CDebugger *pThis = (CDebugger*)lpParam;

    STARTUPINFO si = { 0 };
    si.cb = sizeof(si);

    PROCESS_INFORMATION pi = { 0 };

    //创建调试进程
    BOOL bRet = CreateProcessW(
        NULL,                   // No module name (use command line). 
        pThis->m_strFileName,   // Command line. 
        NULL,                   // Process handle not inheritable. 
        NULL,                   // Thread handle not inheritable. 
        FALSE,                  // Set handle inheritance to FALSE. 
        DEBUG_ONLY_THIS_PROCESS,// Debug flag. 
        NULL,                   // Use parent's environment block. 
        NULL,                   // Use parent's starting directory. 
        &si,                    // Pointer to STARTUPINFO structure.
        &pi);                   // Pointer to PROCESS_INFORMATION structure.

    if (!bRet)
    {
        return 0;
    }

    pThis->m_dwProcessId = pi.dwProcessId;

    CDebugEvent dbgEvt;

    while (pThis->m_dwProcessId != 0)
    {
        //接受调试事件
        BOOL bRet = WaitForDebugEvent(&dbgEvt, INFINITE);

        if (bRet)
        {
            //回调函数
            pThis->m_pfnNotify(
                NOTIFY_DEBUGEVENT_PRE,
                pThis,
                &dbgEvt);

            dbgEvt.m_dwContinueStatus = OnDebugEvent(
                dbgEvt.dwDebugEventCode,
                pThis,
                &dbgEvt);

            //回调函数
            pThis->m_pfnNotify(
                NOTIFY_DEBUGEVENT_POST,
                pThis,
                &dbgEvt);

            //提交事件处理结果
            ContinueDebugEvent(
                dbgEvt.dwProcessId,
                dbgEvt.dwThreadId,
                dbgEvt.m_dwContinueStatus);
        }
        else if (!bRet && GetLastError() != ERROR_SEM_TIMEOUT)
        {
            pThis->m_pfnNotify(
                NOTIFY_ERROR,
                pThis,
                NULL);

            break;
        }
    }

    CloseHandle(pi.hProcess);

    return 0;
}


DWORD CDebugger::OnCreateProcessDebugEvent(CDebugEvent *pDbgEvt)
{
    //这个标记一下等一下要来系统断点
    m_bSystemBp = true;

    m_pfnNotify(NOTIFY_PROCESS_CREATE, this, pDbgEvt);

    return DBG_EXCEPTION_NOT_HANDLED;
}


DWORD CDebugger::OnExitProcessDebugEvent(CDebugEvent *pDbgEvt)
{
    m_pfnNotify(NOTIFY_PROCESS_EXIT, this, pDbgEvt);

    m_dwProcessId = 0;
    
    return DBG_EXCEPTION_NOT_HANDLED;
}


DWORD CDebugger::OnExceptionBreakPoint(CDebugEvent *pDbgEvt)
{
    BOOL bFound = FALSE;
    PEXCEPTION_RECORD pExcpRec = &pDbgEvt->u.Exception.ExceptionRecord;

    POSITION pos = m_lstBp.GetHeadPosition();

    //瞅瞅能不能找到匹配上的断点
    while (pos != NULL)
    {
        POSITION posCur = pos;
        CSoftBreakPoint *pBp = (CSoftBreakPoint*)
            (IBreakPoint*)m_lstBp.GetNext(pos);

        //运气不好匹配上了
        if (pBp->IsMatch(pExcpRec))
        {
            bFound = TRUE;

            //先重置断点
            pBp->Reset(pDbgEvt->dwProcessId);

            switch (pBp->GetStatus())
            {
            case BREAKPOINT_ALWAYS:
                m_lstRecover.AddTail(pBp); //添加到恢复链表
                break;
            case BREAKPOINT_ONCE:
                m_lstBp.RemoveAt(posCur); //用完就移除了吧
                break;
            }
        }
    }

    //如果没有匹配到直接返回
    if (!bFound)
    {
        if (m_bSystemBp)
        {
            m_bSystemBp = false;
            m_pfnNotify(NOTIFY_SYSTEM_BREAKPOINT, this, pDbgEvt);

            return DBG_CONTINUE;
        }

        return DBG_EXCEPTION_NOT_HANDLED;
    }

    //打开线程
    HTHREAD hThread(pDbgEvt->dwThreadId);

    if (hThread == NULL)
    {
        return DBG_EXCEPTION_NOT_HANDLED;
    }

    //获取线程环境
    CONTEXT Context = { 0 };
    Context.ContextFlags |= CONTEXT_ALL;
    BOOL bRet = GetThreadContext(hThread, &Context);

    if (!bRet)
    {
        return DBG_EXCEPTION_NOT_HANDLED;
    }

    //造单步异常
    SetStepInto(&Context, false);

    Context.ip--;

    //设置线程环境
    bRet = SetThreadContext(hThread, &Context);

    if (!bRet)
    {
        return DBG_EXCEPTION_NOT_HANDLED;
    }

    if (m_pLastAddr != (LPVOID)Context.ip)
    {
        m_pfnNotify(NOTIFY_BREAKPOINT, this, pDbgEvt);
        m_pLastAddr = (LPVOID)Context.ip;
    }

    return DBG_CONTINUE;
}


DWORD CDebugger::OnExceptionSingleStep(CDebugEvent *pDbgEvt)
{
    PEXCEPTION_RECORD pExepRec = &pDbgEvt->u.Exception.ExceptionRecord;

    if (m_pLastAddr != (LPVOID)pExepRec->ExceptionAddress)
    {
        //先恢复原有断点
        m_lstRecover.Set(pDbgEvt->dwProcessId);
        m_lstRecover.RemoveAll();
        m_pLastAddr = NULL;
    }

    //打开线程
    HTHREAD hThread(pDbgEvt->dwThreadId);

    if (hThread == NULL)
    {
        return DBG_EXCEPTION_NOT_HANDLED;
    }

    //获取线程环境
    CONTEXT Context = { 0 };
    Context.ContextFlags |= CONTEXT_ALL;
    BOOL bRet = GetThreadContext(hThread, &Context);

    if (!bRet)
    {
        return DBG_EXCEPTION_NOT_HANDLED;
    }

    POSITION pos = m_lstHbp.GetHeadPosition();

    //瞅瞅能不能找到匹配上的断点
    while (pos != NULL)
    {
        POSITION posCur = pos;
        CHardBreakPoint *pHbp = (CHardBreakPoint*)
            (IBreakPoint*)m_lstHbp.GetNext(pos);

        //运气不好匹配上了
        if (pHbp->IsMatch(&Context))
        {
            //先重置断点
            pHbp->Reset(&Context);

            switch (pHbp->GetStatus())
            {
            case BREAKPOINT_ALWAYS:
                m_lstRecover.AddTail(pHbp); //添加到恢复链表
                break;
            case BREAKPOINT_ONCE:
                m_lstHbp.RemoveAt(posCur); //用完就移除了吧
                break;
            }

            //断步配合
            SetStepInto(&Context, false);

            //需要单步通知
            m_bSingleStep = TRUE;
        }
    }

    //设置线程环境
    bRet = SetThreadContext(hThread, &Context);

    if (!bRet)
    {
        return DBG_EXCEPTION_NOT_HANDLED;
    }

    if (m_bSingleStep && 
        m_pLastAddr != pExepRec->ExceptionAddress)
    {
        m_bSingleStep = FALSE;
        m_pfnNotify(NOTIFY_BREAKPOINT, this, pDbgEvt);
        m_pLastAddr = pExepRec->ExceptionAddress;
    }

    return DBG_CONTINUE;
}


DWORD CDebugger::OnExceptionAccessViolation(CDebugEvent *pDbgEvt)
{
    BOOL bFound = FALSE;
    CMemBreakPoint *pMbpHit = NULL;
    PEXCEPTION_RECORD pExepRec = &pDbgEvt->u.Exception.ExceptionRecord;
    
    POSITION pos = m_lstHbp.GetHeadPosition();

    //瞅瞅能不能找到匹配上的断点
    while (pos != NULL)
    {
        POSITION posCur = pos;
        CMemBreakPoint *pMbp = (CMemBreakPoint*)
            (IBreakPoint*)m_lstHbp.GetNext(pos);

        //运气不好匹配上了
        if (pMbp->IsMatch(pExepRec))
        {
            //先重置断点
            pMbp->Reset(pDbgEvt->dwProcessId);

            switch (pMbp->GetStatus())
            {
            case BREAKPOINT_ALWAYS:
                m_lstRecover.AddTail(pMbp); //添加到恢复链表
                break;
            case BREAKPOINT_ONCE:
                m_lstMbp.RemoveAt(posCur); //用完就移除了吧
                break;
            }

            bFound = TRUE;
        }
        else if (pMbp->InRegion(pExepRec))
        {
            pMbpHit = pMbp;
        }
    }

    if (!bFound)
    {
        //如果是误伤
        if (pMbpHit != NULL)
        {
            pMbpHit->Reset(pDbgEvt->dwProcessId); //先还原内存属性
            m_lstRecover.AddTail(pMbpHit); //然后添加到恢复链表
            SetStepInto(pDbgEvt, false); //置单步

            return DBG_CONTINUE;
        }

        return DBG_EXCEPTION_NOT_HANDLED;
    }

    //断步配合
    SetStepInto(pDbgEvt, false);

    if (m_pLastAddr != pExepRec->ExceptionAddress)
    {
        m_pfnNotify(NOTIFY_BREAKPOINT, this, pDbgEvt);
        m_pLastAddr = pExepRec->ExceptionAddress;
    }

    return DBG_CONTINUE;
}