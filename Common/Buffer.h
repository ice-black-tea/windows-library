#pragma once

#include "../StringT.h"

const size_t BUFFER_GROW_SIZE = 0x1000;

class CBuffer
{
public:
    CBuffer();
    CBuffer(size_t nSize);
    ~CBuffer();

protected:
    CBuffer(const CBuffer& buffer);

public:
    operator void* ();
    operator char* ();
    operator wchar_t* ();

    bool IsEmpty() const; //已写入数据是否为空

    size_t GetWritableSize() const; //可写空间大小， m_pBase + m_nSize - m_pRead
    size_t GetReadableSize() const; //可读空间大小， m_pWrite - m_pRead
    size_t GetTotalSize() const; //总分配空间大小， m_pBase + m_nSize

    void *GetBuffer(int nPos = 0) const; //获取缓存区数据
    void SetBufferSize(size_t nSize); //设置数据空间大小
    
    void* Write(size_t nSize); //分配指定大小写缓冲区
    void* Read(size_t nSize); //读指定大小缓冲区
    void  Delete(size_t nSize); //删除指定大小缓冲区（等同与Read）

    size_t Write(const void* pData, size_t nSize);
    size_t Insert(const void* pData, size_t nSize);
    size_t Read(void* pData, size_t nSize);

    bool StartWith(const void *pData, size_t nSize);
    bool EndWith(const void *pData, size_t nSize);

    size_t Alloc(size_t nSize);
    void Swap(CBuffer& buffer);
    void Copy(const CBuffer& buffer);
    void Clear();

protected:
    char*	m_pBase;
    char*	m_pWrite;
    char*   m_pRead;
    size_t	m_nSize;
};


template <typename T>
inline CBuffer& operator<< (CBuffer& buffer, const T& data)
{
    buffer.Write(&data, sizeof(data));

    return buffer;
}


template <typename T>
inline CBuffer& operator>> (CBuffer& buffer, T& data)
{
    if (buffer.GetReadableSize() >= sizeof(data))
        buffer.Read(&data, sizeof(data));

    return buffer;
}


template <>
inline CBuffer& operator<< (CBuffer& buffer, const CMyStringA& strData)
{
    int nLen = strData.GetLength() * sizeof(char);

    buffer.Write(&nLen, sizeof(nLen));
    buffer.Write(strData, nLen);

    return buffer;
}


template <>
inline CBuffer& operator>> (CBuffer& buffer, CBuffer& data)
{
    int nLen = 0;

    if (buffer.GetReadableSize() >= sizeof(nLen))
    {
        nLen = *(int*)buffer.GetBuffer(0);
    }

    if (buffer.GetReadableSize() >= sizeof(nLen) + (size_t)nLen)
    {
        buffer.Delete(sizeof(nLen));
        data.Write(buffer.Read(nLen), nLen);
    }

    return buffer;
}


template <>
inline CBuffer& operator<< (CBuffer& buffer, const CBuffer& data)
{
    int nLen = data.GetReadableSize();

    buffer.Write(&nLen, sizeof(nLen));
    buffer.Write(((CBuffer&)data).Read(nLen), nLen);

    return buffer;
}


inline CBuffer& operator<< (CBuffer& buffer, const char *pszData)
{
    int nLen = strlen(pszData);

    buffer.Write(&nLen, sizeof(nLen));
    buffer.Write(pszData, nLen);

    return buffer;
}


inline CBuffer& operator<< (CBuffer& buffer, const wchar_t *pszData)
{
    int nLen = wcslen(pszData);

    buffer.Write(&nLen, sizeof(nLen));
    buffer.Write(pszData, nLen);

    return buffer;
}

