#include "Config.h"
#include <Windows.h>
#include <tchar.h>

CMyString GetCurrentPath()
{
    TCHAR szPath[MAX_PATH] = { 0 };

    //拿到当前exe路径
    HMODULE hModule = GetModuleHandle(0);

    GetModuleFileName(hModule, szPath, MAX_PATH);

    //拿到exe所在路径
    TCHAR *pDest = _tcsrchr(szPath, TEXT('\\'));

    if (pDest != NULL)
    {
        pDest[1] = TEXT('\0');
    }

    return szPath;
}


CMyString GetConfigString(const TCHAR *pszPath, const TCHAR *pszAppName, const TCHAR *pszKeyName)
{
    CMyString strPath = GetCurrentPath();
    strPath.Append(pszPath);

    TCHAR szConn[MAXBYTE] = { 0 };
    DWORD dwRet = GetPrivateProfileString(
        pszAppName,             // INI文件中的一个字段名[节名]可以有很多个节名
        pszKeyName,             // lpAppName 下的一个键名，也就是里面具体的变量名
        TEXT(""),               // 如果lpReturnedString为空,则把个变量赋给lpReturnedString
        szConn,                 // 存放键值的指针变量,用于接收INI文件中键值(数据)的接收缓冲区
        MAXBYTE,                // lpReturnedString的缓冲区大小
        strPath);               // INI文件的路径

    return szConn;
}