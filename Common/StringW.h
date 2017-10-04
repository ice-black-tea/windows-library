// String.h: interface for the CMyStringW class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_STRINGW_H__02F3F393_12C6_4024_8829_7B6D7A1FB1F4__INCLUDED_)
#define AFX_STRINGW_H__02F3F393_12C6_4024_8829_7B6D7A1FB1F4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <string.h>
#include "List.h"

#if !defined(__AFX_H__)
class CMyStringW;
typedef CMyStringW CStringW;
#endif // !__AFX_H__

#pragma warning(push)
#pragma warning(disable:4244)
#pragma warning(disable:4267)

/************************************************************************/
/*                               字符串                                 */
/************************************************************************/
class CMyStringW
{
public:
    CMyStringW();
    CMyStringW(wchar_t ch, int nRepeat = 1);
    CMyStringW(const wchar_t *pszSrc);
    CMyStringW(const wchar_t *pszSrc, int nLength);
    CMyStringW(const char *pszSrc);
    CMyStringW(const char *pszSrc, int nLength);
    CMyStringW(const CMyStringW& obj);
    virtual ~CMyStringW();

public:
    inline int GetSize() const; //缓冲区大小
    inline int GetLength() const; //字符串长度
    inline bool IsEmpty() const; //是否为空
    inline const wchar_t *GetBuffer() const; //获取字符串
    inline wchar_t *GetBufferSetLength(int nLength = 0); //获取可写缓冲区
    inline void ReleaseBuffer(); //设置长度
    inline CMyStringW& Empty(); //清空字符串

    CMyStringW& Append(wchar_t ch); //字符串拼接
    CMyStringW& Append(const wchar_t *pszSrc); //字符串拼接
    CMyStringW& Append(const char *pszSrc); //字符串拼接
    CMyStringW& Append(const CMyStringW& obj); //字符串拼接
    bool IsEqual(wchar_t ch) const; //字符串是否相同
    bool IsEqual(const wchar_t *pszSrc) const; //字符串是否相同
    bool IsEqual(const CMyStringW& obj) const; //字符串是否相同
    int Compare(wchar_t ch) const; //比较字符串
    int Compare(const wchar_t *pszSrc) const; //比较字符串
    int Compare(const CMyStringW& obj) const; //比较字符串
    bool Contain(wchar_t ch) const; //是否包含子串
    int CompareNoCase(char ch) const; //比较字符串
    int CompareNoCase(const wchar_t *pszSrc) const; //比较字符串
    int CompareNoCase(const CMyStringW& obj) const; //比较字符串
    bool Contain(const wchar_t *pszSub) const; //是否包含子串
    bool Contain(const CMyStringW& obj) const; //是否包含子串
    int Find(wchar_t ch) const; //查找字符
    int Find(const wchar_t *pszSub) const; //查找子串
    int Find(wchar_t ch, int nStart) const; //查找字符
    int Find(const wchar_t *pszSub, int nStart) const; //查找子串
    int ReverseFind(wchar_t ch) const; //反向查找字符

    CMyStringW& Upper(); //转为大写
    CMyStringW& Lower(); //转为小写

    CMyStringW& Remove(int nIndex, int nCount = 0); //移除字符串
    CMyStringW& Insert(int nIndex, const wchar_t *pszSrc); //插入字符串
    CMyStringW& Replace(const wchar_t *pszOld, const wchar_t *pszNew); //替换字符串
    int Split(const wchar_t *pszSplit, CMyList<CMyStringW>& lstStrings); //字符串分割

    CMyStringW& Format(const wchar_t *pszFmt, ...); //格式化字符串

public:
    static int ToInt(const wchar_t *pszSrc); //转为整形
    static int ToInt(const CMyStringW& obj); //转为整形
    static double ToFloat(const wchar_t *pszSrc); //转为浮点型
    static double ToFloat(const CMyStringW& obj); //转为浮点型
    static int ToHex(const CMyStringW& obj); //转为十六进制
    static int ToHex(const wchar_t *pszSrc); //转为十六进制
    static CMyStringW FromMultiByte(const char *pszSrc, int nBytes = -1); //多字节转宽字节

