.section .bss

.align 16
stack_bottom:
.skip 16384
stack_top:


.section .text
.code64

.global _entry
.type _entry, @function
_entry:
    mov $0xDEADBEEFCAFEBABE, %rax

    /* Bootinfo pointer */
    pop %rdi

    /* Setup stack */
    mov $stack_top, %rbp
    mov $stack_top, %rsp

    /* Move bootinfo is in 1st arg reg %rdi */ 
    /* Begin kernel main */
    call kernel_main

    cli
1:
    hlt
    jmp 1b
