#include "../include/DexFile/sha1.h"
#include "../include/zlib/zlib.h"
#include "../Common.h"
#include "DexFile.h"
#include <stddef.h>

#pragma warning (disable : 4996)

static uintptr_t readULEB128(const u1** data)
{
    uintptr_t result = 0;
    uintptr_t shift = 0;
    unsigned char byte;
    const u1* p = *data;
    do {
        byte = *p++;
        result |= (byte & 0x7f) << shift;
        shift += 7;
    } while (byte & 0x80);
    *data = p;
    return result;
}

static uintptr_t writeULEB128(uintptr_t n, u1** data)
{
    uintptr_t result = 0;
    u1* p = *data;
    *p = n & 0x7f;
    while ((n = n >> 7) > 0)
    {
        *p++ |= 0x80;
        *p = n & 0x7f;
    }
    result = p + 1 - *data;
    *data = p + 1;
    return result;
}


CDexFile::CDexFile()
    : m_pHeader(this)
    , m_aryLink(this)
    , m_pMapList(this)
    , m_aryStringId(this)
    , m_aryTypeId(this)
    , m_aryProtoId(this)
    , m_aryFieldId(this)
    , m_aryMethodId(this)
    , m_aryClassDef(this)
    , m_aryData(this)
{
}


CDexFile::~CDexFile()
{
}

//读取文件数据并验证格式
bool CDexFile::ParseFile()
{
    bool bRet = false;

    if ((IsOpen() || Open()) && Read() && ParseHeader())
    {
        bRet = true;
        Close();
    }

    return bRet;
}

//修复文件
bool CDexFile::RepairFile()
{
    m_pHeader->fileSize = m_Buffer.GetReadableSize();
    m_pHeader->headerSize = sizeof(*m_pHeader);

    //修复sha-1
    unsigned char sha1Digest[kSHA1DigestLen];
    SHA1_CTX context;
    SHA1Init(&context);
    SHA1Update(&context, (unsigned char*)&m_pHeader->fileSize,
        m_Buffer.GetReadableSize() - offsetof(DexHeader, fileSize));
    SHA1Final(sha1Digest, &context);
    memcpy(m_pHeader->signature, sha1Digest, sizeof(sha1Digest));

    //修复校验和
    uLong adler = adler32(0L, Z_NULL, 0);
    adler = adler32(adler, (Bytef*)m_pHeader->signature,
        m_Buffer.GetReadableSize() - offsetof(DexHeader, signature));
    m_pHeader->checksum = adler;

    return true;
}

//修复文件
// bool CDexFile::ConfuseType()
// {
//     assert(GetDexHeader() != NULL);
// 
//     DexHeader *pHeader = GetDexHeader();
// 
//     DexTypeId *pTypeId = GetDexTypeIds();
//     if (pTypeId == NULL)
//     {
//         return false;
//     }
// 
//     DexStringId *pStringId = GetDexStringIds();
//     if (pStringId == NULL)
//     {
//         return false;
//     }
// 
//     CMyArray<CMyStringA> aryString(pHeader->stringIdsSize);
//     for (u4 i = 0; i < pHeader->stringIdsSize; i++)
//     {
//         aryString[i] = GetStringData(&pStringId[i]);
//     }
// 
//     for (u4 i = 0; i < pHeader->typeIdsSize; i++)
//     {
//         u4 idx = pTypeId[i].descriptorIdx;
//         CMyStringA str = aryString[idx];
//         for (int i = 0; i < 0x90; i++) {
//             str += 'c';
//         }
//         aryString[idx] = str;
//     }
// 
//     int offset = pStringId[0].stringDataOff;
//     for (u4 i = 0; i < pHeader->stringIdsSize; i++)
//     {
//         pStringId[i].stringDataOff = offset;
//         offset += SetSrtingData(&pStringId[i], aryString[i]);
//     }
// 
//     return true;
// }

//获取Dex头
bool CDexFile::ParseHeader()
{
    assert(!m_Buffer.IsEmpty());

    m_pHeader.SetOffset((size_t)0);
    m_aryLink.SetOffset(m_pHeader->linkOff);
    m_aryLink.SetCount(m_pHeader->linkSize);
    m_pMapList.SetOffset(m_pHeader->mapOff);
    m_aryStringId.SetOffset(m_pHeader->stringIdsOff);
    m_aryStringId.SetCount(m_pHeader->stringIdsSize);
    m_aryTypeId.SetOffset(m_pHeader->typeIdsOff);
    m_aryTypeId.SetCount(m_pHeader->typeIdsSize);
    m_aryProtoId.SetOffset(m_pHeader->protoIdsOff);
    m_aryProtoId.SetCount(m_pHeader->protoIdsSize);
    m_aryFieldId.SetOffset(m_pHeader->fieldIdsOff);
    m_aryFieldId.SetCount(m_pHeader->fieldIdsSize);
    m_aryMethodId.SetOffset(m_pHeader->methodIdsOff);
    m_aryMethodId.SetCount(m_pHeader->methodIdsSize);
    m_aryClassDef.SetOffset(m_pHeader->classDefsOff);
    m_aryClassDef.SetCount(m_pHeader->classDefsSize);
    m_aryData.SetOffset(m_pHeader->dataOff);
    m_aryData.SetCount(m_pHeader->dataSize);

    return true;
}

#define MAP_LIST_SIZE(cnt) \
    (sizeof(((DexMapList*)NULL)->size) + \
     sizeof(((DexMapList*)NULL)->list[0]) * (cnt - 1))

