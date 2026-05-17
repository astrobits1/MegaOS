#include <libk/hashtb.h>

/* Knuth's Multiplicative Hash (64 bit) 
 * This serves the purpose of distributing PFN derived indices well
 * enough throughout the list while being cheap and fast */
uint64_t hash_multiplicative(uint64_t key) {
    const uint64_t c = 0x9e3779b97f4a7c15ULL;
    return key*c;
}
