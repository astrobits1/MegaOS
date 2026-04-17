/* Allocating space for gdtr (GDT Descriptor) pointer */
.section .data
.align 8
gdtr:
    .short 0            /* Size (16 bit) */
    .long 0             /* Offset (32 bit) */


.section .text
.code32

/* void gdtr_load(void* offset, uint16_t size) */
.global gdtr_load
.type gdtr_load, @function
gdtr_load:
    push %ebp
    mov %esp, %ebp
    push %eax

    /* Load size */
    movw 12(%ebp),   %ax
    movw %ax,        gdtr

    /* Load offset */
    movl 8(%ebp),    %eax
    movl %eax,       gdtr+2

    /* Write to GDTR */ 
    lgdt gdtr

    pop %eax
    pop %ebp
    ret


/* void reload_seg_regs() */
.global reload_seg_regs
.type reload_seg_regs, @function
reload_seg_regs:
    push %ebp
    mov %esp, %ebp
    push %eax

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
    /* Load code selector on index 1 into cs using jmp */
    ljmp $0x08, $1f
1:
    pop %eax
    pop %ebp
    ret