size_t CDexFile::InitOffset(CDexFile& file)
{
    size_t nOffset = 0;

    //Dex头
    m_pHeader.SetOffset(nOffset);
    m_pHeader->endianTag = kDexEndianConstant;
    memcpy(m_pHeader->magic, file.m_pHeader->magic, sizeof(m_pHeader->magic));
    nOffset += m_pHeader.GetSize();

    //字符串
    m_aryStringId.SetOffset(nOffset);
    m_aryStringId.SetCount(file.m_aryStringId.GetCount());
    m_pHeader->stringIdsOff = m_aryStringId.GetOffset();
    m_pHeader->stringIdsSize = m_aryStringId.GetCount();
    nOffset += m_aryStringId.GetSize();

    //类型
    m_aryTypeId.SetOffset(nOffset);
    m_aryTypeId.SetCount(file.m_aryTypeId.GetCount());
    m_pHeader->typeIdsOff = m_aryTypeId.GetOffset();
    m_pHeader->typeIdsSize = m_aryTypeId.GetCount();
    nOffset += m_aryTypeId.GetSize();

    //原型
    m_aryProtoId.SetOffset(nOffset);
    m_aryProtoId.SetCount(file.m_aryProtoId.GetCount());
    m_pHeader->protoIdsOff = m_aryProtoId.GetOffset();
    m_pHeader->protoIdsSize = m_aryProtoId.GetCount();
    nOffset += m_aryProtoId.GetSize();

    //字段
    m_aryFieldId.SetOffset(nOffset);
    m_aryFieldId.SetCount(file.m_aryFieldId.GetCount());
    m_pHeader->fieldIdsOff = m_aryFieldId.GetOffset();
    m_pHeader->fieldIdsSize = m_aryFieldId.GetCount();
    nOffset += m_aryFieldId.GetSize();

    //方法
    m_aryMethodId.SetOffset(nOffset);
    m_aryMethodId.SetCount(file.m_aryMethodId.GetCount());
    m_pHeader->methodIdsOff = m_aryMethodId.GetOffset();
    m_pHeader->methodIdsSize = m_aryMethodId.GetCount();
    nOffset += m_aryMethodId.GetSize();

    //类
    m_aryClassDef.SetOffset(nOffset);
    m_aryClassDef.SetCount(file.m_aryClassDef.GetCount());
    m_pHeader->classDefsOff = m_aryClassDef.GetOffset();
    m_pHeader->classDefsSize = m_aryClassDef.GetCount();
    nOffset += m_aryClassDef.GetSize();

    //数据
    m_aryData.SetOffset(nOffset);
    m_aryData.SetCount(file.m_aryData.GetCount());
    m_pHeader->dataOff = m_aryData.GetOffset();
    m_pHeader->dataSize = m_aryData.GetCount();
    nOffset += m_aryData.GetSize();

    //链接
    m_aryLink.SetOffset(nOffset);
    m_aryLink.SetCount(file.m_aryLink.GetCount());
    m_pHeader->linkOff = m_aryLink.GetOffset();
    m_pHeader->linkSize = m_aryLink.GetCount();
    nOffset += m_aryLink.GetSize();

    return nOffset;
}

//混淆Dex文件
bool CDexFile::ConfuseFile(CDexFile& file)
{
    size_t nOffset = InitOffset(file);

    //连接
    memcpy(m_aryLink, file.m_aryLink, m_aryLink.GetSize());
    memcpy(m_aryTypeId, file.m_aryTypeId, m_aryTypeId.GetSize());
    memcpy(m_aryFieldId, file.m_aryFieldId, m_aryFieldId.GetSize());
    memcpy(m_aryMethodId, file.m_aryMethodId, m_aryMethodId.GetSize());

    CMyArray<CMyStringA> aryStrings(file.m_aryStringId.GetCount());
    for (int i = 0; i < aryStrings.GetCount(); i++)
    {
        aryStrings[i] = file.GetStringData(&file.m_aryStringId[i]);
    }

    for (int i = 0; i < m_aryTypeId.GetCount(); i++)
    {
        u4 idx = m_aryTypeId[i].descriptorIdx;
        CMyStringA str = aryStrings[idx];
        for (int i = 0; i < 0x90; i++) {
            str += 'c';
        }
        aryStrings[idx] = str;
    }

    for (int i = 0; i < m_aryStringId.GetCount(); i++)
    {
        m_aryStringId[i].stringDataOff = nOffset;
        CMyStringA str = aryStrings[i];
        nOffset += SetSrtingData(nOffset, aryStrings[i]);
    }

//     m_pMapList.SetOffset(file.m_pHeader->mapOff);
//     size_t nSize = MAP_LIST_SIZE(file.m_pMapList->size);
//     memcpy(SetBuffer(m_pHeader->mapOff, nSize), file.m_pMapList, nSize);

    RepairFile();

    return true;
}

//获取字符串数据
CMyStringA CDexFile::GetStringData(DexStringId* pStringId)
{
    const u1* ptr = (u1*)GetBuffer(pStringId->stringDataOff);

    // Skip the uleb128 length.
    while (*(ptr++) > 0x7f) /* empty */;

    return (const char*)ptr;
}

//设置字符串数据
int CDexFile::SetSrtingData(size_t nStringDataOff, CMyStringA& strData)
{
    u1* ptr = (u1*)SetBuffer(nStringDataOff, strData.GetLength());

    int nLeb128 = writeULEB128(strData.GetLength(), &ptr);
    int nLen = strData.GetLength() + 1;

    strcpy((char*)SetBuffer(GetOffset(ptr), nLen), strData);

    return nLeb128 + nLen;
}