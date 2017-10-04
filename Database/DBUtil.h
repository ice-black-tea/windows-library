#pragma once

#pragma warning ( push )
//#pragma warning ( disable : 4146 )
#pragma warning ( disable : 4290 )
#import "msado15.dll" rename("EOF", "ADOEOF") no_namespace 
#include <windows.h>

const LPTSTR CONFIG_FILE = TEXT("config.ini"); //ini文件名
const LPTSTR CONFIG_APP = TEXT("ADO"); //节名
const LPTSTR CONFIG_KEY_CONNECTION = TEXT("Connection"); //连接字符串
const LPTSTR CONFIG_KEY_USERID = TEXT("UserID"); //用户名
const LPTSTR CONFIG_KEY_PASSWORD = TEXT("Password"); //密码

/************************************************************************/
/*                            数据库工具类                              */
/************************************************************************/
class CDBUtil
{
    //使用前需要初始化CoInitialize(NULL);
    //使用完调用CoUninitialize();
public:
    CDBUtil(); //默认从应用程序所在目录读取配置文件的连接字符串
    CDBUtil(_bstr_t strConnection, _bstr_t strUserID = "", _bstr_t strPassword = "");
    ~CDBUtil();

public:
    //打开连接
    bool Open(long lOptions = adConnectUnspecified) throw(_com_error);
    //关闭连接
    void Close();

    //是否打开
    inline bool IsOpen() throw(_com_error);

    //添加一个参数
    inline void AddParamter(const _variant_t& vtValue = vtMissing, DataTypeEnum Type = adInteger,
        long lSize = -1, ParameterDirectionEnum Direction = adParamInput, _bstr_t strName = "") throw(_com_error);
    //移除参数列表
    inline void RefreshParamter() throw(_com_error);

    //执行sql返回影响行数
    inline int ExecuteNonQuery(_bstr_t strSql, _variant_t *pParams = NULL, long lOptions = adCmdText) throw(_com_error);
    //执行sql返回第一行第一列
    inline _variant_t ExecuteScalar(_bstr_t strSql, _variant_t *pParams = NULL, long lOptions = adCmdText) throw(_com_error);
    //执行sql返回记录集
    inline _RecordsetPtr ExecuteRecord(_bstr_t strSql, _variant_t *pParams = NULL, long lOptions = adCmdText) throw(_com_error);

protected:
    _bstr_t m_strConnection;
    _bstr_t m_strUserID;
    _bstr_t m_strPassword;
    _CommandPtr m_pCommand;
    _ConnectionPtr m_pConnection;
};


inline bool CDBUtil::IsOpen() throw(_com_error)
{
    return m_pConnection != NULL && m_pConnection->GetState() != adStateClosed;
}


inline void CDBUtil::AddParamter(const _variant_t& vtValue, DataTypeEnum Type, long lSize, ParameterDirectionEnum Direction, _bstr_t strName) throw(_com_error)
{
    m_pCommand->Parameters->Append(m_pCommand->CreateParameter(strName, Type, Direction, lSize, vtValue));
}


inline void CDBUtil::RefreshParamter() throw(_com_error)
{
    while (m_pCommand->Parameters->GetCount() > 0)
        m_pCommand->Parameters->Delete(m_pCommand->Parameters->GetCount() - 1);
}


inline int CDBUtil::ExecuteNonQuery(_bstr_t strSql, _variant_t *pParams, long lOptions) throw(_com_error)
{
    _variant_t t;
    m_pCommand->CommandText = strSql;
    m_pCommand->Execute(&t, pParams, lOptions);
    RefreshParamter();
    return t;
}


inline _variant_t CDBUtil::ExecuteScalar(_bstr_t strSql, _variant_t *pParams, long lOptions) throw(_com_error)
{
    m_pCommand->CommandText = strSql;
    _RecordsetPtr rs = m_pCommand->Execute(NULL, pParams, lOptions);
    RefreshParamter();
    return !rs->ADOEOF ? rs->Fields->Item[(long)0]->Value : _variant_t();
}


inline _RecordsetPtr CDBUtil::ExecuteRecord(_bstr_t strSql, _variant_t *pParams, long lOptions) throw(_com_error)
{
    m_pCommand->CommandText = strSql;
    _RecordsetPtr rs = m_pCommand->Execute(NULL, pParams, lOptions);
    RefreshParamter();
    return rs;
}

#pragma warning ( pop )