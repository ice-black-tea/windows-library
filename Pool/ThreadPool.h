#pragma once

#include "../Common.h"
#include "ThreadCmd.h"

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
    bool Excute(IThreadCmd* pCmd);
    static unsigned int __stdcall ThreadProc(void* lpParam);

protected:
    int     m_nCount;
    HANDLE* m_phThread;
    HANDLE  m_hEvent;
    HANDLE  m_hSemaphore;
    CThreadCmdManager m_CmdMgr;
};


inline bool CThreadPool::IsRunning()
{
    return m_hEvent != NULL && WaitForSingleObject(m_hEvent, 0) == WAIT_OBJECT_0;
}