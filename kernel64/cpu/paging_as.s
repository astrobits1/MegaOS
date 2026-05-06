.section .text
.code64


/* void load_pml4(void* pml4) */
.global load_pml4
.type load_pml4, @function
load_pml4:
    /* PCID not handled and PCIDE assumed 0 (disabled) */
    /* Lower 12 bits must be 0 for page alignment, flags are overwritten on them
       PWT and PCT for top level page map (PML4) are disabled (0) */
    and $~0xFFF, %rdi
    mov %rdi, %cr3
    ret


