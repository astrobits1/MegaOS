#ifndef LIBFS_STRING_H
#define LIBFS_STRING_H
#ifdef __cplusplus
extern "C" {
#endif

#include <libfs/cdefs.h>
#include <stddef.h>

void* memcpy(void* __restrict dst, const void* __restrict src, size_t n);
void* memmove(void* dst, const void* src, size_t n);
int memcmp(const void* s1, const void* s2, size_t n);
void* memset(void* s, int c, size_t n);
size_t strlen(const char* s);

#ifdef __cplusplus
}
#endif
#endif
