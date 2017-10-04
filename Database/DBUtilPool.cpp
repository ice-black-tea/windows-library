#include "DBUtilPool.h"


CDBUtilPool::CDBUtilPool()
    : m_nCount(0)
    , m_aryDBUtil()
{
    CoInitialize(NULL);
}


CDBUtilPool::~CDBUtilPool()
{
    Destory();
    CoUninitialize();
}


bool CDBUtilPool::Create(int nCount)
{
    LOCK_SCOPE(m_lockDBUtil);

    try
    {
        m_nCount = nCount;
        m_aryDBUtil.SetSize(nCount, false);

        for (int i = 0; i < nCount; i++)
        {
            CDBUtil *pDBUtil = new CDBUtil();

            if (!pDBUtil->Open())
            {
                Destory();
                return false;
            }

            m_aryDBUtil.Append(pDBUtil);
        }
    }
    catch (_com_error&)
    {
        Destory();
        return false;
    }

    return true;
}


void CDBUtilPool::Destory()
{
    LOCK_SCOPE(m_lockDBUtil);

    for (int i = 0; i < m_aryDBUtil.GetCount(); i++)
    {
        delete m_aryDBUtil[i];
    }

    m_aryDBUtil.RemoveAll();

    m_nCount = 0;
}


CDBUtil *CDBUtilPool::GetDBUtil()
{
    LOCK_SCOPE(m_lockDBUtil);

    if (!m_aryDBUtil.IsEmpty())
    {
        int nIndex = m_aryDBUtil.GetCount() - 1;
        CDBUtil *pDBUtil = m_aryDBUtil[nIndex];
        m_aryDBUtil.RemoveAt(nIndex);
        return pDBUtil;
    }
    
    return NULL;
}


void CDBUtilPool::ReturnDBUtil(CDBUtil *pDBUtil)
{
    LOCK_SCOPE(m_lockDBUtil);

    if (m_aryDBUtil.GetCount() < m_nCount)
    {
        pDBUtil->RefreshParamter();
        m_aryDBUtil.Append(pDBUtil);
    }
    else
    {
        delete pDBUtil;
    }
}