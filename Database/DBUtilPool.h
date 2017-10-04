#pragma once

#include "../Common.h"
#include "../Util/Lock.h"
#include "DBUtil.h"

typedef CMyArray<CDBUtil*> CDBUtilArrary;

class CDBUtilPool
{
    DECLARE_SINGLETON(CDBUtilPool)
protected:
    CDBUtilPool();
    ~CDBUtilPool();

public:
    bool Create(int nCount);
    void Destory();

    CDBUtil *GetDBUtil();
    void ReturnDBUtil(CDBUtil *pDBUtil);

protected:
    int m_nCount;
    CLock m_lockDBUtil;
    CDBUtilArrary m_aryDBUtil;
};

