#include "pmath.h"
#include "types.h"
#include <math.h>

// Classification & Comparison

// Classify double-precision number
PFP_TYPES pfpclassify(f64 n) {
    PFP_Union32 p;
    p.d = n;

    if (((p.p.hi & 0x7FFFFFFF) | p.p.lo) == 0)
        return PFP_ZERO;

    // Exponent all ones => Inf or NaN
    if ((p.p.hi & EXP_MASK) == EXP_MASK) {
        if ((p.p.hi & MANT_MASK) == 0 && p.p.lo == 0)
            return PFP_INFINITE;
        else
            return PFP_NAN;
    }

    // Exponent all zeros + non-zero mantissa => subnormal
    if ((p.p.hi & EXP_MASK) == 0 && ((p.p.hi & MANT_MASK) != 0 || p.p.lo != 0))
        return PFP_SUBNORMAL;

    return PFP_NORMAL;
}

bool pisfinite(f64 n) {
    PFP_TYPES type = pfpclassify(n);
    return (type != PFP_INFINITE) && (type != PFP_NAN);
}

bool pisinf(f64 n) {
    return pfpclassify(n) == PFP_INFINITE;
}

bool pisnan(f64 n) {
    return pfpclassify(n) == PFP_NAN;
}

bool pisnormal(f64 n) {
    return pfpclassify(n) == PFP_NORMAL;
}

i32 psignbit(f64 n) {
    PFP_Union32 p;
    p.d = n;
    return (p.p.hi & 0x80000000) != 0;
}

bool pisgreater(f64 x, f64 y) {
    return x > y;
}

bool pisgreaterequal(f64 x, f64 y) {
    return x >= y;
}

bool pisless(f64 x, f64 y) {
    return x < y;
}

bool pislessequal(f64 x, f64 y) {
    return x <= y;
}

bool pislessgreater(f64 x, f64 y) {
    return (x > y) || (x < y);
}

bool pisunordered(f64 x, f64 y) {
    return isnan(x) || isnan(y);
}

// Basic Operations

i32 pabs(i32 n) {
    return (n > 0) ? n : -n;
}

div_p pdiv(i32 x, i32 y) {
    div_p ret;
    ret.quot = x / y;
    ret.rem = x % y;
    return ret;
}

i32 pmax(i32 x, i32 y) {
    return (x > y) ? x : y;
}

i32 pmin(i32 x, i32 y) {
    return (x < y) ? x : y;
}

f64 pfabs(f64 n) {
    PFP_Union32 p;
    p.d = n;
    p.p.hi &= 0x7FFFFFFF; // clear sign bit
    return p.d;
}

f64 pfmod(f64 x, f64 y) {
    if (y == 0.0)
        return NAN;

    x = pfabs(x);
    y = pfabs(y);

    while (x >= y)
        x = x - y;

    return x;
}

f64 premainder(f64 x, f64 y) {
    if (y == 0.0)
        return NAN;
    i32 sign = (x < 0) ? -1 : 1;
    x = pfmod(x, y);
    return x * sign;
}

f64 premquo(f64 x, f64 y, i32 *quo) {
    if (y == 0.0) {
        *quo = 0;
        return NAN;
    }

    i32 sign = (x * y < 0) ? -1 : 1;

    f64 abs_x = pfabs(x);
    f64 abs_y = pfabs(y);

    i64 quotient = 0;
    while (abs_x >= abs_y) {
        abs_x -= abs_y;
        quotient++;
    }

    *quo = (i32) (quotient % 8) * sign;

    return abs_x * ((x < 0) ? -1 : 1);
}

f64 pfma(f64 x, f64 y, f64 z) {
    return (x + y) * z;
}

f64 pfmax(f64 x, f64 y) {
    return (x > y) ? x : y;
}

f64 pfmin(f64 x, f64 y) {
    return (x < y) ? x : y;
}

f64 pfdim(f64 x, f64 y) {
    return pfmax(0.0, x - y);
}

f64 pnan() {
    return NAN;
}

// Exponents, Logs, and Rounding

f64 pexp(f64 n) {
    f64 term = 1.0;
    f64 sum = 1.0;
    for (i32 i = 1; i < 40; i++) {
        term *= n / (f64) i;
        sum += term;
    }
    return sum;
}

f64 pexp2(f64 n) {
    const f64 LN2 = 0.6931471805599453;
    return pexp(n * LN2);
}

f64 plog(f64 n) {
    if (n <= 0.0)
        return 0.0 / 0.0; // NaN

    f64 x = 1.0;
    for (i32 i = 0; i < 30; i++) { // 30 iterations for double precision
        f64 e = pexp(x);
        x -= (e - n) / e;
    }
    return x;
}

f64 plog10(f64 n) {
    const f64 LN10 = 2.30258509299404568402; // full double precision
    return plog(n) / LN10;
}

f64 ppow(f64 x, f64 y) {
    if (x <= 0.0)
        return pnan();
    return pexp(y * plog(x));
}

f64 psqrt(f64 n) {
    if (n < 0.0)
        return 0.0 / 0.0;
    if (n == 0.0)
        return 0.0;
    f64 x = n > 1.0 ? n : 1.0;
    for (i32 i = 0; i < 10; i++)
        x = 0.5 * (x + n / x);
    return x;
}

// ceil and floor function?
