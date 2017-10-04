#include "HardBreakPoint.h"
#include <stddef.h>

#define DRX_SET(ctx, idx, val) \
    (((DWORD&)*((char*)&(ctx) + aryOffset[idx])) = (DWORD)val)

//执行断点
#define DR7_E(ctx, idx) \
    do \
    { \
        ((PDR7)&(ctx).Dr7)->L##idx = 1; \
        ((PDR7)&(ctx).Dr7)->RW##idx = 0; \
        ((PDR7)&(ctx).Dr7)->LEN##idx = 0; \
    } while (false);

//写入断点
#define DR7_W(ctx, idx, len) \
    do \
    { \
        ((PDR7)&(ctx).Dr7)->L##idx = 1; \
        ((PDR7)&(ctx).Dr7)->RW##idx = 1; \
        ((PDR7)&(ctx).Dr7)->LEN##idx = len; \
    } while (false);

//访问断点
#define DR7_R(ctx, idx, len) \
    do \
    { \
        ((PDR7)&(ctx).Dr7)->L##idx = 1; \
        ((PDR7)&(ctx).Dr7)->RW##idx = 3; \
        ((PDR7)&(ctx).Dr7)->LEN##idx = len; \
    } while (false);

//设置断点
#define DR7_SET(ctx, idx, type, len) \
    do \
    { \
        switch (type) \
        { \
        case BREAKPOINT_EXECUTE: \
            DR7_E(ctx, idx); \
            break; \
        case BREAKPOINT_WRITE: \
            DR7_W(ctx, idx, len); \
            break; \
        case BREAKPOINT_READ: \
            DR7_R(ctx, idx, len); \
            break; \
        } \
    } while (false);

#define DR7_RESET(ctx, idx) \
    ((PDR7)&(ctx).Dr7)->L##idx = 0;

const DWORD aryOffset[]
{
    offsetof(CONTEXT, Dr0),
    offsetof(CONTEXT, Dr1),
    offsetof(CONTEXT, Dr2),
    offsetof(CONTEXT, Dr3),
};

typedef struct tagDR6
{
    unsigned int B0 : 1;
    unsigned int B1 : 1;
    unsigned int B2 : 1;
    unsigned int B3 : 1;

    unsigned int Reserved : 9;

    unsigned int BD : 1;
    unsigned int BS : 1;
    unsigned int BT : 1;

    unsigned int Reserved1 : 16;

}DR6, *PDR6;


typedef struct tagDR7
{
    unsigned int L0 : 1;
    unsigned int G0 : 1;
    unsigned int L1 : 1;
    unsigned int G1 : 1;
    unsigned int L2 : 1;
    unsigned int G2 : 1;
    unsigned int L3 : 1;
    unsigned int G3 : 1;

    unsigned int LE : 1;
    unsigned int GE : 1;

    unsigned int Reserved : 3;
    unsigned int GD : 1;
    unsigned int Reserved1 : 2;

    unsigned int RW0 : 2;
    unsigned int LEN0 : 2;
    unsigned int RW1 : 2;
    unsigned int LEN1 : 2;
    unsigned int RW2 : 2;
    unsigned int LEN2 : 2;
    unsigned int RW3 : 2;
    unsigned int LEN3 : 2;
}DR7, *PDR7;


CHardBreakPoint::CHardBreakPoint(DWORD dwIndex, LPVOID pAddr,
    SIZE_T dwLen, int nOperate, int nStatus)
    : m_dwIndex(dwIndex)
    , m_pAddr(pAddr)
    , m_dwLen(dwLen)
    , m_bSet(FALSE)
    , m_nFlag(nOperate | nStatus)
{

}


CHardBreakPoint::~CHardBreakPoint()
{
}


int CHardBreakPoint::GetType()
{
    return BREAKPOINT_HARDWARE;
}


int CHardBreakPoint::GetStatus()
{
    return BREAKPOINT_STATUS(m_nFlag);
}


int CHardBreakPoint::GetOperate()
{
    return BREAKPOINT_OPERATE(m_nFlag);
}


inline LPVOID CHardBreakPoint::GetAddress()
{
    return m_pAddr;
}


bool CHardBreakPoint::Set(DWORD dwPid)
{
    if (!IsValid())
    {
        return false;
    }

    if (!CSnapshoot::EnumThread(Set, dwPid, this))
    {
        CSnapshoot::EnumThread(Reset, dwPid, this);

        return false;
    }

    m_bSet = TRUE;

    return true;
}


bool CHardBreakPoint::Reset(DWORD dwPid)
{
    if (!IsValid())
    {
        return false;
    }

    if (!CSnapshoot::EnumThread(Reset, dwPid, this))
    {
        SET_BREAKPOINT_STATUS(m_nFlag, BREAKPOINT_FORBIDDEN);

        return false;
    }

    m_bSet = FALSE;

    return true;
}


