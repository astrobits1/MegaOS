.section .data
.align 16
idtr:
    .short 0            /* Size (16 bit) */
    .quad 0             /* Offset (64 bit) */


.section .text
.code64

/* void idtr_load(void* offset, uint16_t size) */
.global idtr_load
.type idtr_load, @function
idtr_load:
    /* Load size */
    movw %si,        idtr

    /* Load offset */
    movq %rdi,       idtr+2

    /* Write to GDTR */ 
    lidt idtr
    ret
