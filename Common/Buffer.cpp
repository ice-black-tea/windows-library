#include <string.h>
#include <math.h>
#include "Buffer.h"

#ifdef _WIN32
#include <Windows.h>
#endif

CBuffer::CBuffer()
    : m_nSize(0)
    , m_pBase(NULL)
    , m_pWrite(NULL)
    , m_pRead(NULL)
{

}


CBuffer::CBuffer(size_t nSize)
    : m_nSize(0)
    , m_pBase(NULL)
    , m_pWrite(NULL)
    , m_pRead(NULL)
{
    Alloc(nSize);
}


CBuffer::CBuffer(const CBuffer& buffer)
{
    Copy(buffer);
}


CBuffer::~CBuffer()
{
    Clear();
}


CBuffer::operator void* ()
{
    return m_pRead;
}


CBuffer::operator char* ()
{
    return (char*)m_pRead;
}


CBuffer::operator wchar_t* ()
{
    return (wchar_t*)m_pRead;
}


bool CBuffer::IsEmpty() const
{
    return GetReadableSize() == 0;
}


size_t CBuffer::GetWritableSize() const
{
    return m_pBase + m_nSize - m_pRead;
}


size_t CBuffer::GetReadableSize() const
{
    return m_pWrite - m_pRead;
}


size_t CBuffer::GetTotalSize() const
{
    return m_nSize;
}


void *CBuffer::GetBuffer(int nPos) const
{
    return m_pRead + nPos;
}


void CBuffer::SetBufferSize(size_t nSize)
{
    if (nSize > GetWritableSize())
        Alloc(nSize);

    m_pWrite = m_pRead + nSize;
}


void *CBuffer::Write(size_t nSize)
{
    size_t nNewSize = nSize + GetReadableSize();

    if (nNewSize > GetWritableSize())
    {
        Alloc(nNewSize);
    }

    void *pWrite = m_pWrite;

    m_pWrite += nSize;

    return pWrite;
}


void *CBuffer::Read(size_t nSize)
{
    if (nSize > GetReadableSize())
    {
        nSize = GetReadableSize();
    }

    void *pRead = m_pRead;

    m_pRead += nSize;

    return pRead;
}


void CBuffer::Delete(size_t nSize)
{
    if (nSize > GetReadableSize())
    {
        nSize = GetReadableSize();
    }

    m_pRead += nSize;
}


size_t CBuffer::Write(const void* pData, size_t nSize)
{
    size_t nNewSize = nSize + GetReadableSize();

    if (nNewSize > GetWritableSize())
    {
        Alloc(nNewSize);
    }

    memmove(m_pWrite, pData, nSize);

    m_pWrite += nSize;

    return nSize;
}


size_t CBuffer::Insert(const void* pData, size_t nSize)
{
    if ((size_t)(m_pRead - m_pBase) >= nSize)
    {
        memmove(m_pRead - nSize, pData, nSize);
        m_pRead -= nSize;

        return nSize;
    }

    size_t nNewSize = nSize + GetReadableSize();

    if (nNewSize > GetWritableSize())
    {
        Alloc(nSize + GetReadableSize());
    }

    memmove(m_pRead + nSize, m_pRead, GetReadableSize());
    memmove(m_pRead, pData, nSize);

    m_pWrite += nSize;

    return nSize;
}


size_t CBuffer::Read(void* pData, size_t nSize)
{
    if (nSize > GetReadableSize())
    { 
        nSize = GetReadableSize();
    }

    if (nSize != 0)
    {
        memmove(pData, m_pRead, nSize);
        m_pRead += nSize;
    }

    return nSize;
}


bool CBuffer::StartWith(const void *pData, size_t nSize)
{
    if (nSize <= GetReadableSize())
    {
        return memcmp(pData, m_pRead, nSize) == 0;
    }

    return false;
}


bool CBuffer::EndWith(const void *pData, size_t nSize)
{
    if (nSize <= GetReadableSize())
    {
        return memcmp(pData, m_pWrite - nSize - 1, nSize) == 0;
    }

    return false;
}


size_t CBuffer::Alloc(size_t nSize)
{
    size_t nBufSize = GetReadableSize();

    if (nSize <= nBufSize)
    {
        return 0;
    }

    size_t nNewSize = (size_t)ceil(1.0 * nSize / BUFFER_GROW_SIZE) * BUFFER_GROW_SIZE;

#ifdef _WIN32
    char* pNewBuf = (char*)VirtualAlloc(NULL, nNewSize, MEM_COMMIT, PAGE_READWRITE);;
#else
    char* pNewBuf = new char[nNewSize];
    memset(pNewBuf, 0, nNewSize);
#endif

    if (pNewBuf == NULL)
    {
        m_pBase = m_pRead = m_pWrite = NULL;
        return 0;
    }
    
    if (nBufSize > 0)
    {
        memmove(pNewBuf, m_pRead, nBufSize);
    }

    if (m_pBase != NULL)
    {
#ifdef _WIN32
        VirtualFree(m_pBase, 0, MEM_RELEASE);
#else
        delete[] m_pBase;
#endif
    }

    m_pBase = m_pRead = pNewBuf;
    m_pWrite = m_pRead + nBufSize;

    m_nSize = nNewSize;

    return m_nSize;
}


void CBuffer::Clear()
{
    if (m_pBase != NULL)
    {
#ifdef _WIN32
        VirtualFree(m_pBase, 0, MEM_RELEASE);
#else
        delete[] m_pBase;
#endif
    }

    m_pBase = m_pWrite = m_pRead = NULL;
    m_nSize = 0;
}


void CBuffer::Swap(CBuffer& buffer)
{
    char *pBase = buffer.m_pBase;
    char *pWrite = buffer.m_pWrite;
    char *pRead = buffer.m_pRead;
    size_t nSize = buffer.m_nSize;

    buffer.m_pBase = m_pBase;
    buffer.m_pWrite = m_pWrite;
    buffer.m_pRead = m_pRead;
    buffer.m_nSize = m_nSize;

    m_pBase = pBase;
    m_pWrite = pWrite;
    m_pRead = pRead;
    m_nSize = nSize;
}


void CBuffer::Copy(const CBuffer& buffer)
{
    size_t nBufSize = buffer.GetReadableSize();

    if (nBufSize > m_nSize)
    {
        Clear();
        Alloc(nBufSize);
    }

    memmove(m_pBase, buffer.m_pRead, nBufSize);

    m_pRead = m_pBase;
    m_pWrite = m_pBase + nBufSize;
}

