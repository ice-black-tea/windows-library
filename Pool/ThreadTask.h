#pragma once

#include <windows.h>
#include "../Common/List.h"
#include "../Common/SmartPtr.h"
#include "../Util/Lock.h"

class IThreadTask;
typedef CMyList<CSmartPtr<IThreadTask>> ThreadTaskList;

/************************************************************************/
/*                                任务                                  */
/************************************************************************/
class IThreadTask: public CRefCnt
{
public:
    IThreadTask();
    virtual ~IThreadTask();

public:
    virtual bool Excute() = 0;
};

/************************************************************************/
/*                              任务管理                                */
/************************************************************************/
class CThreadTaskManager
{
public:
    CThreadTaskManager();
    ~CThreadTaskManager();

public:
    bool IsEmpty();
    int GetCount();
    bool Insert(CSmartPtr<IThreadTask> lpTask); //插入命令
    CSmartPtr<IThreadTask> GetNext(); //获取下一条命令
    void Clear();

protected:
    CLock m_lock;
    ThreadTaskList m_lstTask;
};



/************************************************************************/
/*                                实现                                  */
/************************************************************************/
inline IThreadTask::IThreadTask()
{
}

inline IThreadTask::~IThreadTask()
{
}


inline CThreadTaskManager::CThreadTaskManager()
{
    
}

inline CThreadTaskManager::~CThreadTaskManager()
{
    Clear();
}

inline bool CThreadTaskManager::IsEmpty()
{
    LOCK_SCOPE(m_lock);

    return m_lstTask.IsEmpty();
}

inline int CThreadTaskManager::GetCount()
{
    LOCK_SCOPE(m_lock);

    return m_lstTask.GetCount();
}

inline bool CThreadTaskManager::Insert(CSmartPtr<IThreadTask> pTask)
{
    LOCK_SCOPE(m_lock);

    if (pTask != NULL)
    {
        m_lstTask.AddTail(pTask);
    }

    return true;
}

inline CSmartPtr<IThreadTask> CThreadTaskManager::GetNext()
{
    LOCK_SCOPE(m_lock);

    CSmartPtr<IThreadTask> pTask = NULL;

    if (!m_lstTask.IsEmpty())
    {
        pTask = m_lstTask.GetHead();
        m_lstTask.RemoveHead();
    }

    return pTask;
}

inline void CThreadTaskManager::Clear()
{
    LOCK_SCOPE(m_lock);

    m_lstTask.RemoveAll();
}