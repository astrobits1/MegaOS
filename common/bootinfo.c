#include <common/bootinfo.h>
#include <common/drivers/vga/vga.h>

void bootinfo_print_memory_map_type(enum MEMORY_MAP_ENTRY_TYPE type) {
    switch (type) {
        case MEMORY_MAP_ENTRY_USABLE:
            vga_print_color("Usable", VGA_COLOR_LIGHT_GREEN);
            break;
        case MEMORY_MAP_ENTRY_USABLE_ACPI:
            vga_print_color("Usable (ACPI)", VGA_COLOR_LIGHT_GREEN);
            break;
        case MEMORY_MAP_ENTRY_UNUSABLE:
            vga_print_color("Unusable", VGA_COLOR_LIGHT_RED);
            break;
        case MEMORY_MAP_ENTRY_PRESERVE:
            vga_print_color("Preserve", VGA_COLOR_LIGHT_RED);
            break;
        default:
            vga_print_color("Unknown", VGA_COLOR_LIGHT_GREY);
            break;
    }
}

void bootinfo_print_memory_map(struct bootinfo* info) {
    vga_print_color("Memory Map\n", VGA_COLOR_LIGHT_GREEN);
    for (uint32_t i=0; i<info->map_entry_count; i++) {
        struct memory_map_entry* map_entries = (void*)(uintptr_t)info->map_entries;
        struct memory_map_entry* entry = &map_entries[i];
        
        vga_print_uint_color(entry->addr, 16, 12, VGA_COLOR_LIGHT_CYAN);
        vga_print("-");
        vga_print_uint_color(entry->addr+entry->length, 16, 12, VGA_COLOR_LIGHT_CYAN);
        vga_print_color("\nSize: ", VGA_COLOR_LIGHT_MAGENTA);
        vga_print_uint(entry->length, 16, 12);
        vga_print_color(",       Type: ", VGA_COLOR_LIGHT_MAGENTA); 
        bootinfo_print_memory_map_type(entry->type); 
        vga_print("\n");
    }
    vga_print("\n");
}
