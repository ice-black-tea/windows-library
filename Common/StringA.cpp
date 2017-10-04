// String.cpp: implementation of the CMyStringA class.
//
//////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include "StringA.h"

#pragma warning(disable:4244)
#pragma warning(disable:4267)
#pragma warning(disable:4996)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMyStringA::CMyStringA()
    : m_nSize(0)
    , m_nLength(0)
    , m_pBuff(NULL)
    , m_pnRefCnt(NULL)
{

}

CMyStringA::CMyStringA(char ch, int nRepeat)
    : m_nSize(0)
    , m_nLength(0)
    , m_pBuff(NULL)
    , m_pnRefCnt(NULL)
{
    Alloc(nRepeat + 1);

    m_nLength = nRepeat;

    for (int i = 0; i < nRepeat; i++)
    {
        m_pBuff[i] = ch;
    }
}

CMyStringA::CMyStringA(const char *pszSrc)
    : m_nSize(0)
    , m_nLength(0)
    , m_pBuff(NULL)
    , m_pnRefCnt(NULL)
{
    AllocCopy(pszSrc);
}

CMyStringA::CMyStringA(const char *pszSrc, int nLength)
    : m_nSize(0)
    , m_nLength(0)
    , m_pBuff(NULL)
    , m_pnRefCnt(NULL)
{
    AllocCopy(pszSrc, nLength + 1);
}

CMyStringA::CMyStringA(const wchar_t *pszSrc)
{
    CMyStringA strSrc = FromWideChar(pszSrc);
    m_nSize = strSrc.m_nSize;
    m_nLength = strSrc.m_nLength;
    m_pBuff = strSrc.m_pBuff;
    m_pnRefCnt = strSrc.m_pnRefCnt;
    AddRefCnt();
}

CMyStringA::CMyStringA(const wchar_t *pszSrc, int nLength)
{
    CMyStringA strSrc = FromWideChar(pszSrc, nLength);
    m_nSize = strSrc.m_nSize;
    m_nLength = strSrc.m_nLength;
    m_pBuff = strSrc.m_pBuff;
    m_pnRefCnt = strSrc.m_pnRefCnt;
    AddRefCnt();
}

CMyStringA::CMyStringA(const CMyStringA& obj)
    : m_nSize(obj.m_nSize)
    , m_nLength(obj.m_nLength)
    , m_pBuff(obj.m_pBuff)
    , m_pnRefCnt(obj.m_pnRefCnt)
{
    AddRefCnt();
}

CMyStringA::~CMyStringA()
{
    Release();
}

//字符串拼接
CMyStringA& CMyStringA::Append(char ch)
{
    char sz[2] = { ch };

    return Append(sz);
}

//字符串拼接
CMyStringA& CMyStringA::Append(const char *pszSrc)
{
    if (pszSrc == NULL)
    {
        return *this;
    }

    int nLength = GetLength(pszSrc) + m_nLength;
    int nSize = nLength < m_nSize ? m_nSize : nLength + 1;

    CopyBeforeWrite(nSize);
    strcat(m_pBuff, pszSrc);
    m_nLength = nLength;

    return *this;
}

//字符串拼接
CMyStringA& CMyStringA::Append(const wchar_t *pszSrc)
{
    CMyStringA strSrc = FromWideChar(pszSrc);

    return Append(strSrc.m_pBuff);
}

//字符串拼接
CMyStringA& CMyStringA::Append(const CMyStringA& obj)
{
    if (this != &obj)
    {
        Append(obj.m_pBuff);
    }
    else
    {
        Append(CMyStringA(obj.m_pBuff).m_pBuff);
    }

    return *this;
}

//字符串是否相同
bool CMyStringA::IsEqual(char ch) const
{
    char sz[2] = { ch };

    return IsEqual(sz);
}

//字符串是否相同
bool CMyStringA::IsEqual(const char *pszSrc) const
{
    if (pszSrc == NULL)
    {
        return false;
    }

    return strcmp(m_pBuff, pszSrc) == 0;
}

//字符串是否相同
bool CMyStringA::IsEqual(const CMyStringA& obj) const
{
    return IsEqual(obj.m_pBuff);
}

//字符串比较
int CMyStringA::Compare(char ch) const
{
    char sz[2] = { ch };

    return Compare(sz);
}

//字符串比较
int CMyStringA::Compare(const char *pszSrc) const
{
    if (pszSrc == NULL)
    {
        return -1;
    }

    return strcmp(m_pBuff, pszSrc);
}

//字符串比较
int CMyStringA::Compare(const CMyStringA& obj) const
{
    return Compare(obj.m_pBuff);
}

//比较字符串
int CMyStringA::CompareNoCase(char ch) const
{
    char sz[2] = { ch };

    return CompareNoCase(sz);
}

//比较字符串
int CMyStringA::CompareNoCase(const char *pszSrc) const
{
    if (pszSrc == NULL)
    {
        return -1;
    }

    return stricmp(m_pBuff, pszSrc);
}

