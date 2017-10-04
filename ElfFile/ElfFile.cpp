#include "ElfFile.h"

#pragma warning ( disable : 4996 )

#define VALID_OFFSET(offset, len) \
    ((offset) < GetSize() && \
     ((len) == 0 || (offset) + (len) <= GetSize()))

#undef PAGE_SIZE
#define PAGE_SIZE 4096
#undef PAGE_MASK
#define PAGE_MASK 4095

#ifndef PT_ARM_EXIDX
#define PT_ARM_EXIDX    0x70000001      /* .ARM.exidx segment */
#endif

#define FLAG_LINKED     0x00000001
#define FLAG_ERROR      0x00000002
#define FLAG_EXE        0x00000004 // The main executable

#ifndef SHT_INIT_ARRAY
#define SHT_INIT_ARRAY	  14		/* Array of constructors */
#endif

#ifndef SHT_FINI_ARRAY
#define SHT_FINI_ARRAY	  15		/* Array of destructors */
#endif

#ifndef SHT_PREINIT_ARRAY
#define SHT_PREINIT_ARRAY 16		/* Array of pre-constructors */
#endif

#ifndef DT_INIT_ARRAY
#define DT_INIT_ARRAY      25
#endif

#ifndef DT_FINI_ARRAY
#define DT_FINI_ARRAY      26
#endif

#ifndef DT_INIT_ARRAYSZ
#define DT_INIT_ARRAYSZ    27
#endif

#ifndef DT_FINI_ARRAYSZ
#define DT_FINI_ARRAYSZ    28
#endif

#ifndef DT_PREINIT_ARRAY
#define DT_PREINIT_ARRAY   32
#endif

#ifndef DT_PREINIT_ARRAYSZ
#define DT_PREINIT_ARRAYSZ 33
#endif

struct Elf_Shstr
{
    const char *m_szName;
    Elf_Word    m_nOffset;
};

#define SECTION_INFO(section) \
    section(SECTION_EMPTY, "") \
    section(SECTION_INTERP, ".interp") \
    section(SECTION_DYNSYM, ".dynsym") \
    section(SECTION_DYNSTR, ".dynstr") \
    section(SECTION_HASH, ".hash") \
    section(SECTION_REL_DYN, ".rel.dyn") \
    section(SECTION_REL_PLT, ".rel.plt") \
    section(SECTION_PLT, ".plt") \
    section(SECTION_TEXT, ".text") \
    section(SECTION_FINI_ARRAY, ".fini_array") \
    section(SECTION_INIT_ARRAY, ".init_array") \
    section(SECTION_PREINIT_ARRAY, ".preinit_array") \
    section(SECTION_DYNMAIC, ".dynmaic") \
    section(SECTION_GOT, ".got") \
    section(SECTION_DATA, ".data") \
    section(SECTION_BSS, ".bss") \
    section(SECTION_SHSTRTAB, ".shstrtab") \
    section(SECTION_MAX_COUNT, "")

#define _SECTION_INDEX(idx, name) idx,
#define _SECTION_NAME(idx, name) { name },

enum { SECTION_INFO(_SECTION_INDEX) };

static Elf_Shstr *InitShstrs()
{
    static Elf_Shstr aryShstrs[] = { SECTION_INFO(_SECTION_NAME) };

    Elf_Word nOffset = 0;
    for (int i = 0; i <= SECTION_MAX_COUNT; i++)
    {
        aryShstrs[i].m_nOffset = nOffset;
        nOffset += strlen(aryShstrs[i].m_szName) + 1;
    }

    return aryShstrs;
}

static Elf_Shstr *g_aryShstrs = InitShstrs();

CElfFile::CElfFile()
    : m_elfHdr(this)
    , m_elfPhdr(this)
    , m_elfShdr(this)
{
}


CElfFile::~CElfFile()
{
}

bool CElfFile::IsValid()
{
    assert(IsOpen());

    elfhdr hdr = { 0 };

    //读Dos头
    size_t nRet = CFileUtil::Read(0, &hdr, sizeof(elfhdr));

    if (nRet == 0 || !IS_ELF(hdr))
    {
        return false;
    }

    return true;
}

