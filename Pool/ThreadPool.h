#pragma once

#include "../Common.h"
#include "ThreadTask.h"

class CThreadPool
{
    DECLARE_SINGLETON(CThreadPool)
protected:
    CThreadPool();
    ~CThreadPool();

public:
    bool IsRunning();
    bool Create(int nCount);
    void Destory();
    bool PostTask(CSmartPtr<IThreadTask> lpTask);
    static unsigned int __stdcall ThreadProc(void* lpParam);

protected:
    int     m_nCount;
    HANDLE* m_lpThread;
    HANDLE  m_hEvent;
    HANDLE  m_hSemaphore;
    CThreadTaskManager m_TaskManager;
};


inline bool CThreadPool::IsRunning()
{
    return m_hEvent != NULL && WaitForSingleObject(m_hEvent, 0) == WAIT_TIMEOUT;
}
