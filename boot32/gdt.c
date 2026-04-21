#include <boot32/gdt.h>
#include <boot32/vga.h>

/* Allocate space for 8  segment descriptors */
uint8_t GDT[8*SEG_DESCRIPTOR_ALLOC_COUNT];
uint16_t seg_descriptor_count = 0;

void gdt_write_entry(uint16_t index, uint32_t base, uint32_t limit, uint8_t access, uint8_t flags) {
    /* Writes GDT Descriptor values to given index in the GDT */
    uint16_t offset = index*8;

    GDT[offset] = limit&0xFF;
    GDT[offset+1] = limit>>8 & 0xFF;
    GDT[offset+2] = base&0xFF;
    GDT[offset+3] = base>>8 & 0xFF;
    GDT[offset+4] = base>>16 & 0xFF;
    GDT[offset+5] = access;
    GDT[offset+6] = (limit>>16 & 0xF) | ((flags&0xF) << 4);
    GDT[offset+7] = base>>24 & 0xFF;

    /*
    vga_print("base: 0x");
    vga_print_u32(base, 16, 8);
    vga_print(", limit: 0x");
    vga_print_u32(limit, 16, 5);
    vga_print(", access: 0x");
    vga_print_u32(access, 16, 2);
    vga_print(", flags: 0x");
    vga_print_u32(flags, 16, -1);
    vga_print("\n");
    */
}

void gdt_add_entry(uint32_t base, uint32_t limit, uint8_t access, uint8_t flags) {
    gdt_write_entry(seg_descriptor_count++, base, limit, access, flags);
}

void gdt_initialise() {
    //vga_print_color("Initializing Global Descriptor Table (32 bit)\n", VGA_COLOR_LIGHT_GREEN);

    /* Null descriptor */
    //vga_print("Loading NULL Descriptor at GDT index 0:\n");
    gdt_add_entry(0, 0, 0, 0);

    /* Kernel ring 0 code segment (flat) */
    //vga_print("Loading Kernel Ring 0 Code Segment at GDT index 1:\n");
    gdt_add_entry(0, 0xFFFFF, 0x9A, 0xC); 

    /* Kernel ring 0 data segment (flat) */
    //vga_print("Loading Kernel Ring 0 Data Segment at GDT index 2:\n");
    gdt_add_entry(0, 0xFFFFF, 0x92, 0xC);

    /* Load our GDT to the GDTR using assembly routine */
    //vga_print("Loading GDT to GDTR\n");
    gdtr_load((void*)GDT, seg_descriptor_count*8-1);

    /* Reloads code segment located at index 1 to CS 
     * and data segment at index 2 to DS and the rest
     * This has to be hardcoded because CS cannot be written to from
     * registers */
    //vga_print("Reloading segment registers\n");
    reload_seg_regs();

    vga_print_color("Initialized Global Descriptor Table (32 bit)\n", VGA_COLOR_LIGHT_GREEN);
}

void gdt_set_long_mode() {
    /* Enable long mode bit, disable DB size flag *
     * base and limit are ignored in long mode */
    /* Kernel ring 0 code */
    gdt_write_entry(1, 0, 0xFFFFF, 0x9A, 0xA);
    /* Kernel ring 0 data */
    gdt_write_entry(2, 0, 0xFFFFF, 0x92, 0xA);

    /* GDT is prepared for long mode switch now */
}