//读取文件数据并验证格式
bool CElfFile::ParseFile()
{
    bool bRet = false;

    if ((IsOpen() || Open()) && Read() && ParseHeader() && ParseElfInfo())
    {
        bRet = true;
        Close();
    }

    return bRet;
}


bool CElfFile::ParseHeader()
{
    assert(!m_Buffer.IsEmpty());

    // elf头
    elfhdr *pHdr = (elfhdr*)m_Buffer.GetBuffer(0);

    if (GetSize() < sizeof(elfhdr) || !IS_ELF(*pHdr))
    {
        return false;
    }

    m_elfHdr.SetOffset((size_t)0);

    // 程序头
    if (m_elfHdr->e_phoff != 0)
    {
        if (VALID_OFFSET(m_elfHdr->e_phoff, pHdr->e_phentsize * pHdr->e_phnum))
        {
            m_elfPhdr.SetOffset(pHdr->e_phoff);
            m_elfPhdr.SetCount(pHdr->e_phnum);
        }
    }
    
    // 节头
    if (m_elfHdr->e_shoff != 0)
    {
        if (VALID_OFFSET(m_elfHdr->e_shoff, pHdr->e_shentsize * pHdr->e_shnum))
        {
            m_elfShdr.SetOffset(pHdr->e_shoff);
            m_elfShdr.SetCount(pHdr->e_shnum);
        }
    }

    return m_elfPhdr != NULL;
}


