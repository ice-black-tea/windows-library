#pragma once

#include <windows.h>
#include "../Common/List.h"
#include "../Util/Lock.h"

class IThreadCmd;
typedef CMyList<IThreadCmd*> ThreadCmdList;

/************************************************************************/
/*                                命令                                  */
/************************************************************************/
class IThreadCmd
{
public:
    IThreadCmd();
    virtual ~IThreadCmd();

public:
    virtual bool Excute() = 0;
};

/************************************************************************/
/*                              命令管理                                */
/************************************************************************/
class CThreadCmdManager
{
public:
    CThreadCmdManager();
    ~CThreadCmdManager();

public:
    bool Insert(IThreadCmd* pCmd); //插入命令
    IThreadCmd* GetCommand(); //获取下一条命令

protected:
    CLock m_lockList;
    ThreadCmdList m_lstCmd;
};



/************************************************************************/
/*                                实现                                  */
/************************************************************************/
inline IThreadCmd::IThreadCmd()
{
}

inline IThreadCmd::~IThreadCmd()
{
}
    

inline CThreadCmdManager::CThreadCmdManager()
{
    
}

inline CThreadCmdManager::~CThreadCmdManager()
{
    LOCK_SCOPE(m_lockList);

    ThreadCmdList::iterator itList = m_lstCmd.begin();

    for (; itList != m_lstCmd.end(); itList++)
    {
        delete *itList;
    }

    m_lstCmd.RemoveAll();
}

inline bool CThreadCmdManager::Insert(IThreadCmd* pCmd)
{
    LOCK_SCOPE(m_lockList);

    if (pCmd != NULL)
    {
        m_lstCmd.AddTail(pCmd);
    }

    return true;
}

inline IThreadCmd* CThreadCmdManager::GetCommand()
{
    LOCK_SCOPE(m_lockList);

    IThreadCmd* pCmd = NULL;

    if (!m_lstCmd.IsEmpty())
    {
        pCmd = m_lstCmd.GetHead();
        m_lstCmd.RemoveHead();
    }

    return pCmd;
}