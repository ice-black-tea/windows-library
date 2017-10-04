#pragma once

#include "../Common.h"
#include "FileUtil.h"

/************************************************************************/
/*                              CFileEdit                               */
/************************************************************************/

class CFileEdit : public CFileUtil
{
public:
    CFileEdit();
    virtual ~CFileEdit();

public:
    bool Read(int nOffset = 0, size_t nSize = 0); //读取文件
    bool Write(int nOffset = 0, size_t nSize = 0); //保存文件

    size_t GetSize(); //缓冲区可用大小
    size_t GetTotalSize(); //缓冲区总大小

    void Swap(CBuffer& Buffer); //交换所有缓冲区
    bool IsValid(size_t nOffset); //偏移是否有效
    bool IsValid(const void *pData); //缓冲区是否有效
    void *GetBuffer(size_t nOffset, size_t nLen = 0); //获取可读缓冲区，nLen为0不指定长度
    void *SetBuffer(size_t nOffset, size_t nLen); //获取可写缓冲区

    size_t GetOffset(const void *pData); //获取偏移
    size_t GetMaxLen(const void *pData); //获取最大长度

protected:
    CBuffer m_Buffer;
};



/************************************************************************/
/*                             CFileEditPtr                             */
/************************************************************************/

template <typename T>
class CFileEditPtr
{
public:
    CFileEditPtr(CFileEdit *pFile);
    ~CFileEditPtr();

public:
    size_t GetSize() const;
    size_t GetOffset() const;
    size_t GetEndOffset() const;

    void SetOffset(size_t nOffset);
    void SetOffset(const void *pData);
    void SetMemory(int chDest = 0);

    operator T* ();
    T* operator-> ();
    bool operator! ();
    bool operator== (const void *pData);
    bool operator!= (const void *pData);

protected:
    size_t     m_nOffset;
    CFileEdit *m_pFile;

#ifdef _DEBUG
    T*       m_pData;
#endif
};


/************************************************************************/
/*                             CFileEditAry                              */
/************************************************************************/

template <typename T>
class CFileEditAry
{
public:
    CFileEditAry(CFileEdit *pFile);
    ~CFileEditAry();

public:
    int GetCount() const;
    size_t GetSize() const;
    size_t GetOffset() const;
    size_t GetEndOffset() const;

    void SetCount(int nCount);
    void SetOffset(size_t nOffset);
    void SetOffset(const void *pData);
    void SetMemory(int chDest = 0);

    operator T* ();
    T& operator[] (int nIndex);
    bool operator! ();
    bool operator== (const void *pData);
    bool operator!= (const void *pData);

protected:
    int        m_nCount;
    size_t     m_nOffset;
    CFileEdit *m_pFile;

#ifdef _DEBUG
    T*       m_pData;
#endif
};



/************************************************************************/
/*                              CFileEdit                               */
/************************************************************************/

inline CFileEdit::CFileEdit()
{

}

inline CFileEdit::~CFileEdit()
{

}

//读取pe文件
inline bool CFileEdit::Read(int nOffset, size_t nSize)
{
    assert(IsOpen());

    m_Buffer.Clear();

    nSize = nSize == 0 ? SeekToEnd() - nOffset : nSize;

    if (CFileUtil::Read(nOffset, m_Buffer.Write(nSize), nSize) == 0)
    {
        m_Buffer.Clear();
        return false;
    }

    return true;
}

//保存pe文件
inline bool CFileEdit::Write(int nOffset, size_t nSize)
{
    assert(IsOpen());

    nSize = nSize == 0 ? m_Buffer.GetReadableSize() : nSize;

    return CFileUtil::Write(nOffset, m_Buffer, nSize) == m_Buffer.GetReadableSize();
}

//缓冲区总大小
inline size_t CFileEdit::GetTotalSize()
{
    return m_Buffer.GetTotalSize();
}