bool CElfFile::ParseElfInfo()
{
    Elf_Word *d;
    Elf_Phdr *phdr = m_elfPhdr;
    int phnum = m_elfPhdr.GetCount();

    memset(&m_elfInfo, 0, sizeof(m_elfInfo));

    m_elfInfo.dynamic = (Elf_Word)-1;
    m_elfInfo.wrprotect_start = (Elf_Word)-1;
    m_elfInfo.wrprotect_end = 0;

    for (; phnum > 0; --phnum, ++phdr) {
        if (phdr->p_type == PT_LOAD) {
            /* For the executable, we use the m_elfInfo.size field only in
            dl_unwind_find_exidx(), so the meaning of m_elfInfo.size
            is not the size of the executable; it is the last
            virtual address of the loadable part of the executable;
            since m_elfInfo.base == 0 for an executable, we use the
            range [0, m_elfInfo.size) to determine whether a PC value
            falls within the executable section.  Of course, if
            a value is below phdr->p_vaddr, it's not in the
            executable section, but a) we shouldn't be asking for
            such a value anyway, and b) if we have to provide
            an EXIDX for such a value, then the executable's
            EXIDX is probably the better choice.
            */
            if (phdr->p_vaddr + phdr->p_memsz > m_elfInfo.size)
                m_elfInfo.size = phdr->p_vaddr + phdr->p_memsz;
            /* try to remember what range of addresses should be write
            * protected */
            if (!(phdr->p_flags & PF_W)) {
                Elf_Word _end;

                if (phdr->p_vaddr < m_elfInfo.wrprotect_start)
                    m_elfInfo.wrprotect_start = phdr->p_vaddr;
                _end = (((phdr->p_vaddr + phdr->p_memsz + PAGE_SIZE - 1) & (~PAGE_MASK)));
                if (_end > m_elfInfo.wrprotect_end)
                    m_elfInfo.wrprotect_end = _end;
            }
            if (m_elfInfo.linksz < phdr->p_offset + phdr->p_filesz) {
                m_elfInfo.linksz = phdr->p_offset + phdr->p_filesz;
            }
        }
        else if (phdr->p_type == PT_DYNAMIC) {
            if (m_elfInfo.dynamic != (Elf_Word)-1) {
                goto fail;
            }
            m_elfInfo.dynamic = phdr->p_vaddr;
            m_elfInfo.dynamic_size = phdr->p_memsz;
        }
        else if (phdr->p_type == PT_INTERP) {
            m_elfInfo.interp = phdr->p_vaddr;
            m_elfInfo.interp_size = phdr->p_memsz;
        }
        else if (phdr->p_type == PT_ARM_EXIDX) {
            /* exidx entries (used for stack unwinding) are 8 bytes each.
            */
            m_elfInfo.ARM_exidx = (Elf_Word)phdr->p_vaddr;
            m_elfInfo.ARM_exidx_count = phdr->p_memsz / 8;
        }
    }

    if (m_elfInfo.dynamic == (Elf_Word)-1) {
        goto fail;
    }

    /* extract useful information from dynamic section */
    for (d = (Elf_Word*)GetBufferByRva(m_elfInfo.dynamic); *d; d++) {
        switch (*d++) {
        case DT_HASH:
            m_elfInfo.hashtab = *d;
            m_elfInfo.nbucket = ((Elf_Word *)GetBufferByRva(*d))[0];
            m_elfInfo.nchain = ((Elf_Word *)GetBufferByRva(*d))[1];
            m_elfInfo.bucket = (Elf_Word)(*d + 8);
            m_elfInfo.chain = (Elf_Word)(*d + 8 + m_elfInfo.nbucket * 4);
            break;
        case DT_STRTAB:
            m_elfInfo.strtab = (Elf_Word)(*d);
            break;
        case DT_SYMTAB:
            m_elfInfo.symtab = (Elf_Word)(*d);
            break;
#if !defined(ANDROID_SH_LINKER)
        case DT_PLTREL:
            if (*d != DT_REL) {
                goto fail;
            }
            break;
#endif
#ifdef ANDROID_SH_LINKER
        case DT_JMPREL:
            m_elfInfo.plt_rela = (Elf_Word)(*d);
            break;
        case DT_PLTRELSZ:
            m_elfInfo.plt_rela_count = *d / sizeof(Elf_Rela);
            break;
#else
        case DT_JMPREL:
            m_elfInfo.plt_rel = (Elf_Word)(*d);
            break;
        case DT_PLTRELSZ:
            m_elfInfo.plt_rel_count = *d / 8;
            break;
#endif
        case DT_REL:
            m_elfInfo.rel = (Elf_Word)(*d);
            break;
        case DT_RELSZ:
            m_elfInfo.rel_count = *d / 8;
            break;
#ifdef ANDROID_SH_LINKER
        case DT_RELASZ:
            m_elfInfo.rela_count = *d / sizeof(Elf_Rela);
            break;
#endif
        case DT_PLTGOT:
            /* Save this in case we decide to do lazy binding. We don't yet. */
            m_elfInfo.plt_got = (Elf_Word)(*d);
            break;
//        case DT_DEBUG:
//            // Set the DT_DEBUG entry to the addres of _r_debug for GDB
//            *d = (int) &_r_debug;
//            break;
#ifdef ANDROID_SH_LINKER
        case DT_RELA:
            m_elfInfo.rela = (Elf_Rela *)(*d);
            break;
#else
        case DT_RELA:
            goto fail;
#endif
        case DT_INIT:
            m_elfInfo.init_func = *d;
            break;
        case DT_FINI:
            m_elfInfo.fini_func = *d;
            break;
        case DT_INIT_ARRAY:
            m_elfInfo.init_array = *d;
            break;
        case DT_INIT_ARRAYSZ:
            m_elfInfo.init_array_count = ((Elf_Word)*d) / sizeof(Elf_Addr);
            break;
        case DT_FINI_ARRAY:
            m_elfInfo.fini_array = *d;
            break;
        case DT_FINI_ARRAYSZ:
            m_elfInfo.fini_array_count = ((Elf_Word)*d) / sizeof(Elf_Addr);
            break;
        case DT_PREINIT_ARRAY:
            m_elfInfo.preinit_array = *d;
            break;
        case DT_PREINIT_ARRAYSZ:
            m_elfInfo.preinit_array_count = ((Elf_Word)*d) / sizeof(Elf_Addr);
            break;
        case DT_TEXTREL:
            /* TODO: make use of this. */
            /* this means that we might have to write into where the text
            * segment was loaded during relocation... Do something with
            * it.
            */
            break;
        }
    }

    return true;

fail:
    return false;
}


