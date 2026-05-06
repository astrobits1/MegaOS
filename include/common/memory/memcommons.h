#ifndef COMMON_MEMCOMMONS_H
#define COMMON_MEMCOMMONS_H

#define PAGE_4K 0x1000
#define PAGE_2M 0x200000
#define PAGE_1G 0x40000000

#define PAGE_4K_ALIGN(x) (((x)&~(PAGE_4K-1))+PAGE_4K)
#define PAGE_4K_ALIGN_DOWN(x) ((x)&~(PAGE_4K-1))

#define CHECK_PAGE_4K_ALIGN(x) (((x)&~(PAGE_4K-1))==(x))

#endif
