#include "DBObject.h"


CDBObject::CDBObject()
{
}


CDBObject::~CDBObject()
{
}


bool CDBObject::FindByKey(CDBUtil& dbUtil, CDBTable& dbTable)
{
    if (!dbUtil.IsOpen() && !dbUtil.Open())
    {
        return false;
    }

    CMyString strSql = GetSelectSql(dbTable);
    strSql.Append(GetWhereSql(dbUtil, dbTable));

    _RecordsetPtr pRecordSet = dbUtil.ExecuteRecord(strSql.GetBuffer());

    if (pRecordSet->ADOEOF)
    {
        return false;
    }

    FromFields(pRecordSet->Fields, dbTable);

    return true;
}


bool CDBObject::Find(CDBUtil& dbUtil, CDBTable& dbTable, CDBObjectList& lstObjects)
{
    if (!dbUtil.IsOpen() && !dbUtil.Open())
    {
        return false;
    }

    CMyString strSql = GetSelectSql(dbTable);
    strSql.Append(GetWhereSql(dbUtil, dbTable, false, true));

    _RecordsetPtr pRecordSet = dbUtil.ExecuteRecord(strSql.GetBuffer());

    while (!pRecordSet->ADOEOF)
    {
        CDBObject *pObject = dbTable.m_pfnCreateObject();
        pObject->FromFields(pRecordSet->Fields, dbTable);
        lstObjects.AddTail(pObject);
        pRecordSet->MoveNext();
    }

    return true;
}


bool CDBObject::Select(CDBUtil& dbUtil, CDBTable& dbTable, const TCHAR* pszWhere, const TCHAR* pszOrder)
{
    if (!dbUtil.IsOpen() && !dbUtil.Open())
    {
        return false;
    }

    CMyString strSql = GetSelectSql(dbTable);

    if (pszWhere != NULL && pszWhere[0] != TEXT('\0'))
    {
        strSql.Append(TEXT(" WHERE ")).Append(pszWhere);
    }

    if (pszOrder != NULL && pszOrder[0] != TEXT('\0'))
    {
        strSql.Append(TEXT(" ORDER BY ")).Append(pszOrder);
    }

    _RecordsetPtr pRecordSet = dbUtil.ExecuteRecord(strSql.GetBuffer());

    if (pRecordSet->ADOEOF)
    {
        return false;
    }

    FromFields(pRecordSet->Fields, dbTable);

    return true;
}


bool CDBObject::Select(CDBUtil& dbUtil, CDBTable& dbTable, bool bMacthAll)
{
    if (!dbUtil.IsOpen() && !dbUtil.Open())
    {
        return false;
    }

    CMyString strSql = GetSelectSql(dbTable);
    strSql.Append(GetWhereSql(dbUtil, dbTable, bMacthAll, false));

    _RecordsetPtr pRecordSet = dbUtil.ExecuteRecord(strSql.GetBuffer());

    if (pRecordSet->ADOEOF)
    {
        return false;
    }

    FromFields(pRecordSet->Fields, dbTable);

    return true;
}


bool CDBObject::Select(CDBUtil& dbUtil, CDBTable& dbTable, CDBObjectList& lstObjects, bool bMacthAll)
{
    if (!dbUtil.IsOpen() && !dbUtil.Open())
    {
        return false;
    }

    CMyString strSql = GetSelectSql(dbTable);
    strSql.Append(GetWhereSql(dbUtil, dbTable, bMacthAll, false));

    _RecordsetPtr pRecordSet = dbUtil.ExecuteRecord(strSql.GetBuffer());

    while (!pRecordSet->ADOEOF)
    {
        CDBObject *pObject = dbTable.m_pfnCreateObject();
        pObject->FromFields(pRecordSet->Fields, dbTable);
        lstObjects.AddTail(pObject);
        pRecordSet->MoveNext();
    }

    return true;
}


int CDBObject::Update(CDBUtil& dbUtil, CDBTable& dbTable, CDBObject& src, bool bUpdateAll)
{
    if (!dbUtil.IsOpen() && !dbUtil.Open())
    {
        return 0;
    }

    CMyString strSql = GetUpdateSql(dbUtil, dbTable, bUpdateAll);
    strSql.Append(src.GetWhereSql(dbUtil, dbTable));

    return dbUtil.ExecuteNonQuery(strSql.GetBuffer());
}


int CDBObject::Update(CDBUtil& dbUtil, CDBTable& dbTable, const TCHAR* pszWhere, bool bUpdateAll)
{
    if (!dbUtil.IsOpen() && !dbUtil.Open())
    {
        return 0;
    }

    CMyString strSql = GetUpdateSql(dbUtil, dbTable, bUpdateAll);

    if (pszWhere != NULL && pszWhere[0] != TEXT('\0'))
    {
        strSql.Append(TEXT(" WHERE ")).Append(pszWhere);
    }

    return dbUtil.ExecuteNonQuery(strSql.GetBuffer());
}


bool CDBObject::Insert(CDBUtil& dbUtil, CDBTable& dbTable)
{
    if (!dbUtil.IsOpen() && !dbUtil.Open())
    {
        return false;
    }

    CMyString strSql = GetInsertSql(dbUtil, dbTable);

    return dbUtil.ExecuteNonQuery(strSql.GetBuffer()) == 1;
}


int CDBObject::Delete(CDBUtil& dbUtil, CDBTable& dbTable, bool bMacthAll)
{
    if (!dbUtil.IsOpen() && !dbUtil.Open())
    {
        return false;
    }

    CMyString strSql = GetDeleteSql(dbTable);
    strSql.Append(GetWhereSql(dbUtil, dbTable, bMacthAll, false));

    return dbUtil.ExecuteNonQuery(strSql.GetBuffer());
}


