#pragma once

#include <io.h>
#include <stdio.h>
#include <assert.h>
#include "../StringT.h"

#if defined(WIN32)
#define access _access
#endif

/************************************************************************/
/*                             文件工具类                               */
/************************************************************************/
class CFileUtil
{
public:
    enum
    {
        FILE_READ = 1, //读（默认）
        FILE_WRITE = 2, //写
        FILE_EXIST = 4, //文件必须存在
        FILE_CREATE = 8, //文件不存在则创建
        FILE_CREATE_ALWAYS = 16, //总是创建文件
    };

public:
    CFileUtil(); //构造函数
    CFileUtil(const char *pszFileName); //构造函数
    CFileUtil(const wchar_t *pszFileName); //构造函数
    virtual ~CFileUtil(); //析构函数

public:
    void SetFileName(CMyStringA strFileName); //设置文件名
    CMyStringA GetFileName() const; //获取文件名

    bool IsExist() const;
    bool IsOpen() const; //是否打开

    bool Create(); //创建文件
    bool Clear(); //清空文件

    bool Open(int nMode = FILE_READ | FILE_EXIST); //打开文件
    void Close(); //关闭文件

    size_t Write(const void *pBuff, size_t nSize); //写文件
    size_t Write(int nOffset, const void *pBuff, size_t nSize); //写文件
    size_t Read(void *pBuff, size_t nSize); //读文件
    size_t Read(int nOffset, void *pBuff, size_t nSize); //读文件

    bool Seek(int nOffset); //移动文件指针到指定处
    void SeekToBegin(); //移动文件指针至文件开头
    size_t SeekToEnd(); //移动文件指针至文件末，返回文件长度

    void Flush(); //刷新文件缓存区

private:
    CMyStringA m_strFileName; //文件名
    FILE*      m_fpFile; //文件指针
};

//设置文件名
inline void CFileUtil::SetFileName(CMyStringA strFileName)
{
    Close();
    m_strFileName = strFileName;
}

//获取文件名
inline CMyStringA CFileUtil::GetFileName() const
{
    return m_strFileName;
}

inline bool CFileUtil::IsExist() const
{
    return access(m_strFileName, 00) == 0;
}

//是否打开
inline bool CFileUtil::IsOpen() const
{
    return m_fpFile != NULL;
}

//移动文件指针到指定处
inline bool CFileUtil::Seek(int nOffset)
{
    assert(IsOpen());

    return fseek(m_fpFile, nOffset, SEEK_SET) == 0;
}

//移动文件指针至文件开头
inline void CFileUtil::SeekToBegin()
{
    assert(IsOpen());

    fseek(m_fpFile, 0, SEEK_SET);
}

//移动文件指针至文件末，返回文件长度
inline size_t CFileUtil::SeekToEnd()
{
    assert(IsOpen());

    fseek(m_fpFile, 0, SEEK_END);

    return ftell(m_fpFile);
}

//刷新文件缓存区
inline void CFileUtil::Flush()
{
    assert(IsOpen());

    fflush(m_fpFile);
}