template <>
inline CBuffer& operator>> (CBuffer& buffer, CMyStringA& strData)
{
    int nLen = 0;

    if (buffer.GetReadableSize() >= sizeof(nLen))
    {
        nLen = *(int*)buffer.GetBuffer(0);
    }

    if (buffer.GetReadableSize() >= sizeof(nLen) + (size_t)nLen)
    {
        buffer.Delete(sizeof(nLen));
        strData = CMyStringA((char*)buffer.Read(nLen), nLen / sizeof(char));
    }

    return buffer;
}


template <>
inline CBuffer& operator<< (CBuffer& buffer, const CMyStringW& strData)
{
    int nLen = strData.GetLength() * sizeof(wchar_t);

    buffer.Write(&nLen, sizeof(nLen));
    buffer.Write(strData, nLen);

    return buffer;
}


template <>
inline CBuffer& operator>> (CBuffer& buffer, CMyStringW& strData)
{
    int nLen = 0;

    if (buffer.GetReadableSize() >= sizeof(nLen))
    {
        nLen = *(int*)buffer.GetBuffer(0);
    }

    if (buffer.GetReadableSize() >= sizeof(nLen) + (size_t)nLen)
    {
        buffer.Delete(sizeof(nLen));
        strData = CMyStringW((wchar_t*)buffer.Read(nLen), nLen / sizeof(wchar_t));
    }

    return buffer;
}


#ifdef __AFX_H__

template <>
inline CBuffer& operator<< (CBuffer& buffer, const CStringA& strData)
{
    int nLen = strData.GetLength() * sizeof(char);

    buffer.Write(&nLen, sizeof(nLen));
    buffer.Write(strData, nLen);

    return buffer;
}


template <>
inline CBuffer& operator>> (CBuffer& buffer, CStringA& strData)
{
    int nLen = 0;

    if (buffer.GetReadableSize() >= sizeof(nLen))
        buffer.Read(&nLen, sizeof(nLen));

    if (buffer.GetReadableSize() >= (size_t)nLen)
        strData = CMyStringA((char*)buffer.Read(nLen), nLen / sizeof(char));

    return buffer;
}


template <>
inline CBuffer& operator<< (CBuffer& buffer, const CStringW& strData)
{
    int nLen = strData.GetLength() * sizeof(wchar_t);

    buffer.Write(&nLen, sizeof(nLen));
    buffer.Write(strData, nLen);

    return buffer;
}


template <>
inline CBuffer& operator>> (CBuffer& buffer, CStringW& strData)
{
    int nLen = 0;

    if (buffer.GetReadableSize() >= sizeof(nLen))
        buffer.Read(&nLen, sizeof(nLen));

    if (buffer.GetReadableSize() >= (size_t)nLen)
        strData = CMyStringW((wchar_t*)buffer.Read(nLen), nLen / sizeof(wchar_t));

    return buffer;
}

#endif


#ifdef _INC_COMUTIL

//变体就当成字符串解析了
template <>
inline CBuffer& operator<< (CBuffer& buffer, const _variant_t& varData)
{
    if (varData.vt != VT_NULL && varData.vt != VT_EMPTY)
    {
        _bstr_t strData = varData;
        int nLen = strData.length() * sizeof(TCHAR);
        buffer.Write(&nLen, sizeof(nLen));
        buffer.Write((TCHAR*)strData, nLen);
    }
    else
    {
        int nLen = 0;
        buffer.Write(&nLen, sizeof(nLen));
    }

    return buffer;
}

//变体就当成字符串解析了
template <>
inline CBuffer& operator>> (CBuffer& buffer, _variant_t& varData)
{
    int nLen = 0;

    if (buffer.GetReadableSize() >= sizeof(nLen))
    {
        nLen = *(int*)buffer.GetBuffer(0);
    }

    if (buffer.GetReadableSize() >= sizeof(nLen) + (size_t)nLen)
    {
        buffer.Delete(sizeof(nLen));
        varData = CMyString((TCHAR*)buffer.Read(nLen), nLen / sizeof(TCHAR));
    }

    return buffer;
}
#endif