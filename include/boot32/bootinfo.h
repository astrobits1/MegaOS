#ifndef BOOT32_BOOTINFO_H
#define BOOT32_BOOTINFO_H

#include <stdint.h>
#include <stddef.h>

enum MBI_TAG_TYPE {
    MBI_TAG_NULL = 0,
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
    char id[64];
};

struct __attribute__((packed)) mbi_tag_memory_map {
    struct mbi_tag taghead;
    uint32_t entry_size;
    uint32_t entry_version;
};

struct __attribute__((packed)) mbi_memory_map_entry {
    uint32_t addr_lo;
    uint32_t addr_hi;
    uint32_t length_lo;
    uint32_t length_hi;
    uint32_t type;
    uint32_t reserved;
};

/* Kernel specific bootinfo structures */

struct blob {
    void* start;
    void* end;
    uint32_t size;
};

enum MEMORY_MAP_ENTRY_TYPE {
    MEMORY_MAP_ENTRY_USABLE,
    MEMORY_MAP_ENTRY_USABLE_ACPI,
    MEMORY_MAP_ENTRY_PRESERVE,
    MEMORY_MAP_ENTRY_UNUSABLE
};

struct memory_map_entry {
    uint32_t addr_lo;
    uint32_t addr_hi;
    uint32_t length_lo;
    uint32_t length_hi;
    enum MEMORY_MAP_ENTRY_TYPE type;
};

struct bootinfo {
    struct blob kernel_elf;

    /* Memory map information */
    uint32_t map_entry_count;
    struct memory_map_entry* map_entries;
};


struct bootinfo parse_multiboot2_info(void* multiboot2_info);

#endif
