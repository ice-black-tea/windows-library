#pragma once

#include "../Common.h"
#include "../Util/FileEdit.h"
#include "../include/DexFile/DexFile.h"

typedef CFileEditPtr<DexHeader>    CDexHeaderPtr;
typedef CFileEditAry<DexLink>      CDexLinkAry;
typedef CFileEditPtr<DexMapList>   CDexMapListPtr;
typedef CFileEditAry<DexStringId>  CDexStringIdAry;
typedef CFileEditAry<DexTypeId>    CDexTypeIdAry;
typedef CFileEditAry<DexProtoId>   CDexProtoIdAry;
typedef CFileEditAry<DexFieldId>   CDexFieldIdAry;
typedef CFileEditAry<DexMethodId>  CDexMethodIdAry;
typedef CFileEditAry<DexClassDef>  CDexClassDefAry;
typedef CFileEditAry<char>         CDexDataAry;

class CDexFile : public CFileEdit
{
public:
    CDexFile();
    virtual ~CDexFile();
    
public:
    bool ParseFile(); //读取文件数据并解析文件
    bool ParseHeader(); //解析Dex头
    bool RepairFile(); //修复文件
    bool ConfuseFile(CDexFile& file); //混淆Dex文件

    CMyStringA GetStringData(DexStringId* pStringId);
    int SetSrtingData(size_t nStringDataOff, CMyStringA& strData);

    DexTypeId *GetDexTypeIds();

private:
    size_t InitOffset(CDexFile& file);

public:
    CDexHeaderPtr   m_pHeader;
    CDexLinkAry     m_aryLink;
    CDexMapListPtr  m_pMapList;
    CDexStringIdAry m_aryStringId;
    CDexTypeIdAry   m_aryTypeId;
    CDexProtoIdAry  m_aryProtoId;
    CDexFieldIdAry  m_aryFieldId;
    CDexMethodIdAry m_aryMethodId;
    CDexClassDefAry m_aryClassDef;
    CDexDataAry     m_aryData;
};