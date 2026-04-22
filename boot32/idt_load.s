.section .data
.align 8
idtr:
    .short 0            /* Size (16 bit) */
    .long 0             /* Offset (32 bit) */


.section .text
.code32

/* void idtr_load(void* offset, uint16_t size) */
.global idtr_load
.type idtr_load, @function
idtr_load:
    push %ebp
    mov %esp, %ebp
    push %eax

    /* Load size */
    movw 12(%ebp),   %ax
    movw %ax,        idtr

    /* Load offset */
    movl 8(%ebp),    %eax
    movl %eax,       idtr+2

    /* Write to GDTR */ 
    lidt idtr

    pop %eax
    pop %ebp
    ret
