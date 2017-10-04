// String.h: interface for the CMyStringW class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_STRINGA_H__02F3F393_12C6_4024_8829_7B6D7A1FB1F4__INCLUDED_)
#define AFX_STRINGA_H__02F3F393_12C6_4024_8829_7B6D7A1FB1F4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <string.h>
#include "List.h"

#if !defined(__AFX_H__)
class CMyStringA;
typedef CMyStringA CStringA;
#endif // !__AFX_H__

#pragma warning(push)
#pragma warning(disable:4244)
#pragma warning(disable:4267)

/************************************************************************/
/*                               字符串                                 */
/************************************************************************/
class CMyStringA
{
public:
    CMyStringA();
    CMyStringA(char ch, int nRepeat = 1);
    CMyStringA(const char *pszSrc);
    CMyStringA(const char *pszSrc, int nLength);
    CMyStringA(const wchar_t *pszSrc);
    CMyStringA(const wchar_t *pszSrc, int nLength);
    CMyStringA(const CMyStringA& obj);
    virtual ~CMyStringA();

public:
    inline int GetSize() const; //缓冲区大小
    inline int GetLength() const; //字符串长度
    inline bool IsEmpty() const; //是否为空
    inline const char *GetBuffer() const; //获取字符串
    inline char *GetBufferSetLength(int nIndex = 0); //获取可写缓冲区
    inline void ReleaseBuffer(); //设置长度
    inline CMyStringA& Empty(); //清空字符串

    CMyStringA& Append(char ch); //字符串拼接
    CMyStringA& Append(const char *pszSrc); //字符串拼接
    CMyStringA& Append(const wchar_t *pszSrc); //字符串拼接
    CMyStringA& Append(const CMyStringA& obj); //字符串拼接
    bool IsEqual(char ch) const; //字符串是否相同
    bool IsEqual(const char *pszSrc) const; //字符串是否相同
    bool IsEqual(const CMyStringA& obj) const; //字符串是否相同
    int Compare(char ch) const; //比较字符串
    int Compare(const char *pszSrc) const; //比较字符串
    int Compare(const CMyStringA& obj) const; //比较字符串
    int CompareNoCase(char ch) const; //比较字符串
    int CompareNoCase(const char *pszSrc) const; //比较字符串
    int CompareNoCase(const CMyStringA& obj) const; //比较字符串
    bool Contain(char ch) const; //是否包含子串
    bool Contain(const char *pszSub) const; //是否包含子串
    bool Contain(const CMyStringA& obj) const; //是否包含子串
    int Find(char ch) const; //查找字符
    int Find(const char *pszSub) const; //查找子串
    int Find(char ch, int nStart) const; //查找字符
    int Find(const char *pszSub, int nStart) const; //查找子串
    int ReverseFind(char ch) const; //反向查找字符

    CMyStringA& Upper(); //转为大写
    CMyStringA& Lower(); //转为小写
    CMyStringA& Remove(int nIndex, int nCount = 0); //移除字符串
    CMyStringA& Insert(int nIndex, const char *pszSrc); //插入字符串
    CMyStringA& Replace(const char *pszOld, const char *pszNew); //替换字符串
    int Split(const char *pszSplit, CMyList<CMyStringA>& lstStrings); //字符串分割

    CMyStringA& Format(const char *pszFmt, ...); //格式化字符串

public:
    static int ToInt(const char *pszSrc); //转为整形
    static int ToInt(const CMyStringA& obj); //转为整形
    static double ToFloat(const char *pszSrc); //转为浮点型
    static double ToFloat(const CMyStringA& obj); //转为浮点型
    static int ToHex(const CMyStringA& obj); //转为十六进制
    static int ToHex(const char *pszSrc); //转为十六进制
    static CMyStringA FromWideChar(const wchar_t *pszSrc, int nBytes = -1); //宽字节转多字节

    inline static int GetLength(const char *pszSrc); //获取字符串长度

public:
    inline operator char* ();
    inline operator const char* () const;
    inline char& operator[](int nIndex);

    inline const CMyStringA& operator= (char ch);
    inline const CMyStringA& operator= (const char* psz);
    inline const CMyStringA& operator= (const wchar_t* psz);
    inline const CMyStringA& operator= (const CMyStringA& obj);
    inline const CMyStringA& operator+= (char ch);
    inline const CMyStringA& operator+= (const char* psz);
    inline const CMyStringA& operator+= (const wchar_t* psz);
    inline const CMyStringA& operator+= (const CMyStringA& str);
    inline CMyStringA operator+ (char ch) const;
    inline CMyStringA operator+ (const char* psz) const;
    inline CMyStringA operator+ (const wchar_t* psz) const;
    inline CMyStringA operator+ (const CMyStringA& str) const;

