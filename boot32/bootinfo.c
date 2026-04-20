#include <boot32/bootinfo.h>
#include <boot32/vga.h>


struct bootinfo parse_multiboot2_info(void* multiboot2_info) {
    uint32_t addr = (uint32_t)multiboot2_info;
    struct mbi_header* header = (struct mbi_header*)addr;
    uint32_t total_size = header->size;

    uint32_t startaddr = addr;
    addr += sizeof(struct mbi_header);

    /* Fields to read into from tags */
    struct bootinfo info;
    struct blob kernel_elf = {NULL, NULL, 0};

    /* Start reading tags */
    while (addr-startaddr < total_size) {
        struct mbi_tag* tag = (struct mbi_tag*)addr;

        switch (tag->type) {
            case MBI_TAG_MODULE: {
                struct mbi_tag_module* module = (struct mbi_tag_module*)addr;
                
                kernel_elf.start = (void*)module->mod_start;
                kernel_elf.end = (void*)module->mod_end;
                kernel_elf.size = module->mod_end-module->mod_start;

                break;
            }
            default: break;
        }


        addr += tag->size;
        /* Next 8 byte aligned address after parsing tag */
        if ((addr & 0b111) != 0) {
            addr &= ~0b111;
            addr += 0b1000;
        }
    }

    info.kernel_elf = kernel_elf;
    return info;
}