//缓冲区当前大小
inline size_t CFileEdit::GetSize()
{
    return m_Buffer.GetReadableSize();
}

//获取可写缓冲区
inline void CFileEdit::Swap(CBuffer& Buffer)
{
    m_Buffer.Swap(Buffer);
}

//偏移是否有效
inline bool CFileEdit::IsValid(size_t nOffset)
{
    return nOffset < GetTotalSize();
}

//缓冲区是否有效
inline bool CFileEdit::IsValid(const void *pData)
{
    return GetOffset(pData) < GetTotalSize();
}

//获取可读缓冲区
inline void *CFileEdit::GetBuffer(size_t nOffset, size_t nLen)
{
    if (nOffset < m_Buffer.GetReadableSize() &&
        (nLen == 0 || nOffset + nLen <= m_Buffer.GetReadableSize()))
    {
        return (char*)m_Buffer + nOffset;
    }

    return NULL;
}

//获取可写缓冲区
inline void *CFileEdit::SetBuffer(size_t nOffset, size_t nLen)
{
    assert(nOffset <= nOffset + nLen);

    if (nOffset + nLen > m_Buffer.GetReadableSize())
    {
        m_Buffer.SetBufferSize(nOffset + nLen);
    }

    return (char*)m_Buffer + nOffset;
}

//获取指针对于的文件偏移
inline size_t CFileEdit::GetOffset(const void *pData)
{
    return (char*)pData - (char*)m_Buffer;
}

//获取指定位置处最大长度
inline size_t CFileEdit::GetMaxLen(const void *pData)
{
    if (pData >= m_Buffer && pData < (char*)m_Buffer + m_Buffer.GetReadableSize())
    {
        return (char*)m_Buffer + m_Buffer.GetReadableSize() - (char*)pData;
    }

    return 0;
}

/************************************************************************/
/*                            CFileEditPtr                              */
/************************************************************************/

template <typename T>
inline CFileEditPtr<T>::CFileEditPtr(CFileEdit *pFile)
    : m_nOffset(-1)
    , m_pFile(pFile)
{

}

template <typename T>
inline CFileEditPtr<T>::~CFileEditPtr()
{
    //assert(m_pFile != NULL);
}

template <typename T>
inline size_t CFileEditPtr<T>::GetSize() const
{
    return sizeof(T);
}

template <typename T>
inline size_t CFileEditPtr<T>::GetOffset() const
{
    assert(m_pFile != NULL);
    assert(GetEndOffset() <= m_pFile->GetTotalSize());

    return m_nOffset;
}

template <typename T>
inline size_t CFileEditPtr<T>::GetEndOffset() const
{
    assert(m_pFile != NULL);

    return m_nOffset + GetSize();
}

template <typename T>
inline void CFileEditPtr<T>::SetOffset(size_t nOffset)
{
    assert(m_pFile != NULL);

    m_nOffset = nOffset;
    m_pFile->SetBuffer(0, GetEndOffset());

#ifdef _DEBUG
    m_pData = *this;
#endif // _DEBUG
}

template <typename T>
inline void CFileEditPtr<T>::SetOffset(const void *pData)
{
    assert(m_pFile != NULL);

    pData != NULL ? SetOffset(m_pFile->GetOffset(pData)) : m_nOffset = -1;

#ifdef _DEBUG
    m_pData = *this;
#endif // _DEBUG
}

template <typename T>
inline void CFileEditPtr<T>::SetMemory(int chDest)
{
    assert(m_pFile != NULL);

    memset(*this, chDest, GetSize());
}

template <typename T>
inline CFileEditPtr<T>::operator T* ()
{
    assert(m_pFile != NULL);
    assert(GetEndOffset() <= m_pFile->GetTotalSize());

    return *this != NULL ? (T*)m_pFile->GetBuffer(m_nOffset) : NULL;
}

