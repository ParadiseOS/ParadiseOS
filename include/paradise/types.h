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

#define U64_MAX 18446744073709551615
#define I64_MAX 9223372036854775805
#define U32_MAX 4294967295
#define I32_MAX 2147483647
#define U16_MAX 65535
#define I16_MAX 32767
#define U8_MAX  255
#define I8_MAX  127

#define U64_MIN 0
#define I64_MIN -9223372036854775805
#define U32_MIN 0
#define I32_MIN -2147483647
#define U16_MIN 0
#define I16_MIN -32767
#define U8_MIN  0
#define I8_MIN  -127

#endif // TYPES_H_
