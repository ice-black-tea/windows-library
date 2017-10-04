#pragma once

#include "../Common.h"
#include "Define.h"
#include "SoftBreakPoint.h"
#include "HardBreakPoint.h"
#include "MemBreakPoint.h"

class CDebugger
{
public:
    CDebugger();
    virtual ~CDebugger();

public:
    //调试程序是否运行
    bool IsRunning();
    //获得进程id
    DWORD GetProcessID();

    //设置文件名
    void SetFileName(CMyStringW strFileName);
    //获取文件名
    CMyStringW GetFileName() const;

    //设置参数
    void SetParam(LPVOID pParam);
    //获取参数
    LPVOID GetParam() const;

    //初始化
    bool Start(PFNDebugNotify pfnNotify);
    //暂停
    bool Suspend();
    //终止
    void ShutDown();

    //读内存
    bool ReadMemeory(LPVOID pAddr, LPVOID pBuff, SIZE_T dwLen);
    //写内存
    bool WriteMemeory(LPVOID pAddr, LPVOID pBuff, SIZE_T dwLen);

    //获取程序入口点
    LPVOID GetEntryPoint();

    //获得汇编指令
    bool GetAsmCode(LPVOID pAddr, char *pszAsmCode, char *pszOpcode, DWORD *pdwCodeLen);

    //获取一般断点列表
    CBreakPointList& GetSoftBreakPointList();
    //获取硬件断点列表
    CBreakPointList& GetHardBreakPointList();
    //获取内存断点列表
    CBreakPointList& GetMemBreakPointList();

    //设置一般断点
    bool SetSoftBreakPoint(
        LPVOID pAddr,
        int nType = BREAKPOINT_ALWAYS);

    //设置硬件断点
    bool SetHardBreakPoint(
        LPVOID pAddr, //地址
        SIZE_T dwLen, //大小
        int nOperate = BREAKPOINT_READ, //类型
        int nStatus = BREAKPOINT_ALWAYS);

    //设置内存断点
    bool SetMemBreakPoint(
        LPVOID pAddr, //地址
        SIZE_T dwLen, //大小
        int nOperate = BREAKPOINT_ACCESS, //类型
        int nStatus = BREAKPOINT_ALWAYS);

    //移除一般断点
    bool RemoveSoftBreakPoint(int nIndex);
    bool RemoveSoftBreakPoint(IBreakPoint *pBp);
    //移除一般断点
    bool RemoveHardBreakPoint(int nIndex);
    bool RemoveHardBreakPoint(IBreakPoint *pBp);
    //移除一般断点
    bool RemoveMemBreakPoint(int nIndex);
    bool RemoveMemBreakPoint(IBreakPoint *pBp);
    //移除断点
    bool RemoveBreakPoint(CBreakPointList& lstBp, int nIndex);
    bool RemoveBreakPoint(CBreakPointList& lstBp, IBreakPoint *pBp);

    //置单步步入
    bool SetStepInto(LPDEBUG_EVENT pDbgEvt, bool bNotify = true);
    bool SetStepInto(CONTEXT *pContext, bool bNotify = true);

    //置单步步过
    bool SetStepOver(LPDEBUG_EVENT pDbgEvt); 
    bool SetStepOver(CONTEXT *pContext);
    bool IsStepOver(DWORD dwPid, LPVOID pAddr, PDWORD pdwCodeLen); //是否需要单步步过

    //修正二进制指令
    void AdjustBinary(PBYTE pbtCode, LPVOID pAddr, SIZE_T nCount);

    //刷新内存信息
    void RefreshMemInfo();

protected:
    //处理调试事件线程
    static unsigned __stdcall DebugThreadProc(LPVOID lpParam);

    // 处理调试事件
    BEGIN_DEBUG_EVENT_MAP()
        DECLARE_DEBUG_EVENT(CREATE_PROCESS_DEBUG_EVENT, OnCreateProcessDebugEvent)
        DECLARE_DEBUG_EVENT(EXIT_PROCESS_DEBUG_EVENT, OnExitProcessDebugEvent)
        DECLARE_DEBUG_EVENT(EXCEPTION_DEBUG_EVENT, OnExceptionDebugEvent)
    END_DEBUG_EVENT_MAP()