    inline static int GetLength(const wchar_t *pszSrc); //获取字符串长度

public:
    inline operator wchar_t* ();
    inline operator const wchar_t* () const;
    inline wchar_t& operator[](int nIndex);

    inline const CMyStringW& operator= (wchar_t ch);
    inline const CMyStringW& operator= (const wchar_t* psz);
    inline const CMyStringW& operator= (const char* psz);
    inline const CMyStringW& operator= (const CMyStringW& obj);
    inline const CMyStringW& operator+= (wchar_t ch);
    inline const CMyStringW& operator+= (const wchar_t* psz);
    inline const CMyStringW& operator+= (const char* psz);
    inline const CMyStringW& operator+= (const CMyStringW& str);
    inline CMyStringW operator+ (wchar_t ch) const;
    inline CMyStringW operator+ (const wchar_t* psz) const;
    inline CMyStringW operator+ (const char* psz) const;
    inline CMyStringW operator+ (const CMyStringW& str) const;

    inline bool operator== (wchar_t ch) const;
    inline bool operator== (const CMyStringW& str) const;
    inline bool operator== (const wchar_t* psz) const;
    inline bool operator!= (wchar_t ch) const;
    inline bool operator!= (const CMyStringW& str) const;
    inline bool operator!= (const wchar_t* psz) const;
    inline bool operator> (wchar_t ch) const;
    inline bool operator> (const CMyStringW& str) const;
    inline bool operator> (const wchar_t* psz) const;
    inline bool operator>= (wchar_t ch) const;
    inline bool operator>= (const CMyStringW& str) const;
    inline bool operator>= (const wchar_t* psz) const;
    inline bool operator< (wchar_t ch) const;
    inline bool operator< (const CMyStringW& str) const;
    inline bool operator< (const wchar_t* psz) const;
    inline bool operator<= (wchar_t ch) const;
    inline bool operator<= (const CMyStringW& str) const;
    inline bool operator<= (const wchar_t* psz) const;

    friend inline CMyStringW operator+ (wchar_t ch, const CMyStringW& str);
    friend inline CMyStringW operator+ (const wchar_t* psz, const CMyStringW& str);
    friend inline CMyStringW operator+ (const char* psz, const CMyStringW& str);

    friend inline bool operator== (wchar_t ch, const CMyStringW& str);
    friend inline bool operator== (const wchar_t* psz, const CMyStringW& str);
    friend inline bool operator!= (wchar_t ch, const CMyStringW& str);
    friend inline bool operator!= (const wchar_t* psz, const CMyStringW& str);
    friend inline bool operator> (wchar_t ch, const CMyStringW& str);
    friend inline bool operator> (const wchar_t* psz, const CMyStringW& str);
    friend inline bool operator>= (wchar_t ch, const CMyStringW& str);
    friend inline bool operator>= (const wchar_t* psz, const CMyStringW& str);
    friend inline bool operator< (wchar_t ch, const CMyStringW& str);
    friend inline bool operator< (const wchar_t* psz, const CMyStringW& str);
    friend inline bool operator<= (wchar_t ch, const CMyStringW& str);
    friend inline bool operator<= (const wchar_t* psz, const CMyStringW& str);

private:
    wchar_t *m_pBuff;  //缓冲区
    int m_nSize; //空间
    int m_nLength; //长度
    int *m_pnRefCnt; // 引用计数

    static CMyStringW m_strEmpty;

    void Alloc(int nSize);
    void AllocCopy(const wchar_t *pszBuff, int nSize = 0);
    void CopyBeforeWrite(int nSize = 0);

    inline int  GetRefCnt() const; //获取引用计数
    inline void AddRefCnt(); //增加引用计数
    inline void Release(); //释放空间
};

