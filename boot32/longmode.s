.section .data
jmp_addr:
.long 0                 /* Address low */
.long 0                 /* Address high */
.short 0x08             /* CS */

.section .rodata
jmp64:
    /* ebx has address high, eax has address low */

    // mov %ebx, %ecx
    .byte 0x89, 0xd9
    // shl $32, %rcx
    .byte 0x48, 0xC1, 0xE1, 0x20
    // or %rax, %rcx
    .byte 0x48, 0x09, 0xC1
    // jmp *%rcx
    .byte 0xFF, 0xE1


.section .text
.code32

.align 8
/* void jump_kernel(uint32_t entry_lo, uint32_t entry_hi, struct bootinfo* info) */
.global jump_kernel
.type jump_kernel, @function
jump_kernel:
    push %ebp
    mov %esp, %ebp

    /* Load entrypoint pointer to jmp_addr */
    /* Move entrypoint low *//
    mov 8(%ebp), %eax

    /* Move entrypoint high */
    mov 12(%ebp), %ebx

    /* Load bootinfo pointer at the top of the stack, this is our data to be passed */
    mov 16(%ebp), %ecx 

    /* Push bootinfo, stack must be 8 byte aligned for 64 bit */
    push $0
    push %ecx
    /* Indirect long jump, enters kernel64 and enables long mode */
    ljmp $0x08, $jmp64
    /* noreturn */

    pop %ebp
    ret
