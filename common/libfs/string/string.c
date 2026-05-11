#include <libfs/string.h>
#include <stdint.h>

/* TODO
 * memcpy and memset can be optimized using qword/dword/word fitting to minimize writes */ 

void* memcpy(void* __restrict dst, const void* __restrict src, size_t n) {
    const uint8_t* u8_src = (const uint8_t*)src;
    uint8_t* u8_dst = (uint8_t*)dst;

    for (size_t i=0; i<n; i++) {
        u8_dst[i] = u8_src[i];
    }

    return dst;
}

void* memmove(void* dst, const void* src, size_t n) {
    uint8_t* u8_dst = (uint8_t*)dst;
    const uint8_t* u8_src = (const uint8_t*)src;

    if ((uintptr_t)u8_dst > (uintptr_t)u8_src && (uintptr_t)u8_dst < (uintptr_t)(u8_src)+n) {
        /* Source first destination second overlap
         * Solution: Copy in reverse (index n-1) */
        for (size_t i=n-1 ;; i--) {
            u8_dst[i] = u8_src[i];

            if (i==0) break;
        }
    } else {
        /* Destination first source second overlap 
         * OR no overlap
         * Solution: Copy forward (index 0) */
        for (size_t i=0; i<n; i++) {
            u8_dst[i] = u8_src[i];
        }
    }

    return dst;
}

int memcmp(const void* s1, const void* s2, size_t n) {
    unsigned char* uc_s1 = (unsigned char*)s1;
    unsigned char* uc_s2 = (unsigned char*)s2;

    for (size_t i=0; i<n; i++) {
        if (uc_s1[i] > uc_s2[i] || uc_s1[i] < uc_s2[i]) 
            return uc_s2[i]-uc_s1[i];
    }

    return 0;
}

void* memset(void* s, int c, size_t n) {
    uint8_t* u8_s = (uint8_t*)s;

    for (size_t i=0; i<n; i++) {
        u8_s[i] = c;
    }

    return s;
}

size_t strlen(const char* s) {
    for (size_t i=0; i<SIZE_MAX; i++) {
        if (s[i] == '\0') return i;
    }

    return SIZE_MAX;
}
