/* Multiboot2 header constants */
.set MAGIC,    0xE85250D6
.set ARCH,     0                                    /* 32 bit protected mode */
.set LENGTH,   40                                   /* Header + Tag length */
.set CHECKSUM, -(MAGIC+ARCH+LENGTH)


/* 64 bit aligned, load multiboot2 header values, each 32 bit in order */ 

.section multiboot2
.align 8
.long MAGIC
.long ARCH
.long LENGTH
.long CHECKSUM

/* Multiboot2 Tags must be 8 byte aligned, terminated with terminating tag */

/* 1. Multiboot information request tag */
.align 8
.short 1    /* Type Specifier */
.short 0    /* Not optional (bit 0 != 1) */
.long 16    /* Size in bytes of tag */
/* mbi_tag_types */
.long 2     /* Bootloader name */
.long 6     /* Memory Map */

/* 2. Terminating tag */
.align 8
.short 0
.short 0
.long 8


/* ----------------------------------- */

.section bss
.align 16
stack_bottom:
.skip 16384
stack_top:

/* ----------------------------------- */

.code32
.section .text
.global _start
.type _start, @function
_start:
    /* Set stack to the allocated region at runtime */
    mov $stack_top, %esp

    call boot_main

    /* Interrupts are already disabled by bootloader,
       HALT locks up the computer */
1:  hlt
    jmp 1b
