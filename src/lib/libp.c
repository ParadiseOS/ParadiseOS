#include "libp.h"

// NOTE: we can optimize our mass memory operations by operating on the 4-byte
// aligned u32 level instead of at a byte level

void pmemset(void *memory, u8 byte, u32 count) {
    char *bytes = memory;

    for (u32 i = 0; i < count; ++i) {
        bytes[i] = byte;
    }
}

void pmemcpy(void *dst, const void *src, u32 count) {
    u8 *dst_b = dst;
    const u8 *src_b = src;

    while (count--)
        *dst_b++ = *src_b++;
}

bool pmemeql(const void *a, const void *b, u32 count) {
    const u8 *a_bytes = a;
    const u8 *b_bytes = b;

    while (count--) {
        if (*a_bytes++ != *b_bytes++)
            return false;
    }

    return true;
}

bool pstreql(const void *a, const void *b) {
    const u8 *a_bytes = a;
    const u8 *b_bytes = b;
    u8 a_byte;
    u8 b_byte;

    do {
        a_byte = *a_bytes++;
        b_byte = *b_bytes++;
        if (a_byte != b_byte)
            return false;
    } while (a_byte);

    return true;
}

void prng_init(Prng *rng, u64 seed) {
    // seed using splitmix64
    // see: https://rosettacode.org/wiki/Pseudo-random_numbers/Splitmix64

    seed += 0x9e3779b97f4a7c15;
    seed = (seed ^ (seed >> 30)) * 0xbf58476d1ce4e5b9;
    seed = (seed ^ (seed >> 27)) * 0x94d049bb133111eb;
    rng->s[0] = seed ^ (seed >> 31);
}

u64 rotl(u64 x, u32 k) {
    return (x << k) | (x >> (64 - k));
}

u64 prng_next(Prng *rng) {
    u64 s0 = rng->s[0];
    u64 s1 = rng->s[1];
    u64 result = s0 + s1;

    s1 ^= s0;
    rng->s[0] = rotl(s0, 24) ^ s1 ^ (s1 << 16);
    rng->s[1] = rotl(s1, 37);

    return result;
}