template <typename T>
inline T* CFileEditPtr<T>::operator-> ()
{
    assert(m_pFile != NULL);
    assert(GetEndOffset() <= m_pFile->GetTotalSize());

    return (T*)m_pFile->GetBuffer(m_nOffset);
}

template <typename T>
inline bool CFileEditPtr<T>::operator! ()
{
    return m_nOffset == -1;
}

template <typename T>
inline bool CFileEditPtr<T>::operator== (const void *pData)
{
    return pData != NULL ? m_nOffset == m_pFile->GetOffset(pData) : m_nOffset == -1;
}

template <typename T>
inline bool CFileEditPtr<T>::operator!= (const void *pData)
{
    return pData != NULL ? m_nOffset != m_pFile->GetOffset(pData) : m_nOffset != -1;
}

/************************************************************************/
/*                            CFileEditAry                              */
/************************************************************************/

template <typename T>
inline CFileEditAry<T>::CFileEditAry(CFileEdit *pFile)
    : m_nOffset(-1)
    , m_nCount(0)
    , m_pFile(pFile)
{

}

template <typename T>
inline CFileEditAry<T>::~CFileEditAry()
{
    //assert(m_pFile != NULL);
}

template <typename T>
inline size_t CFileEditAry<T>::GetSize() const
{
    return m_nCount * sizeof(T);
}

template <typename T>
inline size_t CFileEditAry<T>::GetOffset() const
{
    assert(m_pFile != NULL);
    assert(GetEndOffset() <= m_pFile->GetTotalSize());

    return m_nOffset;
}

template <typename T>
inline size_t CFileEditAry<T>::GetEndOffset() const
{
    assert(m_pFile != NULL);

    return m_nOffset + GetSize();
}

template <typename T>
inline void CFileEditAry<T>::SetOffset(size_t nOffset)
{
    assert(m_pFile != NULL);

    m_nOffset = nOffset;
    m_pFile->SetBuffer(0, GetEndOffset());

#ifdef _DEBUG
    m_pData = *this;
#endif // _DEBUG
}

template <typename T>
inline void CFileEditAry<T>::SetOffset(const void *pData)
{
    assert(m_pFile != NULL);

    pData != NULL ? SetOffset(m_pFile->GetOffset(pData)) : m_nOffset = -1;

#ifdef _DEBUG
    m_pData = *this;
#endif // _DEBUG
}

template <typename T>
inline int CFileEditAry<T>::GetCount() const
{
    return m_nCount;
}

template <typename T>
inline void CFileEditAry<T>::SetCount(int nCount)
{
    assert(m_pFile != NULL);

    m_nCount = nCount;
    m_pFile->SetBuffer(0, GetEndOffset());
}

template <typename T>
inline void CFileEditAry<T>::SetMemory(int chDest)
{
    assert(m_pFile != NULL);

    memset(*this, chDest, GetSize());
}

template <typename T>
inline CFileEditAry<T>::operator T* ()
{
    assert(m_pFile != NULL);
    assert(GetEndOffset() <= m_pFile->GetTotalSize());

    return *this != NULL ? (T*)m_pFile->GetBuffer(m_nOffset) : NULL;
}

template <typename T>
inline T& CFileEditAry<T>::operator[] (int nIndex)
{
    assert(m_pFile != NULL);
    assert(*this != NULL);

    if (nIndex >= m_nCount)
    {
        SetCount(nIndex + 1);
    }

    return ((T*)m_pFile->GetBuffer(m_nOffset))[nIndex];
}

template <typename T>
inline bool CFileEditAry<T>::operator! ()
{
    return m_nOffset == -1;
}

template <typename T>
inline bool CFileEditAry<T>::operator== (const void *pData)
{
    return pData != NULL ? m_nOffset == m_pFile->GetOffset(pData) : m_nOffset == -1;
}

template <typename T>
inline bool CFileEditAry<T>::operator!= (const void *pData)
{
    return pData != NULL ? m_nOffset != m_pFile->GetOffset(pData) : m_nOffset != -1;
}