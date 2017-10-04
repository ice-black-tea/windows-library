#pragma once

#include "../Common.h"
#include "../Util/FileEdit.h"
#include "../include/ElfFile/elf.h"

typedef CFileEditPtr<elfhdr> CElfHdrPtr;
typedef CFileEditAry<Elf_Phdr> CElfPhdrAry;
typedef CFileEditAry<Elf_Shdr> CElfShdrAry;
typedef CFileEditAry<Elf_Sym> CElfSymAry;
typedef CFileEditAry<Elf_Sym> CElfDynSymAry;
typedef CFileEditPtr<char> CElfStrTabPtr;
typedef CFileEditPtr<char> CElfDynStrTabPtr;
typedef CFileEditAry<char> CElfCharAry;


struct Elf_Info {
    Elf_Word size;
    Elf_Word linksz;

    Elf_Word dynamic;
    Elf_Word dynamic_size;

    Elf_Word interp;
    Elf_Word interp_size;

    Elf_Word wrprotect_start;
    Elf_Word wrprotect_end;

    Elf_Word strtab;
    Elf_Word symtab;

    Elf_Word hashtab;
    Elf_Word nbucket;
    Elf_Word nchain;
    Elf_Word bucket;
    Elf_Word chain;

    Elf_Word plt_got;

    Elf_Word plt_rel;
    Elf_Word plt_rel_count;

    Elf_Word rel;
    Elf_Word rel_count;

    Elf_Word plt_rela;
    Elf_Word plt_rela_count;

    Elf_Word rela;
    Elf_Word rela_count;

    Elf_Word preinit_array;
    Elf_Word preinit_array_count;

    Elf_Word init_array;
    Elf_Word init_array_count;
    Elf_Word fini_array;
    Elf_Word fini_array_count;

    Elf_Word init_func;
    Elf_Word fini_func;

    /* ARM EABI section used for stack unwinding. */
    Elf_Word ARM_exidx;
    Elf_Word ARM_exidx_count;
};


class CElfFile : public CFileEdit
{
public:
    CElfFile();
    virtual ~CElfFile();

public:
    bool IsValid(); //是否为有效的dex文件
    bool ParseFile(); //读取文件数据并解析文件
    bool ParseHeader();
    bool ParseElfInfo();

    bool RepairFile();

    size_t OffsetToRva(size_t nOffset); //获取相对虚拟地址
    size_t RvaToOffset(size_t nRva); //获取文件偏移

    void *GetBufferByRva(size_t nRva, size_t nLen = 0); //获取可读缓冲区，nLen为0不指定长度
    void *SetBufferByRva(size_t nRva, size_t nLen); //获取可写缓冲区

protected:
    Elf_Word GetNextSection(Elf_Word nOffset);
    int RepairInterp();
    int RepairDynsym(int nDynstr);
    int RepairDynstr();
    int RepairHash();
    int RepairRelDyn(int nDynsym);
    int RepairRelPlt(int nDynsym);
    int RepairPlt();
    int RepairText();
    int RepairInitArray();
    int RepairFiniArray();
    int RepairPreinitArray();
    int RepairDynamic();
    int RepairGot();
    int RepairBss();
    int RepairShstrtab(Elf32_Word nOffset);

public:
    CElfHdrPtr  m_elfHdr;
    CElfPhdrAry m_elfPhdr;
    CElfShdrAry m_elfShdr;
    Elf_Info    m_elfInfo;
};