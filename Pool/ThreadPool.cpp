#include <process.h>
#include "ThreadPool.h"

const int MAX_SEMAPHORE = 0x7FFFFFFF;


CThreadPool::CThreadPool()
    : m_nCount(0)
    , m_phThread(NULL)
    , m_hEvent(NULL)
    , m_hSemaphore(NULL)
{
}


CThreadPool::~CThreadPool()
{
    Destory();
}


bool CThreadPool::Create(int nCount)
{
    bool bRet = false;

    if (nCount <= 0)
    {
        return false;
    }

    m_nCount = nCount;

    //创建退出事件
    m_hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (m_hEvent == NULL)
    {
        goto EXIT_LABLE;
    }

    //创建信号量
    m_hSemaphore = CreateSemaphore(NULL, 0, MAX_SEMAPHORE, NULL);
    if (m_hSemaphore == NULL)
    {
        goto EXIT_LABLE;
    }

    //线程数组
    m_phThread = new HANDLE[m_nCount];
    if (m_phThread == NULL)
    {
        goto EXIT_LABLE;
    }

    //创建线程
    for (int i = 0; i < m_nCount; i++)
    {
        m_phThread[i] = (HANDLE)_beginthreadex(
            NULL, 0, ThreadProc, this, 0, NULL);

        if (m_phThread[i] == INVALID_HANDLE_VALUE)
        {
            m_nCount = i;
            goto EXIT_LABLE;
        }
    }

    bRet = true;

EXIT_LABLE:

    if (!bRet)
    {
        Destory();
    }

    return bRet;
}


void CThreadPool::Destory()
{
    if (m_hEvent != NULL)
    {
        //通知线程退出
        SetEvent(m_hEvent);

        if (m_phThread != NULL && m_nCount > 0)
        {
            //等待线程退出
            DWORD dwRet = WaitForMultipleObjects(
                m_nCount, m_phThread, TRUE, INFINITE);

            if (dwRet != WAIT_FAILED)
            {
                for (int i = 0; i < m_nCount; i++)
                {
                    CloseHandle(m_phThread[i]);
                }
            }
        }

        CloseHandle(m_hEvent);
        m_hEvent = NULL;
    }

    if (m_hSemaphore != NULL)
    {
        CloseHandle(m_hSemaphore);
        m_hSemaphore = NULL;
    }

    if (m_phThread != NULL)
    {
        delete[] m_phThread;
        m_phThread = NULL;
    }
}


bool CThreadPool::Excute(IThreadCmd* pCmd)
{
    m_CmdMgr.Insert(pCmd);

    ReleaseSemaphore(m_hSemaphore, 1, NULL);

    return true;
}


enum
{
    ID_EVENT,
    ID_SEMAPHORE,
    OBJECT_COUNT
};


unsigned int __stdcall CThreadPool::ThreadProc(void* lpParam)
{
    CThreadPool* pThreadPool = (CThreadPool*)lpParam;

    HANDLE arySyncObj[OBJECT_COUNT] = {
        pThreadPool->m_hEvent,
        pThreadPool->m_hSemaphore
    };

    while (true)
    {
        DWORD dwRet = WaitForMultipleObjects(
            OBJECT_COUNT, arySyncObj, FALSE, INFINITE);

        //等待出错或超时退出则线程
        if (dwRet == WAIT_FAILED || dwRet == WAIT_TIMEOUT)
        {
            return 0;
        }

        int nIndex = dwRet - WAIT_OBJECT_0;

        //事件对象被触发退出线程
        if (nIndex == ID_EVENT)
        {
            return 0;
        }

        //执行操作
        IThreadCmd* pCmd = pThreadPool->m_CmdMgr.GetCommand();

        if (pCmd != NULL)
        {
            try
            {
                pCmd->Excute();
            }
            catch (...)
            {
            	
            }
            delete pCmd;
        }
    }

    return 0;
}
