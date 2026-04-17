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
    push %eax

    /* Load size */
    movw 8(%esp),    %ax
    movw %ax,        gdtr

    /* Load offset */
    movl 4(%esp),    %eax
    movl %eax,       gdtr+2

    /* Write to GDTR */ 
    lgdt gdtr

    pop %eax
    ret
    
