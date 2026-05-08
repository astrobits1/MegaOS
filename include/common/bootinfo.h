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
    uint64_t start;
    uint64_t end;
    uint32_t size;
} __attribute__((packed));

struct memory_map_entry {
    uint64_t addr;
    uint64_t length;
    enum MEMORY_MAP_ENTRY_TYPE type;
} __attribute__((packed));

struct bootinfo {
    /* Kernel ELF module pointers */
    struct blob kernel_elf;
    /* Kernel physical pointers */
    struct blob kernel_physical;
    /* Memory map information */
    uint32_t map_entry_count;
    /* struct memory_map_entry* */
    uint64_t map_entries;
} __attribute__((packed));


void bootinfo_print_memory_map(struct bootinfo* info);
void bootinfo_print_memory_map_type(enum MEMORY_MAP_ENTRY_TYPE type);

#endif
