#pragma once

#include "../Common/HashMap.h"
#include "../Common/List.h"
#include "ClientContext.h"

typedef CMyList<ClientContext*> ContextList;
typedef CHashMap<AccessToken, ClientContext*> ContextMap;

class CClientManager
{
public:
    CClientManager();
    ~CClientManager();

    ClientContext* Create();
    ClientContext* Find(AccessToken& Token);
    void Insert(ClientContext* pContext);
    void Remove(ClientContext* pContext);
    void RemoveAll();

    inline int GetCount();

protected:
    CLock       m_lockList;
    ContextMap  m_mapContexts;
    ContextList m_lstFreeContexts;
};


inline int CClientManager::GetCount()
{
    return m_mapContexts.GetCount();
}
