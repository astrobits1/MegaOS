ENTRY(_entry)

SECTIONS {
    /* Bootstrapper must load kernel to -2GB virtual address */
    _KERNEL_BEGIN = 0xFFFFFFFF80000000;

    /* code */
    .text ALIGN(4K) : {
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

    _KERNEL_END = .;
}
