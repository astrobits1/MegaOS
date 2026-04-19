.section .text
.code32

/* Routine to enable long mode paging
   Should be called after setting page maps */

/* Enables 64 bit paging by entering x86_64 compatibility mode
   preparing for long mode */
/* void enable_paging64() */ 
.global enable_paging64
.type enable_paging64, @function
enable_paging64:
    push %eax 

    /* Set PAE flag in CR4 for long mode paging */
    .equ bit5, 1<<5
    mov %cr4, %eax
    or $bit5, %eax
    mov %eax, %cr4

    /* Set LME bit in EFER */
    .equ EFER, 0xC0000080
    .equ bit8, 1<<8
    mov $EFER, %ecx
    rdmsr
    or $bit8, %eax
    wrmsr

    /* Set Paging and Protected mode flag in CR0 */
    .equ bit31, 1<<31
    .equ bit0, 1<<0
    mov %cr0, %eax
    or $bit31, %eax
    or $bit0, %eax
    mov %eax, %cr0

    /* Compatibility mode enabled */
    pop %eax
    ret


/* void load_pml4(void* pml4) */
.global load_pml4
.type load_pml4, @function
load_pml4:
    /* PCID not handled and PCIDE assumed 0 (disabled) */
    push %ebp
    mov %esp, %ebp
    push %eax

    mov 8(%ebp), %eax
    /* Lower 12 bits must be 0 for page alignment, flags are overwritten on them
       PWT and PCT for top level page map (PML4) are disabled (0) */
    and $~0xFFF, %eax
    mov %eax, %cr3

    pop %eax
    pop %ebp
    ret
