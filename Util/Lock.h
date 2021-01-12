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
    CLock(const CLock& obj);

private:
    CRITICAL_SECTION m_cs;
};


class CLockAspect : IAspect<CLock>
{
public:
    CLockAspect(CLock *pLock) : IAspect<CLock>(pLock){ m_pObject->Lock(); }
    ~CLockAspect() { m_pObject->Unlock(); }

private:
    CLockAspect(const CLockAspect& obj);
};


class CAtomicInteger
{
public:
    CAtomicInteger() : m_nValue(0) { }
    CAtomicInteger(int nValue) : m_nValue(nValue) { }

public:
    int AddAndGet(int delta) { LOCK_SCOPE(m_lock); m_nValue += delta; return m_nValue; }
    int GetAndAdd(int delta) { LOCK_SCOPE(m_lock); int nResult = m_nValue; m_nValue += delta; return nResult; }
    int Get() { LOCK_SCOPE(m_lock); return m_nValue; }
    void Clear() { LOCK_SCOPE(m_lock); m_nValue = 0; }

private:
    CAtomicInteger(const CAtomicInteger& obj) { }

private:
    CLock m_lock;
    int m_nValue;
};

#endif // WIN32