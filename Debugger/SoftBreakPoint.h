#pragma once

#include "BreakPoint.h"


class CSoftBreakPoint
    : public IBreakPoint
{
public:
    CSoftBreakPoint(LPVOID pAddr, int nStatus = BREAKPOINT_ALWAYS);
    virtual ~CSoftBreakPoint();

public:
    virtual int GetType();
    virtual int GetStatus();
    virtual int GetOperate();
    virtual LPVOID GetAddress();

    virtual bool IsSet();
    virtual bool Set(DWORD dwPid);
    virtual bool Reset(DWORD dwPid);

    bool IsMatch(PEXCEPTION_RECORD pExcpRec);
    BYTE GetBpCode();
    BYTE GetOrigCode();

protected:
    INT    m_nFlag;
    BOOL   m_bSet;       //是否设置
    LPVOID m_pAddr;      //断点地址
    BYTE   m_btOrigCode; //原指令
    BYTE   m_btBpCode;   //中断指令
};
