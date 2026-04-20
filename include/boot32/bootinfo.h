#ifndef BOOT32_BOOTINFO_H
#define BOOT32_BOOTINFO_H

#include <stdint.h>
#include <stddef.h>

enum MBI_TAG_TYPE {
    MBI_TAG_BOOTLOADER_NAME = 2,
    MBI_TAG_MODULE = 3,
    MBI_TAG_MEMORY_MAP = 6
};

struct __attribute__((packed)) mbi_header {
    uint32_t size;
    uint32_t reserved;
};

struct __attribute__((packed)) mbi_tag {
    uint32_t type;
    uint32_t size;
};

struct __attribute__((packed)) mbi_tag_module {
    struct mbi_tag taghead;
    uint32_t mod_start;
    uint32_t mod_end;
    /* string must fit in 100 characters or it will be truncated */
    char id[16];
};

struct blob {
    void* start;
    void* end;
    uint32_t size;
};

struct bootinfo {
    struct blob kernel_elf;
};


struct bootinfo parse_multiboot2_info(void* multiboot2_info);

#endif