//比较字符串
int CMyStringA::CompareNoCase(const CMyStringA& obj) const
{
    return CompareNoCase(obj.m_pBuff);
}

//是否包含子串
bool CMyStringA::Contain(char ch) const
{
    char sz[2] = { ch };

    return Contain(sz);
}

//是否包含子串
bool CMyStringA::Contain(const char *pszSub) const
{
    if (pszSub == NULL)
    {
        return true;
    }

    return strstr(m_pBuff, pszSub) != NULL;
}

//是否包含子串
bool CMyStringA::Contain(const CMyStringA& obj) const
{
    return Contain(obj.m_pBuff);
}

//查找字符
int CMyStringA::Find(char ch) const
{
    char sz[2] = { ch };

    return Find(sz);
}

//查找子串
int CMyStringA::Find(const char *pszSub) const
{
    char* psz = strstr(m_pBuff, pszSub);

    return psz != NULL ? psz - m_pBuff : -1;
}

//查找字符
int CMyStringA::Find(char ch, int nStart) const
{
    char sz[2] = { ch };

    return Find(sz, nStart);
}

//查找子串
int CMyStringA::Find(const char *pszSub, int nStart) const
{
    char* psz = strstr(m_pBuff + nStart, pszSub);

    return psz != NULL ? psz - m_pBuff : -1;
}

//反向查找字符
int CMyStringA::ReverseFind(char ch) const
{
    for (int i = m_nLength - 1; i >= 0; i--)
    {
        if (m_pBuff[i] == ch)
        {
            return i;
        }
    }

    return -1;
}

//转为大写
CMyStringA& CMyStringA::Upper()
{
    if (m_pBuff == NULL)
    {
        return *this;
    }

    CopyBeforeWrite();

    strupr(m_pBuff);

    return *this;
}

//转为小写
CMyStringA& CMyStringA::Lower()
{
    if (m_pBuff == NULL)
    {
        return *this;
    }

    CopyBeforeWrite();

    strlwr(m_pBuff);

    return *this;
}

//移除字符串
CMyStringA& CMyStringA::Remove(int nIndex, int nCount)
{
    if (nIndex < 0 || nIndex >= m_nLength || nCount < 0)
    {
        return *this;
    }

    CopyBeforeWrite();

    if (nCount == 0 || nIndex + nCount >= m_nLength)
    {
        m_nLength = nIndex;
        m_pBuff[nIndex] = L'\0';
    }
    else
    {
        m_nLength -= nCount;
        for (int i = nIndex; i <= m_nLength; i++)
        {
            m_pBuff[i] = m_pBuff[i + nCount];
        }
    }

    return *this;
}

//插入字符串
CMyStringA& CMyStringA::Insert(int nIndex, const char *pszSrc)
{
    if (pszSrc == NULL || *pszSrc == '\0' || nIndex < 0)
    {
        return *this;
    }

    int nLength = GetLength(pszSrc), i = 0;

    CopyBeforeWrite(m_nLength + nLength + 1);

    nIndex = nIndex < m_nLength ? nIndex : m_nLength;

    for (i = m_nLength; i >= nIndex; i--)
    {
        m_pBuff[i + nLength] = m_pBuff[i];
    }

    for (i = 0; i < nLength; i++)
    {
        m_pBuff[i + nIndex] = pszSrc[i];
    }

    m_nLength += nLength;

    return *this;
}

//替换字符串
CMyStringA& CMyStringA::Replace(const char *pszOld, const char *pszNew)
{
    int nOldLength = GetLength(pszOld);

    if (m_nLength == 0 || nOldLength == 0)
    {
        return *this;
    }

    int nNewLength = GetLength(pszNew);

    int nCount = 0;
    char *pszStart = m_pBuff;
    char *pszTarget = NULL;

    while ((pszTarget = strstr(pszStart, pszOld)) != NULL)
    {
        nCount++;
        pszStart = pszTarget + nOldLength;
    }

    if (nCount > 0)
    {
        CopyBeforeWrite(m_nLength + (nNewLength - nOldLength) * nCount + 1);

        pszStart = m_pBuff;

        while ((pszTarget = strstr(pszStart, pszOld)) != NULL)
        {
            int nLength = m_nLength - (pszTarget - m_pBuff) - nOldLength + 1;

            memmove(pszTarget + nNewLength,
                pszTarget + nOldLength,
                nLength * sizeof(char));
            memmove(pszTarget, pszNew, nNewLength * sizeof(char));

            m_nLength += nNewLength - nOldLength;
            pszStart = pszTarget + nNewLength;
        }
    }

    return *this;
}