bool CHardBreakPoint::IsSet()
{
    return !IS_BREAKPOINT_FORBIDDEN(m_nFlag) && m_bSet == TRUE;
}


bool CHardBreakPoint::IsMatch(CONTEXT *pContext)
{
    return IsSet() && (pContext->Dr6 & (SIZE_T)(1 << m_dwIndex)) != 0;
}


DWORD CHardBreakPoint::GetIndex()
{
    return m_dwIndex;
}


SIZE_T CHardBreakPoint::GetLength()
{
    return m_dwLen;
}


bool CHardBreakPoint::IsValid()
{
    //下标大于4的话
    if (m_dwIndex >= 4)
    {
        return false;
    }

    //如果执行断点
    if (BREAKPOINT_OPERATE(m_nFlag) != BREAKPOINT_EXECUTE &&
        (m_dwLen > 4 ||                       //大于4字节
        (m_dwLen & (m_dwLen - 1)) != 0 ||     //不是2的n次幂
        ((DWORD)m_pAddr & (m_dwLen - 1)) != 0))//不能被大小整除
    {
        return false;
    }

    return true;
}


bool CHardBreakPoint::Set(CONTEXT *pContext)
{
    //设置drx
    DRX_SET(*pContext, m_dwIndex, m_pAddr);

    //设置dr7
    switch (m_dwIndex)
    {
    case 0:
        DR7_SET(*pContext, 0, BREAKPOINT_OPERATE(m_nFlag), m_dwLen);
        return true;
    case 1:
        DR7_SET(*pContext, 1, BREAKPOINT_OPERATE(m_nFlag), m_dwLen);
        return true;
    case 2:
        DR7_SET(*pContext, 2, BREAKPOINT_OPERATE(m_nFlag), m_dwLen);
        return true;
    case 3:
        DR7_SET(*pContext, 3, BREAKPOINT_OPERATE(m_nFlag), m_dwLen);
        return true;
    default:
        return false;
    }
}


bool CHardBreakPoint::Reset(CONTEXT *pContext)
{
    //设置dr7
    switch (m_dwIndex)
    {
    case 0:
        DR7_RESET(*pContext, 0);
        return true;
    case 1:
        DR7_RESET(*pContext, 1);
        return true;
    case 2:
        DR7_RESET(*pContext, 2);
        return true;
    case 3:
        DR7_RESET(*pContext, 3);
        return true;
    default:
        return false;
    }
}


ENUM_RET_TYPE CHardBreakPoint::Set(THREADENTRY32 *pThreadEntry, LPVOID pParam)
{
    CHardBreakPoint *pHbp = (CHardBreakPoint*)pParam;

    //打开线程
    HTHREAD hThread(pThreadEntry->th32ThreadID);

    if (hThread == NULL)
    {
        return ENUM_RET_FALSE;
    }

    //获取线程环境
    CONTEXT Context = { 0 };
    Context.ContextFlags |= CONTEXT_ALL;
    BOOL bRet = GetThreadContext(hThread, &Context);

    if (!bRet)
    {
        return ENUM_RET_FALSE;
    }

    bRet = pHbp->Set(&Context) &&SetThreadContext(hThread, &Context);

    return bRet == TRUE ? ENUM_CONTINUE : ENUM_RET_FALSE;
}


ENUM_RET_TYPE CHardBreakPoint::Reset(THREADENTRY32 *pThreadEntry, LPVOID pParam)
{
    CHardBreakPoint *pHbp = (CHardBreakPoint*)pParam;

    //打开线程
    HTHREAD hThread(pThreadEntry->th32ThreadID);

    if (hThread == NULL)
    {
        //return ENUM_RET_FALSE;
        return ENUM_CONTINUE;
    }

    //获取线程环境
    CONTEXT Context = { 0 };
    Context.ContextFlags |= CONTEXT_ALL;
    BOOL bRet = GetThreadContext(hThread, &Context);

    if (!bRet)
    {
        //return ENUM_RET_FALSE;
        return ENUM_CONTINUE;
    }

    bRet = pHbp->Reset(&Context) && SetThreadContext(hThread, &Context);

    return ENUM_CONTINUE;
    //return bRet == TRUE ? ENUM_CONTINUE : ENUM_RET_FALSE;
}


//找到一个可用的硬件断点
int CHardBreakPoint::FindFreeReg(CBreakPointList *pLstHbp)
{
    for (int i = 0; i < 4; i++)
    {
        CBreakPointList::iterator itList = pLstHbp->begin();

        for (; itList != pLstHbp->end(); itList++)
        {
            CHardBreakPoint *pHbp = (CHardBreakPoint*)(IBreakPoint*)*itList;

            if (pHbp->m_dwIndex == i)
            {
                break;
            }
        }

        if (itList == pLstHbp->end())
        {
            return i;
        }
    }

    return -1;
}