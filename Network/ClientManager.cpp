#include "ClientManager.h"


CClientManager::CClientManager()
{

}


CClientManager::~CClientManager()
{
    RemoveAll();
}


ClientContext* CClientManager::Create()
{
    ClientContext* pContext = NULL;

    LOCK_SCOPE(m_lockList);

    if (!m_lstFreeContexts.IsEmpty())
    {
        pContext = m_lstFreeContexts.GetHead();
        m_lstFreeContexts.RemoveHead();
    }
    else
    {
        pContext = new ClientContext;
    }

    return pContext;
}


ClientContext* CClientManager::Find(AccessToken& Token)
{
    LOCK_SCOPE(m_lockList);

    ClientContext *pContext = NULL;
    m_mapContexts.Lookup(Token, pContext);

    return pContext;
}


void CClientManager::Insert(ClientContext* pContext)
{
    LOCK_SCOPE(m_lockList);

    m_mapContexts.SetAt(pContext->m_Token, pContext);
}


void CClientManager::Remove(ClientContext* pContext)
{
    LOCK_SCOPE(m_lockList);

    ClientContext* pContextTemp = NULL;

    if (m_mapContexts.Lookup(pContext->m_Token, pContextTemp) &&
        pContextTemp == pContext)
    {
        pContext->m_Socket.Close();
        pContext->m_RecvBuff.Clear();
        pContext->m_SendBuff.Clear();
        m_mapContexts.RemoveKey(pContext->m_Token);
        m_lstFreeContexts.AddTail(pContext);
    }
}


void CClientManager::RemoveAll()
{
    LOCK_SCOPE(m_lockList);

    ContextMap::iterator itMap = m_mapContexts.begin();
    for (; itMap != m_mapContexts.end(); itMap++)
    {
        delete itMap->m_Value;
    }

    ContextList::iterator itList = m_lstFreeContexts.begin();
    for (; itList != m_lstFreeContexts.end(); itList++)
    {
        delete *itList;
    }

    m_mapContexts.RemoveAll();
    m_lstFreeContexts.RemoveAll();
}