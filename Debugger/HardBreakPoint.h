#pragma once

#include "BreakPoint.h"
#include "../Util/Snapshoot.h"

class CHardBreakPoint
    : public IBreakPoint
{
public:
    friend class CDebugger;

    CHardBreakPoint(DWORD dwIndex, LPVOID pAddr, SIZE_T dwLen,
        int nOperate, int nStatus = BREAKPOINT_ALWAYS);

    virtual ~CHardBreakPoint();

public:
    virtual int GetType();
    virtual int GetStatus();
    virtual int GetOperate();
    virtual LPVOID GetAddress();

    virtual bool IsSet();
    virtual bool Set(DWORD dwPid);
    virtual bool Reset(DWORD dwPid);

    bool IsMatch(CONTEXT *pContext);
    DWORD GetIndex();
    SIZE_T GetLength();

protected:
    bool IsValid();
    bool Set(CONTEXT *pContext);
    bool Reset(CONTEXT *pContext);

    static ENUM_RET_TYPE Set(THREADENTRY32 *pThreadEntry, LPVOID pParam);
    static ENUM_RET_TYPE Reset(THREADENTRY32 *pThreadEntry, LPVOID pParam);
    static int FindFreeReg(CBreakPointList *pLstHbp); //找到一个可用的硬件断点

protected:
    INT    m_nFlag;
    BOOL   m_bSet;              //是否设置
    DWORD  m_dwIndex;           //Drx ==> x
    LPVOID m_pAddr;             //地址
    SIZE_T m_dwLen;             //大小
};