Elf_Word CElfFile::GetNextSection(Elf_Word nOffset)
{
#define SET_NEXT_OFFSET(rva) \
    do { \
        Elf_Word tmp = RvaToOffset(rva);\
        if (tmp > nOffset && tmp < nNext) \
            nNext = tmp; \
    } while (false)

    Elf_Word nNext = m_elfInfo.linksz;

    SET_NEXT_OFFSET(m_elfInfo.dynamic);
    SET_NEXT_OFFSET(m_elfInfo.interp);
    SET_NEXT_OFFSET(m_elfInfo.strtab);
    SET_NEXT_OFFSET(m_elfInfo.symtab);
    SET_NEXT_OFFSET(m_elfInfo.hashtab);
    SET_NEXT_OFFSET(m_elfInfo.plt_got);
    SET_NEXT_OFFSET(m_elfInfo.plt_rel);
    SET_NEXT_OFFSET(m_elfInfo.rel);
    SET_NEXT_OFFSET(m_elfInfo.plt_rela);
    SET_NEXT_OFFSET(m_elfInfo.rela);
    SET_NEXT_OFFSET(m_elfInfo.preinit_array);
    SET_NEXT_OFFSET(m_elfInfo.init_array);
    SET_NEXT_OFFSET(m_elfInfo.fini_array);

    return nNext;
}

int CElfFile::RepairInterp()
{
    int nIndex = m_elfShdr.GetCount();
    Elf_Shdr *pShdr = &m_elfShdr[nIndex];

    pShdr->sh_name = g_aryShstrs[SECTION_INTERP].m_nOffset;
    pShdr->sh_type = SHT_PROGBITS;
    pShdr->sh_flags = SHF_ALLOC;
    pShdr->sh_addr = m_elfInfo.interp;
    pShdr->sh_offset = RvaToOffset(pShdr->sh_addr);
    pShdr->sh_size = m_elfInfo.interp_size;
    pShdr->sh_link = 0;
    pShdr->sh_info = 0;
    pShdr->sh_addralign = 1;
    pShdr->sh_entsize = 0;

    return nIndex;
}

int CElfFile::RepairDynsym(int nDynstr)
{
    int nIndex = m_elfShdr.GetCount();
    Elf_Shdr *pShdr = &m_elfShdr[nIndex];

    pShdr->sh_name = g_aryShstrs[SECTION_DYNSYM].m_nOffset;
    pShdr->sh_type = SHT_DYNSYM;
    pShdr->sh_flags = SHF_ALLOC;
    pShdr->sh_addr = m_elfInfo.symtab;
    pShdr->sh_offset = RvaToOffset(pShdr->sh_addr);
    pShdr->sh_size = m_elfInfo.strtab - m_elfInfo.symtab;
    pShdr->sh_link = nDynstr;
    pShdr->sh_info = 0;
    pShdr->sh_addralign = 4;
    pShdr->sh_entsize = sizeof(Elf_Sym);

    return nIndex;
}

int CElfFile::RepairDynstr()
{
    int nIndex = m_elfShdr.GetCount();
    Elf_Shdr *pShdr = &m_elfShdr[nIndex];

    pShdr->sh_name = g_aryShstrs[SECTION_DYNSTR].m_nOffset;
    pShdr->sh_type = SHT_STRTAB;
    pShdr->sh_flags = SHF_ALLOC;
    pShdr->sh_addr = m_elfInfo.strtab;
    pShdr->sh_offset = RvaToOffset(pShdr->sh_addr);
    pShdr->sh_size = m_elfInfo.hashtab - m_elfInfo.strtab;
    pShdr->sh_link = 0;
    pShdr->sh_info = 0;
    pShdr->sh_addralign = 1;
    pShdr->sh_entsize = 0;

    return nIndex;
}

int CElfFile::RepairHash()
{
    int nIndex = m_elfShdr.GetCount();
    Elf_Shdr *pShdr = &m_elfShdr[nIndex];

    pShdr->sh_name = g_aryShstrs[SECTION_HASH].m_nOffset;
    pShdr->sh_type = SHT_HASH;
    pShdr->sh_flags = SHF_ALLOC;
    pShdr->sh_addr = m_elfInfo.hashtab;
    pShdr->sh_offset = RvaToOffset(pShdr->sh_addr);
    pShdr->sh_size = (m_elfInfo.nbucket + m_elfInfo.nchain) * 4 + 8;
    pShdr->sh_link = 0;
    pShdr->sh_info = 0;
    pShdr->sh_addralign = 1;
    pShdr->sh_entsize = sizeof(Elf_Word);

    return nIndex;
}

