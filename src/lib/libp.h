#ifndef LIBP_H_
#define LIBP_H_

#include "lib/types.h"

// NOTE: This should be reserved for Paradise specific things. At some point we
// will need to port libc and call it plibc.

void pmemset(void *memory, u8 byte, u32 count);
void pmemcpy(void *dst, const void *src, u32 count);
bool pmemeql(const void *a, const void *b, u32 count);
bool pstreql(const void *a, const void *b);

// see https://prng.di.unimi.it/xoroshiro128plus.c
typedef struct {
    u64 s[2];
} Prng;

void prng_init(Prng *rng, u64 seed);
u64 prng_next(Prng *rng);

#endif // LIBP_H_