//缓冲区大小
inline int CMyStringW::GetSize() const
{
    return m_nSize;
}

//字符串长度
inline int CMyStringW::GetLength() const
{
    return m_nLength;
}

//字符串长度
inline int CMyStringW::GetLength(const wchar_t *pszSrc)
{
    return pszSrc != NULL ? wcslen(pszSrc) : 0;
}

//是否为空
inline bool CMyStringW::IsEmpty() const
{
    return m_nLength == 0;
}

//获取字符串
inline const wchar_t *CMyStringW::GetBuffer() const
{
    return m_pBuff;
}

//获取字符串
inline wchar_t *CMyStringW::GetBufferSetLength(int nSize)
{
    CopyBeforeWrite(nSize);
    return m_pBuff;
}

//设置长度
inline void CMyStringW::ReleaseBuffer()
{
    m_nLength = GetLength(m_pBuff);
}

//清空字符串
inline CMyStringW& CMyStringW::Empty()
{
    *this = m_strEmpty;

    return *this;
}

//获取引用计数
inline int CMyStringW::GetRefCnt() const
{
    return m_pnRefCnt != NULL ? *m_pnRefCnt : 0;
}

//增加引用计数
inline void CMyStringW::AddRefCnt()
{
    if (m_pnRefCnt == NULL)
    {
        m_pnRefCnt = new int(0);
    }

    ++(*m_pnRefCnt);
}

//释放空间
inline void CMyStringW::Release()
{
    if (m_pnRefCnt != NULL && --(*m_pnRefCnt) == 0)
    {
        delete[] m_pBuff;
        delete m_pnRefCnt;
    }

    m_pBuff = NULL;
    m_pnRefCnt = NULL;
    m_nLength = 0;
    m_nSize = 0;
}

inline CMyStringW::operator wchar_t* ()
{
    return m_pBuff;
}

inline CMyStringW::operator const wchar_t* () const
{
    return m_pBuff;
}

inline wchar_t& CMyStringW::operator[](int nIndex)
{
    return m_pBuff[nIndex];
}

inline const CMyStringW& CMyStringW::operator= (wchar_t ch)
{
    Release();
    AllocCopy(&ch, 2);

    return *this;
}

inline const CMyStringW& CMyStringW::operator= (const wchar_t* psz)
{
    Release();
    AllocCopy(psz);

    return *this;
}

inline const CMyStringW& CMyStringW::operator= (const char* psz)
{
    Release();
    CMyStringW obj = FromMultiByte(psz);
    m_nSize = obj.m_nSize;
    m_nLength = obj.m_nLength;
    m_pBuff = obj.m_pBuff;
    m_pnRefCnt = obj.m_pnRefCnt;
    AddRefCnt();

    return *this;
}

inline const CMyStringW& CMyStringW::operator= (const CMyStringW& obj)
{
    if (this != &obj)
    {
        Release();
        m_nSize = obj.m_nSize;
        m_nLength = obj.m_nLength;
        m_pBuff = obj.m_pBuff;
        m_pnRefCnt = obj.m_pnRefCnt;
        AddRefCnt();
    }

    return *this;
}

inline const CMyStringW& CMyStringW::operator+= (wchar_t ch)
{
    return Append(ch);
}

inline const CMyStringW& CMyStringW::operator+= (const wchar_t* psz)
{
    return Append(psz);
}

inline const CMyStringW& CMyStringW::operator+= (const char* psz)
{
    return Append(psz);
}

inline const CMyStringW& CMyStringW::operator+= (const CMyStringW& str)
{
    return Append(str);
}

inline CMyStringW CMyStringW::operator+ (wchar_t ch) const
{
    return CMyStringW(*this).Append(ch);
}

inline CMyStringW CMyStringW::operator+ (const wchar_t* psz) const
{
    return CMyStringW(*this).Append(psz);
}

inline CMyStringW CMyStringW::operator+ (const char* psz) const
{
    return CMyStringW(*this).Append(psz);
}

