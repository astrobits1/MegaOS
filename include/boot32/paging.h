#ifndef BOOT32_PAGING_H
#define BOOT32_PAGING_H

void paging_initialise();

/* paging_as.s */
void enable_paging64();
void load_pml4(volatile void* pml4);

#endif
