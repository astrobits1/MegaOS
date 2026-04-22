#ifndef COMMON_BOOTINFO_H
#define COMMON_BOOTINFO_H

#include <stdint.h>

/* Kernel specific bootinfo structures */


enum MEMORY_MAP_ENTRY_TYPE {
    MEMORY_MAP_ENTRY_USABLE,
    MEMORY_MAP_ENTRY_USABLE_ACPI,
    MEMORY_MAP_ENTRY_PRESERVE,
    MEMORY_MAP_ENTRY_UNUSABLE
};

struct blob {
    void* start;
    void* end;
    uint32_t size;
};

struct memory_map_entry {
    uint64_t addr;
    uint64_t length;
    enum MEMORY_MAP_ENTRY_TYPE type;
};

struct bootinfo {
    struct blob kernel_elf;

    /* Memory map information */
    uint32_t map_entry_count;
    struct memory_map_entry* map_entries;
};

#endif
