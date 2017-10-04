#include "../Common.h"
#include "DBUtil.h"
#include "Config.h"
#include <tchar.h>

#pragma warning ( disable : 4290 )


CDBUtil::CDBUtil()
{
    static _bstr_t strConnection = GetConfigString(CONFIG_FILE, CONFIG_APP, CONFIG_KEY_CONNECTION);
    static _bstr_t strUserID = GetConfigString(CONFIG_FILE, CONFIG_APP, CONFIG_KEY_USERID);
    static _bstr_t strPassword = GetConfigString(CONFIG_FILE, CONFIG_APP, CONFIG_KEY_PASSWORD);
    m_strConnection = strConnection;
    m_strUserID = strUserID;
    m_strPassword = strPassword;
}


CDBUtil::CDBUtil(_bstr_t strConnection, _bstr_t strUserID, _bstr_t strPassword)
    : m_strConnection(strConnection)
    , m_strUserID(strUserID)
    , m_strPassword(strPassword)
{

}


CDBUtil::~CDBUtil()
{
    Close();
}


bool CDBUtil::Open(long lOptions) throw(_com_error)
{
    try
    {
        HRESULT hr = m_pConnection.CreateInstance(__uuidof(Connection));

        if (FAILED(hr))
        {
            m_pConnection = NULL;
            return false;
        }

        hr = m_pConnection->Open(m_strConnection, m_strUserID, m_strPassword, lOptions);

        if (FAILED(hr))
        {
            m_pConnection = NULL;
            return false;
        }

        hr = m_pCommand.CreateInstance(__uuidof(Command));

        if (FAILED(hr))
        {
            m_pConnection = NULL;
            return false;
        }

        m_pCommand->ActiveConnection = m_pConnection;
    }
    catch (_com_error e)
    {
        m_pConnection = NULL;
        throw e;
    }

    return true;
}


void CDBUtil::Close()
{
    try
    {
        if (IsOpen())
        {
            m_pConnection->Close();
        }
    }
    catch (_com_error e)
    {

    }

    m_pConnection = NULL;
    m_pCommand = NULL;
}
