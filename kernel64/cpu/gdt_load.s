.section .data
.align 16
gdtr:
    .short 0            /* Size (16 bit) */
    .quad 0             /* Offset (64 bit) */


.section .text
.code64

/* void gdtr_load(void* offset, uint16_t size) */
.global gdtr_load
.type gdtr_load, @function
gdtr_load:
    /* Load size */
    movw %si,         gdtr

    /* Load offset */
    movq %rdi,        gdtr+2

    /* Write to GDTR */ 
    lgdt gdtr
    ret


/* void reload_seg_regs() */
.global reload_seg_regs
.type reload_seg_regs, @function
reload_seg_regs:
    push %rbp
    mov %rsp, %rbp
    push %rax

    /* Load data selector on index 2 into ds and the misc segmentation registers */
    movw $0x10, %ax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs
    movw %ax, %ss

    /* Data segment must be loaded before code segment, because our GDT is effectively
       located in our data segment, and it has to be accessible first and formost or CS 
       indexing fails */
    /* Load code selector on index 1 into cs using long return */
    push $0x08
    lea 1f, %rax
    push %rax
    lretq
1:
    pop %rax
    pop %rbp
    ret
