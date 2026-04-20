
.global _entry
.type _entry, @function
_entry:
    call kernel_main

    cli
1:
    hlt
    jmp 1b