int CElfFile::RepairRelDyn(int nDynsym)
{
    int nIndex = m_elfShdr.GetCount();
    Elf_Shdr *pShdr = &m_elfShdr[nIndex];

    pShdr->sh_name = g_aryShstrs[SECTION_REL_DYN].m_nOffset;
    pShdr->sh_type = SHT_REL;
    pShdr->sh_flags = SHF_ALLOC;
    pShdr->sh_addr = m_elfInfo.rel;
    pShdr->sh_offset = RvaToOffset(pShdr->sh_addr);
    pShdr->sh_size = m_elfInfo.rel_count * sizeof(Elf_Rel);
    pShdr->sh_link = nDynsym;
    pShdr->sh_info = 0;
    pShdr->sh_addralign = 4;
    pShdr->sh_entsize = sizeof(Elf_Rel);

    return nIndex;
}

int CElfFile::RepairRelPlt(int nDynsym)
{
    int nIndex = m_elfShdr.GetCount();
    Elf_Shdr *pShdr = &m_elfShdr[nIndex];

    pShdr->sh_name = g_aryShstrs[SECTION_REL_PLT].m_nOffset;
    pShdr->sh_type = SHT_REL;
    pShdr->sh_flags = SHF_ALLOC;
    pShdr->sh_addr = m_elfInfo.plt_rel;
    pShdr->sh_offset = RvaToOffset(pShdr->sh_addr);
    pShdr->sh_size = m_elfInfo.plt_rel_count * sizeof(Elf_Rel);
    pShdr->sh_link = nDynsym;
    pShdr->sh_info = 0;
    pShdr->sh_addralign = 4;
    pShdr->sh_entsize = sizeof(Elf_Rel);

    return nIndex;
}

int CElfFile::RepairText()
{
    int nIndex = m_elfShdr.GetCount();
    Elf_Shdr *pShdr = &m_elfShdr[nIndex];

    pShdr->sh_name = g_aryShstrs[SECTION_TEXT].m_nOffset;
    pShdr->sh_type = SHT_PROGBITS;
    pShdr->sh_flags = SHF_ALLOC;
    pShdr->sh_addr = m_elfInfo.plt_rel + m_elfInfo.plt_rel_count * sizeof(Elf_Rel);
    pShdr->sh_offset = RvaToOffset(pShdr->sh_addr);
    pShdr->sh_size = GetNextSection(pShdr->sh_offset) - pShdr->sh_offset;
    pShdr->sh_link = 0;
    pShdr->sh_info = 0;
    pShdr->sh_addralign = 1;
    pShdr->sh_entsize = 0;

    return nIndex;
}

int CElfFile::RepairInitArray()
{
    int nIndex = m_elfShdr.GetCount();
    Elf_Shdr *pShdr = &m_elfShdr[nIndex];

    pShdr->sh_name = g_aryShstrs[SECTION_INIT_ARRAY].m_nOffset;
    pShdr->sh_type = SHT_INIT_ARRAY;
    pShdr->sh_flags = SHF_ALLOC;
    pShdr->sh_addr = m_elfInfo.init_array;
    pShdr->sh_offset = RvaToOffset(pShdr->sh_addr);
    pShdr->sh_size = m_elfInfo.init_array_count * sizeof(Elf_Addr);
    pShdr->sh_link = 0;
    pShdr->sh_info = 0;
    pShdr->sh_addralign = 4;
    pShdr->sh_entsize = 0;

    return nIndex;
}

int CElfFile::RepairFiniArray()
{
    int nIndex = m_elfShdr.GetCount();
    Elf_Shdr *pShdr = &m_elfShdr[nIndex];

    pShdr->sh_name = g_aryShstrs[SECTION_FINI_ARRAY].m_nOffset;
    pShdr->sh_type = SHT_FINI_ARRAY;
    pShdr->sh_flags = SHF_ALLOC;
    pShdr->sh_addr = m_elfInfo.fini_array;
    pShdr->sh_offset = RvaToOffset(pShdr->sh_addr);
    pShdr->sh_size = m_elfInfo.fini_array_count * sizeof(Elf_Addr);
    pShdr->sh_link = 0;
    pShdr->sh_info = 0;
    pShdr->sh_addralign = 4;
    pShdr->sh_entsize = 0;

    return nIndex;
}