    // 处理异常调试事件
    BEGIN_EXCEPTION_MAP()
        DECLARE_EXCEPTION(EXCEPTION_BREAKPOINT, OnExceptionBreakPoint)
        DECLARE_EXCEPTION(EXCEPTION_SINGLE_STEP, OnExceptionSingleStep)
        DECLARE_EXCEPTION(EXCEPTION_ACCESS_VIOLATION, OnExceptionAccessViolation)
    END_EXCEPTION_MAP()

    DWORD OnCreateProcessDebugEvent(CDebugEvent *pDbgEvt);
    DWORD OnExitProcessDebugEvent(CDebugEvent *pDbgEvt);
    DWORD OnExceptionBreakPoint(CDebugEvent *pDbgEvt);
    DWORD OnExceptionSingleStep(CDebugEvent *pDbgEvt);
    DWORD OnExceptionAccessViolation(CDebugEvent *pDbgEvt);

protected:
    LPVOID              m_pParam;       //留个位置填填自定义参数
    CMyStringW          m_strFileName;  //调试文件名
    PFNDebugNotify      m_pfnNotify;    //调试回调函数

    HANDLE              m_hDbgThread;   //调试线程
    HANDLE              m_hExitEvent;   //退出事件
    DWORD               m_dwProcessId;  //进程id

    CBreakPointList     m_lstBp;        //一般断点列表
    CBreakPointList     m_lstHbp;       //硬件断点列表
    CBreakPointList     m_lstMbp;       //内存断点列表
    CBreakPointList     m_lstRecover;   //需要恢复的断点列表

    CMemInfoList        m_lstMemInfo;   //内存信息

    BOOL                m_bSystemBp;    //是否系统断点
    BOOL                m_bSingleStep;  //单步释放需要提示
    LPVOID              m_pLastAddr;
};

//设置参数
inline void CDebugger::SetParam(LPVOID pParam)
{
    m_pParam = pParam;
}

//获取参数
inline LPVOID CDebugger::GetParam() const
{
    return m_pParam;
}

//设置文件名
inline void CDebugger::SetFileName(CMyStringW strFileName)
{
    ShutDown();
    m_strFileName = strFileName;
}

//获取文件名
inline CMyStringW CDebugger::GetFileName() const
{
    return m_strFileName;
}

//获得进程id
inline DWORD CDebugger::GetProcessID()
{
    return m_dwProcessId;
}

//获取一般断点列表
inline CBreakPointList& CDebugger::GetSoftBreakPointList()
{
    return m_lstBp;
}

//获取硬件断点列表
inline CBreakPointList& CDebugger::GetHardBreakPointList()
{
    return m_lstHbp;
}

//获取内存断点列表
inline CBreakPointList& CDebugger::GetMemBreakPointList()
{
    return m_lstMbp;
}

//移除一般断点
inline bool CDebugger::RemoveSoftBreakPoint(int nIndex)
{
    return RemoveBreakPoint(m_lstBp, nIndex);
}

//移除一般断点
inline bool CDebugger::RemoveSoftBreakPoint(IBreakPoint *pBp)
{
    return RemoveBreakPoint(m_lstBp, pBp);
}

//移除硬件断点
inline bool CDebugger::RemoveHardBreakPoint(int nIndex)
{
    return RemoveBreakPoint(m_lstHbp, nIndex);
}

//移除硬件断点
inline bool CDebugger::RemoveHardBreakPoint(IBreakPoint *pBp)
{
    return RemoveBreakPoint(m_lstHbp, pBp);
}

//移除内存断点
inline bool CDebugger::RemoveMemBreakPoint(int nIndex)
{
    return RemoveBreakPoint(m_lstMbp, nIndex);
}

//移除内存断点
inline bool CDebugger::RemoveMemBreakPoint(IBreakPoint *pBp)
{
    return RemoveBreakPoint(m_lstMbp, pBp);
}