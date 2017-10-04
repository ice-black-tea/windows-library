#pragma once

#include "../Common.h"
#include "DBUtil.h"

/*
//用法：
//.h
class CStudent : public CDBObject
{
    DECLARE_DBOBJECT(CStudent)
public:
    CStudent() { }
    virtual ~CStudent() { }
public:
    _variant_t m_strId;
    _variant_t m_strName;
};

.cpp
BEGIN_FIELD_MAP(CStudent, "tb_student")
    DECLARE_FIELD_STRING_KEY(m_strId, "stu_id", 20)
    DECLARE_FIELD_STRING(m_strName, "stu_name", 20)
END_FIELD_MAP()

//例如增加一条记录
CStudent student;
student.m_strId = "1";
student.m_strName = "2";
student.Insert(CDBUtil());
*/

//声明一个数据库对象
#define DECLARE_DBOBJECT(class_name) \
protected: \
    protected: \
    friend class CDBObject; \
    static class_name* CreateObject(); \
    static CDBTable& _GetTableInfo(); \
    virtual CDBTable& GetTableInfo();

//字段映射表开始
#define BEGIN_FIELD_MAP(class_name, table_name) \
    class_name* class_name::CreateObject() { return new class_name; } \
    CDBTable& class_name::GetTableInfo() { return _GetTableInfo(); } \
    CDBTable& class_name::_GetTableInfo() { \
        static class_name *pNull = NULL; \
        static TCHAR *pszTable = TEXT(table_name); \
        static CDBField dbFields[] = {

//字段映射表结束
#define END_FIELD_MAP() \
        }; \
        static CDBTable dbTable = { pszTable, dbFields, \
            sizeof(dbFields) / sizeof(dbFields[0]), \
            (CDBObject*(*)())CreateObject \
        }; \
        return dbTable; \
    }


//声明字段
#define DECLARE_FIELD(type, member, field, size, key) { type, (size_t)&pNull->member, TEXT(field), size, key },
#define DECLARE_FIELD_INT(member, field) { adInteger, (size_t)&pNull->member, TEXT(field), -1, false },
#define DECLARE_FIELD_INT_KEY(member, field) { adInteger, (size_t)&pNull->member, TEXT(field), -1, true },
#define DECLARE_FIELD_STRING(member, field, size) { adVarChar, (size_t)&pNull->member, TEXT(field), size, false },
#define DECLARE_FIELD_STRING_KEY(member, field, size) { adVarChar, (size_t)&pNull->member, TEXT(field), size, true },

//获取指定指定成员
#define GET_MEMBER(ptr, field) *(_variant_t*)((char*)ptr + field.m_nMember)

class CDBObject;

//字段信息
class CDBField
{
public:
    DataTypeEnum m_Type;         //字段类型
    size_t       m_nMember;      //成员偏移
    TCHAR*       m_pszField;     //字段名称
    long         m_nMaxLen;      //长度
    bool         m_isPrimaryKey; //是否主键
};

//表信息
class CDBTable
{
public:
    TCHAR*    m_pszTable;    //表名
    CDBField* m_pAryFields;  //字段
    int       m_nFieldCount; //字段数量
    CDBObject*(*m_pfnCreateObject)(); //产生对象的函数指针
};

typedef CSmartPtr<CDBObject> CDBObjectPtr;
typedef CMyList<CDBObjectPtr> CDBObjectList;

class CDBObject : public CRefCnt
{
public:
    CDBObject();
    virtual ~CDBObject();

public:
    inline bool FindByKey(CDBUtil& dbUtil); //根据主键查对象
    inline bool Find(CDBUtil& dbUtil, CDBObjectList& lstObjects); //根据对象模糊查找

    inline bool Select(CDBUtil& dbUtil, const TCHAR* pszWhere, const TCHAR* pszOrder = NULL); //根据where语句查找对象
    inline bool Select(CDBUtil& dbUtil, CDBObjectList& lstObjects, const TCHAR* pszWhere, const TCHAR* pszOrder = NULL); //根据where语句查找链表
    inline bool Select(CDBUtil& dbUtil, bool bMacthAll = false); //根据对象查找对象，匹配所有字段/只匹配非空字段
    inline bool Select(CDBUtil& dbUtil, CDBObjectList& lstObjects, bool bMacthAll = false);