int CElfFile::RepairPreinitArray()
{
    int nIndex = m_elfShdr.GetCount();
    Elf_Shdr *pShdr = &m_elfShdr[nIndex];

    pShdr->sh_name = g_aryShstrs[SECTION_PREINIT_ARRAY].m_nOffset;;
    pShdr->sh_type = SHT_INIT_ARRAY;
    pShdr->sh_flags = SHF_ALLOC;
    pShdr->sh_addr = m_elfInfo.preinit_array;
    pShdr->sh_offset = RvaToOffset(pShdr->sh_addr);
    pShdr->sh_size = m_elfInfo.preinit_array_count * sizeof(Elf_Addr);
    pShdr->sh_link = 0;
    pShdr->sh_info = 0;
    pShdr->sh_addralign = 4;
    pShdr->sh_entsize = 0;

    return nIndex;
}

int CElfFile::RepairDynamic()
{
    int nIndex = m_elfShdr.GetCount();
    Elf_Shdr *pShdr = &m_elfShdr[nIndex];

    pShdr->sh_name = g_aryShstrs[SECTION_DYNMAIC].m_nOffset;;
    pShdr->sh_type = SHT_DYNAMIC;
    pShdr->sh_flags = SHF_ALLOC;
    pShdr->sh_addr = m_elfInfo.dynamic;
    pShdr->sh_offset = RvaToOffset(pShdr->sh_addr);
    pShdr->sh_size = m_elfInfo.dynamic_size;
    pShdr->sh_link = 0;
    pShdr->sh_info = 0;
    pShdr->sh_addralign = 4;
    pShdr->sh_entsize = sizeof(Elf_Dyn);

    return nIndex;
}

int CElfFile::RepairGot()
{
    if (m_elfInfo.plt_got != 0)
    {
        int nIndex = m_elfShdr.GetCount();
        Elf_Shdr *pShdr = &m_elfShdr[nIndex];

        pShdr->sh_name = g_aryShstrs[SECTION_GOT].m_nOffset;;
        pShdr->sh_type = SHT_PROGBITS;
        pShdr->sh_flags = SHF_ALLOC;
        pShdr->sh_addr = m_elfInfo.plt_got - m_elfInfo.rel_count * sizeof(Elf_Word);
        pShdr->sh_offset = RvaToOffset(pShdr->sh_addr);
        pShdr->sh_size = GetNextSection(RvaToOffset(m_elfInfo.plt_got)) - pShdr->sh_offset;
        pShdr->sh_link = 0;
        pShdr->sh_info = 0;
        pShdr->sh_addralign = 4;
        pShdr->sh_entsize = 0;

        return nIndex;
    }

    return 0;
}

int CElfFile::RepairBss()
{
    int nIndex = m_elfShdr.GetCount();
    Elf_Shdr *pShdr = &m_elfShdr[nIndex];

    pShdr->sh_name = g_aryShstrs[SECTION_BSS].m_nOffset;;
    pShdr->sh_type = SHT_NOBITS;
    pShdr->sh_flags = SHF_ALLOC;
    pShdr->sh_addr = OffsetToRva(m_elfInfo.linksz - 1) + 1;
    pShdr->sh_offset = m_elfInfo.linksz;
    pShdr->sh_size = m_elfInfo.size - pShdr->sh_addr;
    pShdr->sh_link = 0;
    pShdr->sh_info = 0;
    pShdr->sh_addralign = 1;
    pShdr->sh_entsize = 0;

    return nIndex;
}

