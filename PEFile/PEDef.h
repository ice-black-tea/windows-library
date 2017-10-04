#pragma once

#include <Windows.h>
#include <assert.h>
#include "../Common.h"
#include "../Util/FileEdit.h"

class CPEFile;

//文件头
typedef CFileEditPtr<IMAGE_DOS_HEADER> CImageDosHeaderPtr;
typedef CFileEditPtr<IMAGE_NT_HEADERS> CImageNtHeadersPtr;
typedef CFileEditAry<IMAGE_DATA_DIRECTORY> CImageDataDirectoryAry;
typedef CFileEditAry<IMAGE_SECTION_HEADER> CImageSectionHeaderAry;

//导出表
typedef CFileEditAry<WORD> CWordAry;
typedef CFileEditAry<DWORD> CDwordAry;
typedef CFileEditPtr<IMAGE_EXPORT_DIRECTORY> CImageExportDirectoryPtr;

//导入表
class CImageThunkData;
class CImageThunkDataAry;
typedef CFileEditAry<IMAGE_IMPORT_DESCRIPTOR> CImageImportDescriptorAry;

//资源表
class CImageResourceDirectoryPtr;
class CImageResourceDirectoryEntry;
class CImageResourceDirectoryEntryAry;

//重定位表
class CImageBaseRelocationPtr;
class CRelocTypeOffset;
typedef CFileEditAry<CRelocTypeOffset> CRelocTypeOffsetAry;

//Tls
typedef CFileEditPtr<IMAGE_TLS_DIRECTORY> CImageTlsDirectoryPtr;


/************************************************************************/
/*                           CImageThunkData                            */
/************************************************************************/

class CImageThunkData : public IMAGE_THUNK_DATA
{
protected:
    friend class CImageThunkDataAry;
    CImageThunkData(IMAGE_THUNK_DATA thunk, CFileEdit *pFile);

public:
    bool IsOrdinal();
    DWORD GetOrdinal();
    IMAGE_IMPORT_BY_NAME* GetForwarderString();

protected:
    CFileEdit *m_pFile;
};


/************************************************************************/
/*                         CImageThunkDataAry                           */
/************************************************************************/

class CImageThunkDataAry : public CFileEditAry < IMAGE_THUNK_DATA >
{
public:
    CImageThunkDataAry(CFileEdit *pFile = NULL);

public:
    CImageThunkData GetAt(int nIndex);
};


/************************************************************************/
/*                           CRelocTypeOffset                           */
/************************************************************************/

class CRelocTypeOffset
{
public:
    WORD GetType();
    WORD GetOffset();
    operator WORD ();

protected:
    WORD m_wTypeOffset;
};


/************************************************************************/
/*                       CImageBaseRelocationPtr                        */
/************************************************************************/

class CImageBaseRelocationPtr : public CFileEditPtr < IMAGE_BASE_RELOCATION >
{
public:
    CImageBaseRelocationPtr(CFileEdit *pFile = NULL);

public:
    bool IsEnd();
    CImageBaseRelocationPtr GetNext();
    int GetTypeOffsetCount();
    CRelocTypeOffsetAry GetTypeOffsetAry();
};


/************************************************************************/
/*                      CImageResourceDirectoryPtr                      */
/************************************************************************/

class CImageResourceDirectoryPtr
    : public CFileEditPtr < IMAGE_RESOURCE_DIRECTORY >
{
public:
    CImageResourceDirectoryPtr(CFileEdit *pFile = NULL);

public:
    #define GetImageResDirEntryAry GetImageResourceDirectoryEntryAry
    CImageResourceDirectoryEntryAry GetImageResourceDirectoryEntryAry();
};

typedef CImageResourceDirectoryPtr CImageResDirPtr;

/************************************************************************/
/*                     CImageResourceDirectoryEntry                     */
/************************************************************************/

class CImageResourceDirectoryEntry
    : public IMAGE_RESOURCE_DIRECTORY_ENTRY
{
public:
    CImageResourceDirectoryEntry(
        IMAGE_RESOURCE_DIRECTORY_ENTRY& entry, CFileEdit *pFile);

public:
    bool NameIsString();
    bool DataIsDirectory();

    WORD GetID();
    IMAGE_RESOURCE_DIR_STRING_U* GetName();

    IMAGE_RESOURCE_DATA_ENTRY* GetData();
    CImageResourceDirectoryPtr GetDirectory();

protected:
    CFileEdit *m_pFile;
};

typedef CImageResourceDirectoryEntry CImageResDirEntry;

/************************************************************************/
/*                    CImageResourceDirectoryEntryAry                   */
/************************************************************************/

class CImageResourceDirectoryEntryAry :
    public CFileEditAry < IMAGE_RESOURCE_DIRECTORY_ENTRY >
{
public:
    CImageResourceDirectoryEntryAry(CFileEdit *pFile = NULL);

public:
    CImageResourceDirectoryEntry GetAt(int nIndex);
};

typedef CImageResourceDirectoryEntryAry CImageResDirEntryAry;