//字符串分割
int CMyStringA::Split(const char *pszSplit, CMyList<CMyStringA>& lstStrings)
{
    lstStrings.RemoveAll();

    int nSplitLength = GetLength(pszSplit);

    if (m_nLength == 0 || nSplitLength == 0)
    {
        return lstStrings.GetCount();
    }

    char *pszStart = m_pBuff;
    char *pszTarget = NULL;

    while ((pszTarget = strstr(pszStart, pszSplit)) != NULL)
    {
        int nLength = pszTarget - pszStart;

        CMyStringA strItem;
        strItem.AllocCopy(pszStart, nLength + 1);
        lstStrings.AddTail(strItem);

        pszStart = pszTarget + nSplitLength;
    }

    return lstStrings.GetCount();
}

//格式化字符串
CMyStringA& CMyStringA::Format(const char *pszFmt, ...)
{
    va_list argList = NULL;
    va_start(argList, pszFmt);

    const int MAXSIZE = 256;
    char szBuff[MAXSIZE] = { 0 };
    _vsnprintf(szBuff, MAXSIZE - 1, pszFmt, argList);

    va_end(argList);

    AllocCopy(szBuff);

    return *this;
}

//分配空间
void CMyStringA::Alloc(int nSize)
{
    Release();

    m_nSize = nSize;
    m_nLength = 0;
    m_pBuff = new char[m_nSize];
    memset(m_pBuff, 0, m_nSize * sizeof(char));

    AddRefCnt();
}

//分配拷贝
void CMyStringA::AllocCopy(const char *pszBuff, int nSize)
{
    int nLength = GetLength(pszBuff);

    if (nSize <= 0)
    {
        nSize = nLength < nSize ? nSize : nLength + 1;
    }

    if (GetRefCnt() > 1 || nSize > m_nSize)
    {
        Release();

        m_nSize = nSize;
        m_pBuff = new char[m_nSize];

        AddRefCnt();
    }

    m_nLength = nLength < m_nSize ? nLength : m_nSize - 1;

    strncpy(m_pBuff, pszBuff, m_nLength);
    m_pBuff[m_nLength] = '\0';
}

//写时拷贝
void CMyStringA::CopyBeforeWrite(int nSize)
{
    nSize = nSize > 0 ? nSize : m_nSize;

    if (GetRefCnt() > 1 || nSize > m_nSize)
    {
        int nLength = m_nLength < nSize ? m_nLength : nSize - 1;
        char *pszBuff = new char[nSize];
        strncpy(pszBuff, m_pBuff, nLength);
        pszBuff[nLength] = '\0';
        Release();

        m_nSize = nSize;
        m_nLength = nLength;
        m_pBuff = pszBuff;
        AddRefCnt();
    }
}

//转为整形
int CMyStringA::ToInt(const char *pszSrc)
{
    int nValue = 0;

    sscanf(pszSrc, "%d", &nValue);

    return nValue;
}

//转为整形
int CMyStringA::ToInt(const CMyStringA& obj)
{
    return ToInt(obj.m_pBuff);
}

//转为浮点型
double CMyStringA::ToFloat(const char *pszSrc)
{
    double dblValue = 0;

    sscanf(pszSrc, "%f", &dblValue);

    return dblValue;
}

//转为浮点型
double CMyStringA::ToFloat(const CMyStringA& obj)
{
    return ToFloat(obj.m_pBuff);
}

//转为十六进制
int CMyStringA::ToHex(const CMyStringA& obj)
{
    return ToHex(obj.m_pBuff);
}

//转为十六进制
int CMyStringA::ToHex(const char *pszSrc)
{
    return strtoul(pszSrc, NULL, 16);
}

#ifdef WIN32

#include <Windows.h>

CMyStringA CMyStringA::FromWideChar(const wchar_t *pszSrc, int nBytes)
{
    CMyStringA strDest;

    if (pszSrc == NULL)
    {
        return strDest;
    }

    int nSize = WideCharToMultiByte(
        CP_ACP,
        0,
        pszSrc,
        nBytes,
        NULL,
        0,
        NULL,
        NULL);

    if (nSize <= 0)
    {
        return strDest;
    }

    strDest.Alloc(nSize);
    strDest.m_nLength = WideCharToMultiByte(
        CP_ACP,
        0,
        pszSrc,
        nBytes,
        strDest.m_pBuff,
        nSize,
        NULL,
        NULL);
    strDest.m_nLength--;

    return strDest;
}
#else // Linux Version

#endif // WIN32


char* stristr(const char * str1, const char * str2)
{
    char *cp = (char *)str1;
    char *s1, *s2;

    if (!*str2)
        return((char *)str1);

    while (*cp)
    {
        s1 = cp;
        s2 = (char *)str2;

        while (*s1 && *s2)
        {
            char ch1 = *s1, ch2 = *s2;
            if (isascii(*s1) && iswupper(*s1)) ch1 = towlower(*s1);
            if (isascii(*s2) && iswupper(*s2)) ch2 = towlower(*s2);

            if (ch1 - ch2 == 0) s1++, s2++;
            else break;
        }

        if (!*s2)
            return(cp);

        cp++;
    }

    return(NULL);
}