int CElfFile::RepairShstrtab(Elf_Word nOffset)
{
    int nIndex = m_elfShdr.GetCount();
    Elf_Shdr *pShdr = &m_elfShdr[nIndex];

    pShdr->sh_name = g_aryShstrs[SECTION_SHSTRTAB].m_nOffset;;
    pShdr->sh_type = SHT_STRTAB;
    pShdr->sh_flags = SHF_ALLOC;
    pShdr->sh_addr = 0;
    pShdr->sh_offset = nOffset;
    pShdr->sh_size = g_aryShstrs[SECTION_MAX_COUNT].m_nOffset;
    pShdr->sh_link = 0;
    pShdr->sh_info = 0;
    pShdr->sh_addralign = 1;
    pShdr->sh_entsize = 0;

    char *pBuf = (char*)SetBuffer(nOffset, pShdr->sh_size);
    for (int i = 0; i < SECTION_MAX_COUNT; i++)
    {
        strcpy(pBuf + g_aryShstrs[i].m_nOffset, g_aryShstrs[i].m_szName);
    }

    return nIndex;
}

bool CElfFile::RepairFile()
{
    size_t nOffset = (m_elfInfo.linksz + PAGE_MASK) & ~PAGE_MASK;

    m_elfShdr.SetOffset(nOffset + 0x200);
    m_elfShdr.SetCount(0);

    // 填节表
    memset(&m_elfShdr[0], 0, sizeof(m_elfShdr[0]));
    int nInterp = RepairInterp();
    int nDynstr = RepairDynstr();
    int nDynsym = RepairDynsym(nDynstr);
    int nHash = RepairHash();
    int nRelDyn = RepairRelDyn(nDynsym);
    int nRelPlt = RepairRelPlt(nDynsym);
    int nText = RepairText();
    int nInitArray = RepairInitArray();
    int nFiniArray = RepairFiniArray();
    int nPreinitArray = RepairPreinitArray();
    int nDynamic = RepairDynamic();
    int nGot = RepairGot();
    int nBss = RepairBss();
    int nShstrtab = RepairShstrtab(nOffset);

    // 修复节表头
    m_elfHdr->e_shstrndx = nShstrtab;
    m_elfHdr->e_shoff = m_elfShdr.GetOffset();
    m_elfHdr->e_shnum = m_elfShdr.GetCount();
    m_elfHdr->e_shentsize = sizeof(m_elfShdr[0]);
    
    // 修复程序头
    for (int i = 0; i < m_elfPhdr.GetCount(); i++)
    {
        if (m_elfPhdr[i].p_type == PT_DYNAMIC)
        {
            m_elfPhdr[i].p_offset = RvaToOffset(m_elfPhdr[i].p_vaddr);
        }
    }

    return true;
}


static unsigned elfhash(const char *_name)
{
    const unsigned char *name = (const unsigned char *)_name;
    unsigned h = 0, g;

    while (*name) {
        h = (h << 4) + *name++;
        g = h & 0xf0000000;
        h ^= g;
        h ^= g >> 24;
    }
    return h;
}


//获取相对虚拟地址
size_t CElfFile::OffsetToRva(size_t nOffset)
{
    for (int i = 0; i < m_elfPhdr.GetCount(); i++)
    {
        Elf_Phdr *pPhdr = &m_elfPhdr[i];

        if (pPhdr->p_type == PT_LOAD)
        {
            if (pPhdr->p_offset <= nOffset &&
                pPhdr->p_offset + pPhdr->p_filesz > nOffset)
            {
                return nOffset - pPhdr->p_offset + pPhdr->p_paddr;
            }
        }
    }

    return -1;
}


//获取文件偏移
size_t CElfFile::RvaToOffset(size_t nRva)
{
    for (int i = 0; i < m_elfPhdr.GetCount(); i++)
    {
        Elf_Phdr *pPhdr = &m_elfPhdr[i];

        if (pPhdr->p_type == PT_LOAD)
        {
            if (pPhdr->p_paddr <= nRva &&
                pPhdr->p_paddr + pPhdr->p_memsz > nRva)
            {
                return nRva - pPhdr->p_paddr + pPhdr->p_offset;
            }
        }
    }

    return -1;
}


//获取可读缓冲区，nLen为0不指定长度
void *CElfFile::GetBufferByRva(size_t nRva, size_t nLen)
{
    return GetBuffer(RvaToOffset(nRva), nLen);
}


//获取可写缓冲区
void *CElfFile::SetBufferByRva(size_t nRva, size_t nLen)
{
    return SetBuffer(RvaToOffset(nRva), nLen);
}