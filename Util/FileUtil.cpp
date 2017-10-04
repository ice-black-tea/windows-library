#include "FileUtil.h"

#pragma warning(disable:4996)

//构造函数
CFileUtil::CFileUtil()
    : m_fpFile(NULL)
{

}

//构造函数
CFileUtil::CFileUtil(const char *pszName)
    : m_fpFile(NULL)
    , m_strFileName(pszName)
{
    
}

//构造函数
CFileUtil::CFileUtil(const wchar_t *pszName)
    : m_fpFile(NULL)
    , m_strFileName(pszName)
{
    
}

//析构函数
CFileUtil::~CFileUtil()
{
    Close();
}

//创建文件
bool CFileUtil::Create()
{
    if (IsExist())
    {
        return true;
    }

    FILE *fpCreate = fopen(m_strFileName, "a");

    if (fpCreate == NULL)
    {
        return false;
    }

    fclose(fpCreate);

    return true;
}

//清空文件
bool CFileUtil::Clear()
{
    Close();

    FILE *fpCreate = fopen(m_strFileName, "w");

    if (fpCreate == NULL)
    {
        return false;
    }

    fclose(fpCreate);

    return true;
}


//打开文件
bool CFileUtil::Open(int nMode)
{
    assert(!IsOpen());

    int nAccess = (nMode & FILE_WRITE) != 0 ? FILE_WRITE : FILE_READ;
    int nCreate = (nMode & FILE_CREATE_ALWAYS) != 0 ? FILE_CREATE_ALWAYS :
                  (nMode & FILE_CREATE) != 0 ? FILE_CREATE : FILE_EXIST;

    char *pszMode = NULL;

    switch (nAccess)
    {
    case FILE_READ: //读
        switch (nCreate)
        {
        case FILE_EXIST: //文件必须存在
            pszMode = "rb";
            break;
        }
        break;
    case FILE_WRITE: //写
        switch (nCreate)
        {
        case FILE_EXIST: //文件必须存在
            pszMode = "rb+";
            break;
        case FILE_CREATE: //文件不存在则创建
            Create();
            pszMode = "rb+";
            break;
        case FILE_CREATE_ALWAYS: //文件总是创建
            pszMode = "wb+";
            break;
        }
        break;
    }

    assert(pszMode != NULL);

    m_fpFile = fopen(m_strFileName, pszMode);

    return IsOpen();
}

//关闭文件
void CFileUtil::Close()
{
    if (IsOpen())
    {
        fclose(m_fpFile);
        m_fpFile = NULL;
    }
}

//写文件
size_t CFileUtil::Write(const void *pBuff, size_t nSize)
{
    assert(IsOpen());

    return fwrite(pBuff, 1, nSize, m_fpFile);
}

//写文件
size_t CFileUtil::Write(int nOffset, const void *pBuff, size_t nSize)
{
    assert(IsOpen());

    if (fseek(m_fpFile, nOffset, SEEK_SET) != 0)
    {
        return false;
    }

    return fwrite(pBuff, 1, nSize, m_fpFile);
}

//读文件
size_t CFileUtil::Read(void *pBuff, size_t nSize)
{
    assert(IsOpen());

    return fread(pBuff, 1, nSize, m_fpFile);
}

//读文件
size_t CFileUtil::Read(int nOffset, void *pBuff, size_t nSize)
{
    assert(IsOpen());

    if (fseek(m_fpFile, nOffset, SEEK_SET) != 0)
    {
        return 0;
    }

    return fread(pBuff, 1, nSize, m_fpFile);
}