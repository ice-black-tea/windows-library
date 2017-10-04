#pragma once

#ifdef WIN32
#include <windows.h>
#include "../Common.h"

#define LOCK_SCOPE(lock) CLockAspect theScopeLock(&lock)

class CLock
{
public:
    CLock() { InitializeCriticalSection(&m_cs); }
    ~CLock(){ DeleteCriticalSection(&m_cs); }
    void Lock() { EnterCriticalSection(&m_cs); }
    void Unlock() { LeaveCriticalSection(&m_cs); }

private:
    CRITICAL_SECTION m_cs;
};


class CLockAspect : IAspect<CLock>
{
public:
    CLockAspect(CLock *pLock) : IAspect<CLock>(pLock){ m_pObject->Lock(); }
    ~CLockAspect() { m_pObject->Unlock(); }
};

#endif // WIN32