    inline bool operator== (char ch) const;
    inline bool operator== (const CMyStringA& str) const;
    inline bool operator== (const char* psz) const;
    inline bool operator!= (char ch) const;
    inline bool operator!= (const CMyStringA& str) const;
    inline bool operator!= (const char* psz) const;
    inline bool operator> (char ch) const;
    inline bool operator> (const CMyStringA& str) const;
    inline bool operator> (const char* psz) const;
    inline bool operator>= (char ch) const;
    inline bool operator>= (const CMyStringA& str) const;
    inline bool operator>= (const char* psz) const;
    inline bool operator< (char ch) const;
    inline bool operator< (const CMyStringA& str) const;
    inline bool operator< (const char* psz) const;
    inline bool operator<= (char ch) const;
    inline bool operator<= (const CMyStringA& str) const;
    inline bool operator<= (const char* psz) const;

    friend inline CMyStringA operator+ (char ch, const CMyStringA& str);
    friend inline CMyStringA operator+ (const char* psz, const CMyStringA& str);
    friend inline CMyStringA operator+ (const wchar_t* psz, const CMyStringA& str);

    friend inline bool operator== (char ch, const CMyStringA& str);
    friend inline bool operator== (const char* psz, const CMyStringA& str);
    friend inline bool operator!= (char ch, const CMyStringA& str);
    friend inline bool operator!= (const char* psz, const CMyStringA& str);
    friend inline bool operator> (char ch, const CMyStringA& str);
    friend inline bool operator> (const char* psz, const CMyStringA& str);
    friend inline bool operator>= (char ch, const CMyStringA& str);
    friend inline bool operator>= (const char* psz, const CMyStringA& str);
    friend inline bool operator< (char ch, const CMyStringA& str);
    friend inline bool operator< (const char* psz, const CMyStringA& str);
    friend inline bool operator<= (char ch, const CMyStringA& str);
    friend inline bool operator<= (const char* psz, const CMyStringA& str);

private:
    char *m_pBuff;  //缓冲区
    int m_nSize; //空间
    int m_nLength; //长度
    int *m_pnRefCnt; // 引用计数

    static CMyStringA m_strEmpty;

    void Alloc(int nSize);
    void AllocCopy(const char *pszBuff, int nSize = 0);
    void CopyBeforeWrite(int nSize = 0);

    inline int  GetRefCnt() const; //获取引用计数
    inline void AddRefCnt(); //增加引用计数
    inline void Release(); //释放空间
};

//缓冲区大小
inline int CMyStringA::GetSize() const
{
    return m_nSize;
}

//字符串长度
inline int CMyStringA::GetLength() const
{
    return m_nLength;
}

//字符串长度
inline int CMyStringA::GetLength(const char *pszSrc)
{
    return pszSrc != NULL ? strlen(pszSrc) : 0;
}

//是否为空
inline bool CMyStringA::IsEmpty() const
{
    return m_nLength == 0;
}

//获取字符串
inline const char *CMyStringA::GetBuffer() const
{
    return m_pBuff;
}

//获取字符串
inline char *CMyStringA::GetBufferSetLength(int nSize)
{
    CopyBeforeWrite(nSize);
    return m_pBuff;
}

//设置长度
inline void CMyStringA::ReleaseBuffer()
{
    m_nLength = GetLength(m_pBuff);
}

//清空字符串
inline CMyStringA& CMyStringA::Empty()
{
    *this = CMyStringA();

    return *this;
}

//获取引用计数
inline int CMyStringA::GetRefCnt() const
{
    return m_pnRefCnt != NULL ? *m_pnRefCnt : 0;
}

//增加引用计数
inline void CMyStringA::AddRefCnt()
{
    if (m_pnRefCnt == NULL)
    {
        m_pnRefCnt = new int(0);
    }

    ++(*m_pnRefCnt);
}

//释放空间
inline void CMyStringA::Release()
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

inline CMyStringA::operator char* ()
{
    return m_pBuff;
}

inline CMyStringA::operator const char* () const
{
    return m_pBuff;
}

inline char& CMyStringA::operator[](int nIndex)
{
    return m_pBuff[nIndex];
}

inline const CMyStringA& CMyStringA::operator= (char ch)
{
    Release();
    AllocCopy(&ch, 2);

    return *this;
}

inline const CMyStringA& CMyStringA::operator= (const char* psz)
{
    Release();
    AllocCopy(psz);

    return *this;
}

inline const CMyStringA& CMyStringA::operator= (const wchar_t* psz)
{
    Release();
    CMyStringA obj = FromWideChar(psz);
    m_nSize = obj.m_nSize;
    m_nLength = obj.m_nLength;
    m_pBuff = obj.m_pBuff;
    m_pnRefCnt = obj.m_pnRefCnt;
    AddRefCnt();

    return *this;
}

