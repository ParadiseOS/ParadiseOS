#ifndef TYPES_H_
#define TYPES_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// clang-format off
typedef uint64_t u64;
typedef int64_t  i64;
typedef uint32_t usize;
typedef int32_t  isize;
typedef uint32_t u32;
typedef int32_t  i32;
typedef uint16_t u16;
typedef int16_t  i16;
typedef uint8_t  u8;
typedef int8_t   i8;
typedef float    f32;
typedef double   f64;
// clang-format on

#define NAN          (*(const f64 *) (const u32[]) {0x00000000, 0x7FF80000})
#define INFINITY     (1.0f / 0.0f)
#define NEG_INFINITY (-INFINITY)

#endif // TYPES_H_