    inline int Update(CDBUtil& dbUtil, CDBObject& src, bool bUpdateAll = false); //根据对象条件更新数据
    inline int Update(CDBUtil& dbUtil, const TCHAR* pszWhere, bool bUpdateAll = false); //根据where语句更新数据

    inline bool Insert(CDBUtil& dbUtil); //添加对象
    inline int Delete(CDBUtil& dbUtil, bool bMacthAll = false); //删除对象

    template <typename T>
    static bool Select(CDBUtil& dbUtil, CDBObjectList& lstObjects, const TCHAR* pszWhere = NULL, const TCHAR* pszOrder = NULL);
    template <typename T>
    static int Delete(CDBUtil& dbUtil, const TCHAR* pszWhere = NULL);
    template <typename T>
    static bool EqualKey(T& lhs, T& rhs);

protected:
    bool FindByKey(CDBUtil& dbUtil, CDBTable& dbTable); //根据主键查对象
    bool Find(CDBUtil& dbUtil, CDBTable& dbTable, CDBObjectList& lstObjects); //根据对象模糊查找
    bool Select(CDBUtil& dbUtil, CDBTable& dbTable, const TCHAR* pszWhere, const TCHAR* pszOrder = NULL);
    bool Select(CDBUtil& dbUtil, CDBTable& dbTable, CDBObjectList& lstObjects, const TCHAR* pszWhere, const TCHAR* pszOrder = NULL);
    bool Select(CDBUtil& dbUtil, CDBTable& dbTable, bool bMacthAll = false);
    bool Select(CDBUtil& dbUtil, CDBTable& dbTable, CDBObjectList& lstObjects, bool bMacthAll = false);
    int Update(CDBUtil& dbUtil, CDBTable& dbTable, CDBObject& src, bool bUpdateAll = false); //根据条件更新数据
    int Update(CDBUtil& dbUtil, CDBTable& dbTable, const TCHAR* pszWhere, bool bUpdateAll = false);
    bool Insert(CDBUtil& dbUtil, CDBTable& dbTable);
    int Delete(CDBUtil& dbUtil, CDBTable& dbTable, bool bMacthAll = false);

    static CMyString GetSelectSql(CDBTable& dbTable);
    static CMyString GetDeleteSql(CDBTable& dbTable);
    CMyString GetUpdateSql(CDBUtil& dbUtil, CDBTable& dbTable, bool bUpdateAll);
    CMyString GetInsertSql(CDBUtil& dbUtil, CDBTable& dbTable);
    CMyString GetWhereSql(CDBUtil& dbUtil, CDBTable& dbTable);
    CMyString GetWhereSql(CDBUtil& dbUtil, CDBTable& dbTable, bool bMacthAll, bool bFuzzy);
    void FromFields(FieldsPtr pFields, CDBTable& dbTable);

    //调用宏（DECLARE_DBOBJECT）即可，不用声明和实现
    virtual CDBTable& GetTableInfo() = 0;
};


inline bool CDBObject::FindByKey(CDBUtil& dbUtil) //根据主键查对象
{
    return FindByKey(dbUtil, GetTableInfo());
}


inline bool CDBObject::Find(CDBUtil& dbUtil, CDBObjectList& lstObjects) //根据对象模糊查找
{
    return Find(dbUtil, GetTableInfo(), lstObjects);
}


inline bool CDBObject::Select(CDBUtil& dbUtil, const TCHAR* pszWhere, const TCHAR* pszOrder)
{
    return Select(dbUtil, GetTableInfo(), pszWhere, pszOrder);
}


inline bool CDBObject::Select(CDBUtil& dbUtil, CDBObjectList& lstObjects, const TCHAR* pszWhere, const TCHAR* pszOrder)
{
    return Select(dbUtil, GetTableInfo(), lstObjects, pszWhere, pszOrder);
}


inline bool CDBObject::Select(CDBUtil& dbUtil, bool bMacthAll)
{
    return Select(dbUtil, GetTableInfo(), bMacthAll);
}


