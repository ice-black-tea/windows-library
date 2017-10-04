#pragma once

#include "PEFile.h"

/************************************************************************/
/*                           CImageThunkData                            */
/************************************************************************/

inline CImageThunkData::CImageThunkData(IMAGE_THUNK_DATA thunk, CFileEdit *pFile)
    : IMAGE_THUNK_DATA(thunk)
    , m_pFile(pFile)
{

}

inline bool CImageThunkData::IsOrdinal()
{
    return (u1.Ordinal & 0x80000000) != 0;
}

inline DWORD CImageThunkData::GetOrdinal()
{
    return u1.Ordinal & 0x0000FFFF;
}

inline IMAGE_IMPORT_BY_NAME* CImageThunkData::GetForwarderString()
{
    return ((IMAGE_IMPORT_BY_NAME*)
        ((CPEFile*)m_pFile)->GetBufferByRva(u1.ForwarderString));
}


/************************************************************************/
/*                         CImageThunkDataAry                           */
/************************************************************************/

inline CImageThunkDataAry::CImageThunkDataAry(CFileEdit *pFile)
    : CFileEditAry<IMAGE_THUNK_DATA>(pFile)
{

}

inline CImageThunkData CImageThunkDataAry::GetAt(int nIndex)
{
    return CImageThunkData((*this)[nIndex], m_pFile);
}


/************************************************************************/
/*                           CRelocTypeOffset                           */
/************************************************************************/

inline WORD CRelocTypeOffset::GetType()
{
    return (m_wTypeOffset & 0xF000) >> 12;
}


inline WORD CRelocTypeOffset::GetOffset()
{
    return m_wTypeOffset & 0x0FFF;
}


inline CRelocTypeOffset::operator WORD ()
{
    return m_wTypeOffset;
}

/************************************************************************/
/*                       CImageBaseRelocationPtr                        */
/************************************************************************/

inline CImageBaseRelocationPtr::CImageBaseRelocationPtr(CFileEdit *pFile)
    : CFileEditPtr<IMAGE_BASE_RELOCATION>(pFile)
{

}


inline bool CImageBaseRelocationPtr::IsEnd()
{
    IMAGE_BASE_RELOCATION Reloc = { 0 };

    return memcmp(*this, &Reloc, GetSize()) == 0;
}


inline CImageBaseRelocationPtr CImageBaseRelocationPtr::GetNext()
{
    CImageBaseRelocationPtr pReloc(m_pFile);

    pReloc.SetOffset(GetOffset() + (*this)->SizeOfBlock);

    return pReloc;
}


inline int CImageBaseRelocationPtr::GetTypeOffsetCount()
{
    return (int)(((*this)->SizeOfBlock - GetSize()) / sizeof(WORD));
}


inline CRelocTypeOffsetAry CImageBaseRelocationPtr::GetTypeOffsetAry()
{
    CRelocTypeOffsetAry aryOffset(m_pFile);

    aryOffset.SetOffset(GetEndOffset());
    aryOffset.SetCount(GetTypeOffsetCount());

    return aryOffset;
}


/************************************************************************/
/*                      CImageResourceDirectoryPtr                      */
/************************************************************************/

inline CImageResourceDirectoryPtr::CImageResourceDirectoryPtr(CFileEdit *pFile)
    : CFileEditPtr<IMAGE_RESOURCE_DIRECTORY>(pFile)
{

}

inline CImageResourceDirectoryEntryAry
    CImageResourceDirectoryPtr::GetImageResourceDirectoryEntryAry()
{
    size_t nOffset = GetOffset() + GetSize();
    int nCount = (*this)->NumberOfIdEntries + (*this)->NumberOfNamedEntries;

    CImageResourceDirectoryEntryAry aryResEntry(m_pFile);
    aryResEntry.SetOffset(nOffset);
    aryResEntry.SetCount(nCount);

    return aryResEntry;
}

/************************************************************************/
/*                     CImageResourceDirectoryEntry                     */
/************************************************************************/

inline CImageResourceDirectoryEntry::CImageResourceDirectoryEntry(
    IMAGE_RESOURCE_DIRECTORY_ENTRY& entry, CFileEdit *pFile)
    : IMAGE_RESOURCE_DIRECTORY_ENTRY(entry)
    , m_pFile(pFile)
{

}

inline bool CImageResourceDirectoryEntry::NameIsString()
{
    return IMAGE_RESOURCE_DIRECTORY_ENTRY::NameIsString != 0;
}

inline bool CImageResourceDirectoryEntry::DataIsDirectory()
{
    return IMAGE_RESOURCE_DIRECTORY_ENTRY::DataIsDirectory != 0;
}


inline WORD CImageResourceDirectoryEntry::GetID()
{
    return Id;
}

inline IMAGE_RESOURCE_DIR_STRING_U* CImageResourceDirectoryEntry::GetName()
{
    size_t nOffset = ((CPEFile*)m_pFile)->GetResourceOffset(NameOffset);

    return (IMAGE_RESOURCE_DIR_STRING_U*)m_pFile->GetBuffer(nOffset);
}


inline IMAGE_RESOURCE_DATA_ENTRY* CImageResourceDirectoryEntry::GetData()
{
    size_t nOffset = ((CPEFile*)m_pFile)->GetResourceOffset(OffsetToDirectory);
    
    return (IMAGE_RESOURCE_DATA_ENTRY*)
        m_pFile->GetBuffer(nOffset, sizeof(IMAGE_RESOURCE_DATA_ENTRY));;
}

inline CImageResourceDirectoryPtr CImageResourceDirectoryEntry::GetDirectory()
{
    CImageResourceDirectoryPtr pRes(m_pFile);

    pRes.SetOffset(((CPEFile*)m_pFile)->GetResourceOffset(OffsetToDirectory));

    return pRes;
}

/************************************************************************/
/*                    CImageResourceDirectoryEntryAry                   */
/************************************************************************/

inline CImageResourceDirectoryEntryAry::CImageResourceDirectoryEntryAry(CFileEdit *pFile)
    : CFileEditAry<IMAGE_RESOURCE_DIRECTORY_ENTRY>(pFile)
{

}

inline CImageResourceDirectoryEntry CImageResourceDirectoryEntryAry::GetAt(int nIndex)
{
    return CImageResourceDirectoryEntry((*this)[nIndex], m_pFile);
}