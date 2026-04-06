#ifndef PMATH_H_
#define PMATH_H_

#include "types.h"

// Definitions

#define EXP_MASK  0x7FF00000
#define MANT_MASK 0x000FFFFF
#define SIGN_MASK 0x80000000

typedef struct {
    int quot;
    int rem;
} div_p;

typedef enum PFP_TYPES {
    PFP_NAN,
    PFP_INFINITE,
    PFP_ZERO,
    PFP_SUBNORMAL,
    PFP_NORMAL
} PFP_TYPES;

typedef union {
    double d;
    struct {
        uint32_t lo;
        uint32_t hi;
    } p;
} PFP_Union32;

// Classification & Comparison

// Categorizes floating point value n;
PFP_TYPES pfpclassify(f64 n);

// Determines if a float n is finite
bool pisfinite(f64 n);

// Determines if a float n is infinite
bool pisinf(f64 n);

// Determines if a float n is nan
bool pisnan(f64 n);

// Determines if a float n is normal
bool pisnormal(f64 n);

// Determines if a float n is signed, Non zero if float is negative
i32 psignbit(f64 n);

// Determines if a float x is greater than float y
bool pisgreater(f64 x, f64 y);

// Determines if a float x is greater than or equal to float y
bool pisgreaterequal(f64 x, f64 y);

// Determines if a float x is less than float y
bool pisless(f64 x, f64 y);

// Determines if a float x is less than or equal to float y
bool pislessequal(f64 x, f64 y);

// Determines if a float x is less than or greater than float y
bool pislessgreater(f64 x, f64 y);

// Determines if either float x or y are NAN
bool pisunordered(f64 x, f64 y);

// Basic Operations

// Computes the absolute value of int n
i32 pabs(i32 n);

// Computes quotient and remainder of integer division
div_p pdiv(i32 x, i32 y);

// Determines larger of two ints x and y
i32 pmax(i32 x, i32 y);

// Determines smaller of two ints x and y
i32 pmin(i32 x, i32 y);

// Computes the absolute value of floating point value n
f64 pfabs(f64 n);

// Computes remainder of the floating point division operation
f64 pfmod(f64 x, f64 y);

// Computes the signed remainder of the floating point division operation
f64 premainder(f64 x, f64 y);

// Computes signed remainder as well as last 3 bits of the division operation
f64 premquo(f64 x, f64 y, i32 *quo);

// Computes fused multiply-add operation
f64 pfma(f64 x, f64 y, f64 z);

// Determines larger of the two floating point values
f64 pfmax(f64 x, f64 y);

// Determines smaller of two floating point values
f64 pfmin(f64 x, f64 y);

// Determines positive difference of two floating point values
f64 pfdim(f64 x, f64 y);

// Returns NaN (not-a-number)
f64 pnan();

// Exponents, Logs, and Rounding

// Compute Euler number to the power of n
f64 pexp(f64 n);

// Computes 2 raised to the power of n
f64 pexp2(f64 n);

// Computes the natural log of n
f64 plog(f64 n);

// Computes the common log of n
f64 plog10(f64 n);

// Computes the value of x raised ot the power of y
f64 ppow(f64 x, f64 y);

// Computes the square root of n
f64 psqrt(f64 n);

// ceil and floor function?

#endif // PMATH_H_