inline bool CDBObject::Select(CDBUtil& dbUtil, CDBObjectList& lstObjects, bool bMacthAll)
{
    return Select(dbUtil, GetTableInfo(), lstObjects, bMacthAll);
}


inline int CDBObject::Update(CDBUtil& dbUtil, CDBObject& src, bool bUpdateAll) //根据条件更新数据
{
    return Update(dbUtil, GetTableInfo(), src, bUpdateAll);
}


inline int CDBObject::Update(CDBUtil& dbUtil, const TCHAR* pszWhere, bool bUpdateAll)
{
    return Update(dbUtil, GetTableInfo(), pszWhere, bUpdateAll);
}


inline bool CDBObject::Insert(CDBUtil& dbUtil)
{
    return Insert(dbUtil, GetTableInfo());
}


inline int CDBObject::Delete(CDBUtil& dbUtil, bool bMacthAll)
{
    return Delete(dbUtil, GetTableInfo(), bMacthAll);
}


template <typename T>
bool CDBObject::Select(CDBUtil& dbUtil, CDBObjectList& lstObjects, const TCHAR* pszWhere, const TCHAR* pszOrder)
{
    if (!dbUtil.IsOpen() && !dbUtil.Open())
    {
        return false;
    }

    CDBTable& dbTable = T::_GetTableInfo();

    CMyString strSql = GetSelectSql(dbTable);

    if (pszWhere != NULL && pszWhere[0] != TEXT('\0'))
    {
        strSql.Append(TEXT(" WHERE "));
        strSql.Append(pszWhere);
    }

    if (pszOrder != NULL && pszOrder[0] != TEXT('\0'))
    {
        strSql.Append(TEXT(" ORDER BY "));
        strSql.Append(pszOrder);
    }

    _RecordsetPtr pRecordSet = dbUtil.ExecuteRecord(strSql.GetBuffer());

    while (!pRecordSet->ADOEOF)
    {
        CDBObject *pObject = new T();
        pObject->FromFields(pRecordSet->Fields, dbTable);
        lstObjects.AddTail(pObject);
        pRecordSet->MoveNext();
    }

    return true;
}


template <typename T>
int CDBObject::Delete(CDBUtil& dbUtil, const TCHAR* pszWhere)
{
    if (!dbUtil.IsOpen() && !dbUtil.Open())
    {
        return false;
    }

    CDBTable dbTable = T::_GetTableInfo();

    CMyString strSql = GetDeleteSql(dbTable);

    if (pszWhere != NULL && pszWhere[0] != TEXT('\0'))
    {
        strSql.Append(TEXT(" WHERE "));
        strSql.Append(pszWhere);
    }

    return dbUtil.ExecuteNonQuery(strSql.GetBuffer());
}


template <typename T>
static bool CDBObject::EqualKey(T& lhs, T& rhs)
{
    CDBTable& dbTable = T::_GetTableInfo();

    for (int i = 0; i < dbTable.m_nFieldCount; i++)
    {
        if (dbTable.m_pAryFields[i].m_isPrimaryKey)
        {
            if (GET_MEMBER(&lhs, dbTable.m_pAryFields[i]) !=
                GET_MEMBER(&rhs, dbTable.m_pAryFields[i]))
            {
                return false;
            }
        }
    }

    return true;
}






// class CStudent : public CDBObject
// {
// protected: \
//     static CStudent* CreateObject() { return new CStudent; } \
//     virtual CDBTable* GetTableInfo(CDBTable& dbTable);
// public:
//     CStudent() { }
//     virtual ~CStudent() { }
// public:
//     _variant_t m_strId;
//     _variant_t m_strName;
// };
// 
// 
// CDBTable* CStudent::GetTableInfo() {
// 
//     static TCHAR *pszTable = TEXT("tb_student");
// 
//     static CDBField dbFields[] = {
//             { adInteger, (size_t)&this->m_strId - (size_t)this, TEXT("stu_id"), 20, false },
//     };
// 
//     static CDBTable dbTable = {
//         pszTable,
//         dbFields,
//         sizeof(dbFields) / sizeof(dbFields[0]),
//         (CDBObject*(*)())CreateObject
//     };
// 
//     return &dbTable;
// }
// 
// 
