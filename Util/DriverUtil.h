#pragma once

#include <Windows.h>
#include <Winsvc.h>
#include "../StringT.h"

class CDriverUtil
{
public:
    CDriverUtil();
    CDriverUtil(CMyString strPathName);
    virtual ~CDriverUtil();

public:
    void SetPathName(CMyString strPathName);

    DWORD Install(BOOL bStart = FALSE);
    DWORD Start();
    DWORD Stop();
    DWORD Uninstall(BOOL bStop = FALSE);
    DWORD ReInstall();

protected:
    CMyString m_strPathName;
    CMyString m_strServiceName;
};

