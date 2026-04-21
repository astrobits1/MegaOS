.section .text
.code64

.global _entry
.type _entry, @function
_entry:
    mov $0xDEADBEEFCAFEBABE, %rax

    /* Bootinfo pointer */
    pop %rbx

    cli
1:
    hlt
    jmp 1b
