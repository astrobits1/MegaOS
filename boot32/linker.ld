ENTRY(_start)

SECTIONS {
    /* Load ELF at or around physical address 2M 
     * A safe offset that will be available under BIOS and UEFI */
    . = 2M;

    /* Multiboot2 header and code */
    .text ALIGN(4K) : {
        *(.multiboot2)
        *(.text)
    }

    /* Read only data */
    .rodata ALIGN(4K) : {
        *(.rodata)
    }

    /* Read/Write initialised data */
    .data ALIGN(4K) : {
        *(.data)
    }

    /* Uninitialised memory 
     * COMMON = C global uninitialised data
     * .bss   = Stack allocated by boot.s */
    .bss ALIGN(4K) : {
        *(COMMON)
        *(.bss)
    }
}