inline const CMyStringA& CMyStringA::operator= (const CMyStringA& obj)
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

inline const CMyStringA& CMyStringA::operator+= (char ch)
{
    return Append(ch);
}

inline const CMyStringA& CMyStringA::operator+= (const char* psz)
{
    return Append(psz);
}

inline const CMyStringA& CMyStringA::operator+= (const wchar_t* psz)
{
    return Append(psz);
}

inline const CMyStringA& CMyStringA::operator+= (const CMyStringA& str)
{
    return Append(str);
}

inline CMyStringA CMyStringA::operator+ (char ch) const
{
    return CMyStringA(*this).Append(ch);
}

inline CMyStringA CMyStringA::operator+ (const char* psz) const
{
    return CMyStringA(*this).Append(psz);
}

inline CMyStringA CMyStringA::operator+ (const wchar_t* psz) const
{
    return CMyStringA(*this).Append(psz);
}

inline CMyStringA CMyStringA::operator+ (const CMyStringA& str) const
{
    return CMyStringA(*this).Append(str);
}

inline bool CMyStringA::operator== (char ch) const
{
    return Compare(ch) == 0;
}

inline bool CMyStringA::operator== (const char* psz) const
{
    return Compare(psz) == 0;
}

inline bool CMyStringA::operator== (const CMyStringA& str) const
{
    return Compare(str) == 0;
}

inline bool CMyStringA::operator!= (char ch) const
{
    return Compare(ch) != 0;
}

inline bool CMyStringA::operator!= (const char* psz) const
{
    return Compare(psz) != 0;
}

inline bool CMyStringA::operator!= (const CMyStringA& str) const
{
    return Compare(str) != 0;
}

inline bool CMyStringA::operator> (char ch) const
{
    return Compare(ch) > 0;
}

inline bool CMyStringA::operator> (const char* psz) const
{
    return Compare(psz) > 0;
}

inline bool CMyStringA::operator> (const CMyStringA& str) const
{
    return Compare(str) > 0;
}

inline bool CMyStringA::operator>= (char ch) const
{
    return Compare(ch) >= 0;
}

inline bool CMyStringA::operator>= (const CMyStringA& str) const
{
    return Compare(str) >= 0;
}

inline bool CMyStringA::operator>= (const char* psz) const
{
    return Compare(psz) >= 0;
}

inline bool CMyStringA::operator< (char ch) const
{
    return Compare(ch) < 0;
}

inline bool CMyStringA::operator< (const CMyStringA& str) const
{
    return Compare(str) < 0;
}

inline bool CMyStringA::operator< (const char* psz) const
{
    return Compare(psz) < 0;
}

inline bool CMyStringA::operator<= (char ch) const
{
    return Compare(ch) <= 0;
}

inline bool CMyStringA::operator<= (const CMyStringA& str) const
{
    return Compare(str) <= 0;
}

inline bool CMyStringA::operator<= (const char* psz) const
{
    return Compare(psz) <= 0;
}

inline CMyStringA operator+ (char ch, const CMyStringA& str)
{
    return CMyStringA(ch) + str;
}

inline CMyStringA operator+ (const char* psz, const CMyStringA& str)
{
    return CMyStringA(psz) + str;
}

inline CMyStringA operator+ (const wchar_t* psz, const CMyStringA& str)
{
    return CMyStringA(psz) + str;
}

inline bool operator== (char ch, const CMyStringA& str)
{
    return str == ch;
}

inline bool operator== (const char* psz, const CMyStringA& str)
{
    return str == psz;
}

inline bool operator!= (char ch, const CMyStringA& str)
{
    return str != ch;
}

inline bool operator!= (const char* psz, const CMyStringA& str)
{
    return str != psz;
}

inline bool operator> (char ch, const CMyStringA& str)
{
    return str < ch;
}

inline bool operator>(const char* psz, const CMyStringA& str)
{
    return str < psz;
}

inline bool operator>= (char ch, const CMyStringA& str)
{
    return str <= ch;
}

inline bool operator>= (const char* psz, const CMyStringA& str)
{
    return str <= psz;
}

inline bool operator< (char ch, const CMyStringA& str)
{
    return str > ch;
}

inline bool operator< (const char* psz, const CMyStringA& str)
{
    return str > psz;
}

inline bool operator<= (char ch, const CMyStringA& str)
{
    return str >= ch;
}

inline bool operator<= (const char* psz, const CMyStringA& str)
{
    return str >= psz;
}

#pragma warning(pop)

#endif // !defined(AFX_STRINGA_H__02F3F393_12C6_4024_8829_7B6D7A1FB1F4__INCLUDED_)