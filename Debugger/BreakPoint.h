#pragma once

#include "../Common/List.h"
#include "../Common/SmartPtr.h"
#include "Define.h"


#define BREAKPOINT_TYPE(flag) ((flag) & BREAKPOINT_TYPE_MASK)
#define BREAKPOINT_STATUS(flag) ((flag) & BREAKPOINT_STATUS_MASK)
#define BREAKPOINT_OPERATE(flag) ((flag) & BREAKPOINT_OPERATE_MASK)

#define SET_BREAKPOINT_TYPE(var, flag) (((var) &= ~BREAKPOINT_TYPE_MASK) |= BREAKPOINT_TYPE(flag))
#define SET_BREAKPOINT_STATUS(var, flag) (((var) &= ~BREAKPOINT_STATUS_MASK) |= BREAKPOINT_STATUS(flag))
#define SET_BREAKPOINT_OPERATE(var, flag) (((var) &= ~BREAKPOINT_OPERATE_MASK) |= BREAKPOINT_OPERATE(flag))

#define IS_BREAKPOINT_FORBIDDEN(flag) (((flag) & BREAKPOINT_FORBIDDEN) != 0)

enum
{
    BREAKPOINT_SOFTWARE     = 0x0001,   //一般断点
    BREAKPOINT_HARDWARE     = 0x0002,   //硬件断点
    BREAKPOINT_MEMORY       = 0x0004,   //内存断点
    BREAKPOINT_TYPE_MASK    = 0x0007,

    BREAKPOINT_ONCE         = 0x0010,   //仅一次
    BREAKPOINT_ALWAYS       = 0x0020,   //始终
    BREAKPOINT_FORBIDDEN    = 0x0040,   //禁用
    BREAKPOINT_STATUS_MASK  = 0x0070,

    BREAKPOINT_EXECUTE      = 0x0100,  //执行（硬件断点）
    BREAKPOINT_READ         = 0x0200,  //读（硬件断点）
    BREAKPOINT_ACCESS       = 0x0200,  //访问（内存断点）
    BREAKPOINT_WRITE        = 0x0400,  //写（内存断点、硬件断点）
    BREAKPOINT_OPERATE_MASK = 0x0700,
};


class IBreakPoint : public CRefCnt
{
public:
    IBreakPoint();
    virtual ~IBreakPoint();

public:
    virtual int GetType() = 0;
    virtual int GetStatus() = 0;
    virtual int GetOperate() = 0;
    virtual LPVOID GetAddress() = 0;

    virtual bool IsSet() = 0;
    virtual bool Set(DWORD dwPid) = 0;
    virtual bool Reset(DWORD dwPid) = 0;
};


typedef CSmartPtr<IBreakPoint> CBreakPointPtr;


class CBreakPointList :
    public CMyList<CBreakPointPtr>
{
public:
    void Set(DWORD dwPid);
    void Reset(DWORD dwPid);
};