inline CMyStringW CMyStringW::operator+ (const CMyStringW& str) const
{
    return CMyStringW(*this).Append(str);
}

inline bool CMyStringW::operator== (wchar_t ch) const
{
    return Compare(ch) == 0;
}

inline bool CMyStringW::operator== (const wchar_t* psz) const
{
    return Compare(psz) == 0;
}

inline bool CMyStringW::operator== (const CMyStringW& str) const
{
    return Compare(str) == 0;
}

inline bool CMyStringW::operator!= (wchar_t ch) const
{
    return Compare(ch) != 0;
}

inline bool CMyStringW::operator!= (const wchar_t* psz) const
{
    return Compare(psz) != 0;
}

inline bool CMyStringW::operator!= (const CMyStringW& str) const
{
    return Compare(str) != 0;
}

inline bool CMyStringW::operator> (wchar_t ch) const
{
    return Compare(ch) > 0;
}

inline bool CMyStringW::operator> (const wchar_t* psz) const
{
    return Compare(psz) > 0;
}

inline bool CMyStringW::operator> (const CMyStringW& str) const
{
    return Compare(str) > 0;
}

inline bool CMyStringW::operator>= (wchar_t ch) const
{
    return Compare(ch) >= 0;
}

inline bool CMyStringW::operator>= (const CMyStringW& str) const
{
    return Compare(str) >= 0;
}

inline bool CMyStringW::operator>= (const wchar_t* psz) const
{
    return Compare(psz) >= 0;
}

inline bool CMyStringW::operator< (wchar_t ch) const
{
    return Compare(ch) < 0;
}

inline bool CMyStringW::operator< (const CMyStringW& str) const
{
    return Compare(str) < 0;
}

inline bool CMyStringW::operator< (const wchar_t* psz) const
{
    return Compare(psz) < 0;
}

inline bool CMyStringW::operator<= (wchar_t ch) const
{
    return Compare(ch) <= 0;
}

inline bool CMyStringW::operator<= (const CMyStringW& str) const
{
    return Compare(str) <= 0;
}

inline bool CMyStringW::operator<= (const wchar_t* psz) const
{
    return Compare(psz) <= 0;
}

inline CMyStringW operator+ (wchar_t ch, const CMyStringW& str)
{
    return CMyStringW(ch) + str;
}

inline CMyStringW operator+ (const wchar_t* psz, const CMyStringW& str)
{
    return CMyStringW(psz) + str;
}

inline CMyStringW operator+ (const char* psz, const CMyStringW& str)
{
    return CMyStringW(psz) + str;
}

inline bool operator== (wchar_t ch, const CMyStringW& str)
{
    return str == ch;
}

inline bool operator== (const wchar_t* psz, const CMyStringW& str)
{
    return str == psz;
}

inline bool operator!= (wchar_t ch, const CMyStringW& str)
{
    return str != ch;
}

inline bool operator!= (const wchar_t* psz, const CMyStringW& str)
{
    return str != psz;
}

inline bool operator> (wchar_t ch, const CMyStringW& str)
{
    return str < ch;
}

inline bool operator>(const wchar_t* psz, const CMyStringW& str)
{
    return str < psz;
}

inline bool operator>= (wchar_t ch, const CMyStringW& str)
{
    return str <= ch;
}

inline bool operator>= (const wchar_t* psz, const CMyStringW& str)
{
    return str <= psz;
}

inline bool operator< (wchar_t ch, const CMyStringW& str)
{
    return str > ch;
}

inline bool operator< (const wchar_t* psz, const CMyStringW& str)
{
    return str > psz;
}

inline bool operator<= (wchar_t ch, const CMyStringW& str)
{
    return str >= ch;
}

inline bool operator<= (const wchar_t* psz, const CMyStringW& str)
{
    return str >= psz;
}

#pragma warning(pop)

#endif // !defined(AFX_STRINGW_H__02F3F393_12C6_4024_8829_7B6D7A1FB1F4__INCLUDED_)
