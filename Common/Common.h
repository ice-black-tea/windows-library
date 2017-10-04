#pragma once

#define COUNT_OF(ary) (sizeof((ary)) / sizeof((ary)[0]))
#define OFFSET_OF(st, mem) ((size_t)&(((st*)NULL)->(mem)))

#ifdef __cplusplus

#define SAFEFREE(ptr) { if ((ptr) != NULL) { delete (ptr); (ptr) = NULL; } }
#define SAFEFREE_ARY(ptr) { if ((ptr) != NULL) { delete[] (ptr); (ptr) = NULL; } }

template <typename T>
class CLocalPtr
{
public:
    CLocalPtr()
        { m_pData = NULL; }
    CLocalPtr(T *pData)
        { m_pData = pData; }
    ~CLocalPtr()
        { SAFEFREE(m_pData); }

public:
    operator T* ()
        { return m_pData; }
    operator const T* () const
        { return m_pData; }
    T* operator-> ()
        { return m_pData; }
    T& operator* ()
        { return *m_pData; }
    T* operator++ ()
        { return ++m_pData; }
    T* operator++ (int)
        { return m_pData++; }
    T* operator-- ()
        { return --m_pData; }
    T* operator-- (int)
        { return m_pData--; }
    bool operator! ()
        { return m_pData == NULL; }
    T** operator& ()
        { return &m_pData; }

protected:
    CLocalPtr(const CLocalPtr<T>& obj) { }
    CLocalPtr<T>& operator= (T *pObj) { }
    CLocalPtr<T>& operator= (const CLocalPtr<T>& obj) { }

protected:
    T *m_pData;
};

template <typename T>
class CLocalAry
{
public:
    CLocalAry()
        { m_pData = NULL; }
    CLocalAry(T *pData)
        { m_pData = pData; }
    CLocalAry(int nCount)
        { m_pData = new T[nCount]; }
    ~CLocalAry()
        { SAFEFREE_ARY(m_pData); }

public:
    operator T* ()
        { return m_pData; }
    operator const T* () const
        { return m_pData; }
    T* operator-> ()
        { return m_pData; }
    T& operator* ()
        { return *m_pData; }
    T* operator++ ()
        { return ++m_pData; }
    T* operator++ (int)
        { return m_pData++; }
    T* operator-- ()
        { return --m_pData; }
    T* operator-- (int)
        { return m_pData--; }
    bool operator! ()
        { return m_pData == NULL; }
    T** operator& ()
        { return &m_pData; }

protected:
    CLocalAry(const CLocalAry<T>& obj) { }
    CLocalAry<T>& operator= (T *pObj) { }
    CLocalAry<T>& operator= (const CLocalAry<T>& obj) { }

protected:
    T *m_pData;
};


class CLocalBuffer : private CLocalAry<char>
{
public:
    CLocalBuffer(int nCount) : CLocalAry<char>(nCount)
        { m_nCount = nCount; if (m_pData != 0) memset(m_pData, 0, m_nCount); }
    ~CLocalBuffer()
        { m_nCount = 0; }
public:
    operator char* ()
        { return m_pData; }
    template <typename Type> operator Type*()
        { return (Type*)m_pData; }
    unsigned int GetSize()
        { return m_nCount; }

protected:
    unsigned int m_nCount;
};


#ifdef WIN32

#include <Windows.h>

//打开进程使用，为了避免打开进程没关
typedef struct tagHPROCESS {
    tagHPROCESS(DWORD dwProcessId, DWORD dwDesiredAccess = PROCESS_ALL_ACCESS, BOOL bInheritHandle = FALSE)
        { m_hProcess = OpenProcess(dwDesiredAccess, bInheritHandle, dwProcessId); }
    ~tagHPROCESS()
        { if (m_hProcess != NULL) { CloseHandle(m_hProcess); m_hProcess = NULL; } }
    operator HANDLE ()
        { return m_hProcess; }
    HANDLE m_hProcess;
}HPROCESS;

//打开线程使用，为了避免打开线程没关
typedef struct tagHTHREAD {
    tagHTHREAD(DWORD dwThreadId, DWORD dwDesiredAccess = THREAD_ALL_ACCESS, BOOL bInheritHandle = FALSE)
        { m_hThread = OpenThread(dwDesiredAccess, bInheritHandle, dwThreadId); }
    ~tagHTHREAD()
        { if (m_hThread != NULL) { CloseHandle(m_hThread); m_hThread = NULL; } }
    operator HANDLE ()
        { return m_hThread;  }
    HANDLE m_hThread;
}HTHREAD;

#endif

#endif