
.section .text
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
    /* Indirect long jump, enables long mode and enters 64 bit code block that
    jumps to virtual address with high in ebx and low in eax.
    This is done because we cannot jump to a higher half address directly in 32 bit mode */
    ljmp $0x08, $jmp64
    /* noreturn */

    pop %ebp
    ret


/* void enable_sse() */
.global enable_sse
.type enable_sse, @function
enable_sse:
    push %eax

    mov %cr0, %eax
    /* Clear CR0.EM */
    .equ bit2, 1<<2
    and $~bit2, %eax

    /* Set CR0.MP */
    or $1, %eax
    mov %eax, %cr0

    mov %cr4, %eax
    /* Set CR4.OSFXSR, CR4.OSXMMEXCPT */
    .equ mask, 0b11<<9
    or $mask, %eax
    mov %eax, %cr4

    pop %eax
    ret
