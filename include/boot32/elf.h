#ifndef BOOT32_ELF_H
#define BOOT32_ELF_H

#include <stdint.h>

typedef uint64_t elft_addr;
typedef uint64_t elft_off;
typedef uint16_t elft_half;
typedef uint32_t elft_word;
typedef int32_t  elft_sword;
typedef uint64_t elft_xword;
typedef int64_t  elft_sxword;
typedef unsigned char elft_char;

/* ELF header */

struct elf_ehdr {
    elft_char e_ident[16];      /* ELF identification */
    elft_half e_type;           /* Object file type */
    elft_half e_machine;        /* Machine type */
    elft_word e_version;        /* Object file version */
    elft_addr e_entry;          /* Entry point address */
    elft_off e_phoff;           /* Program header offset */
    elft_off e_shoff;           /* Section header offset */
    elft_word e_flags;          /* Processor-specific flags */
    elft_half e_ehsize;         /* ELF header size */
    elft_half e_phentsize;      /* Size of program header entry */
    elft_half e_phnum;          /* Number of program header entries */
    elft_half e_shentsize;      /* Size of section header entry */
    elft_half e_shnum;          /* Number of section header entries */
    elft_half e_shstrndx;       /* Section name string table index */
};

/* Program Header Table Entry */

struct elf_phdr {
    elft_word p_type;       /* Type of segment */
    elft_word p_flags;      /* Segment attributes */
    elft_off p_offset;      /* Offset in file */
    elft_addr p_vaddr;      /* Virtual address in memory */
    elft_addr p_paddr;      /* Reserved */
    elft_xword p_filesz;    /* Size of segment in file */
    elft_xword p_memsz;     /* Size of segment in memory */
    elft_xword p_align;     /* Alignment of segment */
};

/* ------------------------------------------------ */

struct elf_metadata {
    uint64_t entrypoint;               /* Entry point as virtual address to jump to
                                       start execution */
    uint64_t virtual_start;            /* Virtual pointer start of the ELF */
    uint32_t memory_image_size;        /* Total size spanning in memory starting from virtual_start */
};


struct elf_metadata load_elf64_exec_at(uint8_t* data, uint32_t size, uint8_t* address);

#endif
