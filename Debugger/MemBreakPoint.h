#pragma once

#include "BreakPoint.h"

//内存保护属性
typedef struct _MEMORY_PROTECT_INFO {
    LPVOID m_pAddr;
    SIZE_T m_dwLen;
    MEMORY_BASIC_INFORMATION *m_pMemInfo;
}MEMORY_PROTECT_INFO, *PMEMORY_PROTECT_INFO;

//内存属性链表
typedef CMyList<MEMORY_BASIC_INFORMATION> CMemInfoList;

//内存保护属性链表
typedef CMyList<MEMORY_PROTECT_INFO> CMemProtectList;


class CMemBreakPoint
    : public IBreakPoint
{
public:
    CMemBreakPoint(LPVOID pAddr, SIZE_T dwLen,
        CMemInfoList *plstMemInfo, int nType, int nStatus);

   virtual ~CMemBreakPoint();

public:
    virtual int GetType();
    virtual int GetStatus();
    virtual int GetOperate();
    virtual LPVOID GetAddress();

    virtual bool IsSet();
    virtual bool Set(DWORD dwPid);
    virtual bool Reset(DWORD dwPid);

    bool InRegion(PEXCEPTION_RECORD pExepRec);
    bool IsMatch(PEXCEPTION_RECORD pExcpRec);
    SIZE_T GetLength();
    DWORD GetProtect();
    CMemProtectList& GetProtectList();

protected:
    INT             m_nFlag;
    BOOL            m_bSet;          //是否设置断点
    LPVOID          m_pAddr;         //断点地址
    SIZE_T          m_dwLen;         //断点大小
    DWORD           m_dwProtect;     //保护属性
    CMemInfoList   *m_pLstMemInfo;   //内存信息链表
    CMemProtectList m_lstMemProtect; //内存保护属性
};