CMyString CDBObject::GetSelectSql(CDBTable& dbTable)
{
    CMyString strSql;
    strSql.GetBufferSetLength(MAXBYTE);
    strSql.ReleaseBuffer();

    strSql.Append(TEXT("SELECT "));

    for (int i = 0; i < dbTable.m_nFieldCount; i++)
    {
        if (i != 0)
        {
            strSql.Append(TEXT(","));
        }

        strSql.Append(dbTable.m_pAryFields[i].m_pszField);
    }

    strSql.Append(" FROM ").Append(dbTable.m_pszTable);

    return strSql;
}


CMyString CDBObject::GetDeleteSql(CDBTable& dbTable)
{
    CMyString strSql;
    strSql.GetBufferSetLength(MAXBYTE);
    strSql.ReleaseBuffer();

    strSql.Append(TEXT("DELETE FROM ")).Append(dbTable.m_pszTable);;

    return strSql;
}


CMyString CDBObject::GetUpdateSql(CDBUtil& dbUtil, CDBTable& dbTable, bool bUpdateAll)
{
    CMyString strSql;
    strSql.GetBufferSetLength(MAXBYTE);
    strSql.ReleaseBuffer();

    strSql.Append(TEXT("UPDATE ")).Append(dbTable.m_pszTable).Append(TEXT(" SET "));

    for (int i = 0, nValid = 0; i < dbTable.m_nFieldCount; i++)
    {
        if (nValid != 0)
        {
            strSql.Append(TEXT(","));
        }

        _variant_t& var = GET_MEMBER(this, dbTable.m_pAryFields[i]);

        if (bUpdateAll || (var.vt != VT_NULL && var.vt != VT_EMPTY))
        {
            strSql.Append(dbTable.m_pAryFields[i].m_pszField).Append(TEXT(" = ?"));
            dbUtil.AddParamter(var, dbTable.m_pAryFields[i].m_Type, dbTable.m_pAryFields[i].m_nMaxLen);
            nValid++;
        }
    }

    return strSql;
}


CMyString CDBObject::GetInsertSql(CDBUtil& dbUtil, CDBTable& dbTable)
{
    CMyString strSql;
    strSql.GetBufferSetLength(MAXBYTE);
    strSql.ReleaseBuffer();

    strSql.Append(TEXT("INSERT INTO ")).Append(dbTable.m_pszTable).Append(TEXT("("));

    for (int i = 0; i < dbTable.m_nFieldCount; i++)
    {
        if (i != 0)
        {
            strSql.Append(TEXT(","));
        }

        strSql.Append(dbTable.m_pAryFields[i].m_pszField);
    }

    strSql.Append(TEXT(") VALUES("));

    for (int i = 0; i < dbTable.m_nFieldCount; i++)
    {
        strSql.Append(i != 0 ? TEXT(",?") : TEXT("?"));
        _variant_t& var = GET_MEMBER(this, dbTable.m_pAryFields[i]);
        dbUtil.AddParamter(var, dbTable.m_pAryFields[i].m_Type, dbTable.m_pAryFields[i].m_nMaxLen);
    }

    strSql.Append(TEXT(")"));

    return strSql;
}


CMyString CDBObject::GetWhereSql(CDBUtil& dbUtil, CDBTable& dbTable)
{
    CMyString strWhere;
    strWhere.GetBufferSetLength(MAXBYTE);
    strWhere.ReleaseBuffer();

    strWhere.Append(TEXT(" WHERE 1 = 1"));

    for (int i = 0; i < dbTable.m_nFieldCount; i++)
    {
        if (dbTable.m_pAryFields[i].m_isPrimaryKey)
        {
            _variant_t& var = GET_MEMBER(this, dbTable.m_pAryFields[i]);
            strWhere.Append(" AND ").Append(dbTable.m_pAryFields[i].m_pszField).Append(" = ? ");
            dbUtil.AddParamter(var, dbTable.m_pAryFields[i].m_Type, dbTable.m_pAryFields[i].m_nMaxLen);
        }
    }

    return strWhere;
}


CMyString CDBObject::GetWhereSql(CDBUtil& dbUtil, CDBTable& dbTable, bool bMacthAll, bool bFuzzy)
{
    CMyString strWhere;
    strWhere.GetBufferSetLength(MAXBYTE);
    strWhere.ReleaseBuffer();

    strWhere.Append(TEXT(" WHERE 1 = 1"));

    for (int i = 0; i < dbTable.m_nFieldCount; i++)
    {
        _variant_t& var = GET_MEMBER(this, dbTable.m_pAryFields[i]);

        if (bMacthAll || (var.vt != VT_NULL && var.vt != VT_EMPTY))
        {
            if (!bFuzzy)
            {
                strWhere.Append(" AND ").Append(dbTable.m_pAryFields[i].m_pszField).Append(" = ? ");
                dbUtil.AddParamter(var, dbTable.m_pAryFields[i].m_Type, dbTable.m_pAryFields[i].m_nMaxLen);
            }
            else
            {
                _bstr_t strVal = (_bstr_t)var;
                if (strVal.length() != 0)
                {
                    strWhere.Append(" AND ").Append(dbTable.m_pAryFields[i].m_pszField).Append(" like ? ");
                    dbUtil.AddParamter(TEXT("%") + strVal + TEXT("%"), adVarChar, -1);
                }
            }
        }
    }

    return strWhere;
}


void CDBObject::FromFields(FieldsPtr pFields, CDBTable& dbTable)
{
    for (int i = 0; i < dbTable.m_nFieldCount; i++)
    {
        _variant_t& var = GET_MEMBER(this, dbTable.m_pAryFields[i]);
        var = pFields->Item[(long)i]->Value;
    }
}