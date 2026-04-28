#include "pmath.h"
#include "stdio.h"
#include "testlib.h"
#include "types.h"

// Test Variables

// Helper Functions

bool div_t_compare(div_p a, div_p b) {
    return (a.quot == b.quot) && (a.rem == b.rem);
}

// Classification & Comparison

// Add Classification Functions

void classify_nan() {
    PFP_TYPES actual;
    f64 x = 0.0 / 0.0;
    PFP_TYPES expected = PFP_NAN;

    actual = pfpclassify(x);

    assert_continue(actual == expected);
}

void classify_inf() {
    PFP_TYPES actual;
    f64 x = 123.456 / 0.0;
    PFP_TYPES expected = PFP_INFINITE;

    actual = pfpclassify(x);

    assert_continue(actual == expected);
}

void classify_neg_inf() {
    PFP_TYPES actual;
    f64 x = -654.321 / 0.0;
    PFP_TYPES expected = PFP_INFINITE;

    actual = pfpclassify(x);

    assert_continue(actual == expected);
}

void classify_zero() {
    PFP_TYPES actual;
    f64 x = 0.0;
    PFP_TYPES expected = PFP_ZERO;

    actual = pfpclassify(x);

    assert_continue(actual == expected);
}

void classify_neg_zero() {
    PFP_TYPES actual;
    f64 x = -0.0;
    PFP_TYPES expected = PFP_ZERO;

    actual = pfpclassify(x);

    assert_continue(actual == expected);
}

void classify_sub_normal() {
    PFP_TYPES actual;
    f64 x = 0.69e-320;
    PFP_TYPES expected = PFP_SUBNORMAL;

    actual = pfpclassify(x);

    assert_continue(actual == expected);
}

void classify_normal() {
    PFP_TYPES actual;
    f64 x = 69.420;
    PFP_TYPES expected = PFP_NORMAL;

    actual = pfpclassify(x);

    assert_continue(actual == expected);
}

// The remaining classification functions are trivial so we omit them.

// Basic Operations

void abs_of_3() {
    i32 actual;
    i32 n = 3;
    i32 expected = 3;

    actual = pabs(n);

    assert_continue(actual == expected);
}

void abs_of_neg_3() {
    i32 actual;
    i32 n = -3;
    i32 expected = 3;

    actual = pabs(n);

    assert_continue(actual == expected);
}

void div_5_by_2() {
    div_p actual;
    i32 x = 5, y = 2;
    div_p expected = {/* quot */ 2, /* rem */ 1};

    actual = pdiv(x, y);

    assert_continue(div_t_compare(actual, expected));
}

void div_2_by_5() {
    div_p actual;
    i32 x = 2, y = 5;
    div_p expected = {/* quot */ 0, /* rem */ 2};

    actual = pdiv(x, y);

    assert_continue(div_t_compare(actual, expected));
}

void div_420_by_69() {
    div_p actual;
    i32 x = 420, y = 69;
    div_p expected = {/* quot */ 6, /* rem */ 6};

    actual = pdiv(x, y);

    assert_continue(div_t_compare(actual, expected));
}

void max_5_and_2() {
    i32 actual;
    i32 x = 5, y = 2;
    i32 expected = 5;

    actual = pmax(x, y);

    assert_continue(actual == expected);
}

void min_5_and_2() {
    i32 actual;
    i32 x = 5, y = 2;
    i32 expected = 2;

    actual = pmin(x, y);

    assert_continue(actual == expected);
}

void fabs_of_420_69() {
    f64 actual;
    f64 n = 420.69;
    f64 expected = 420.69;

    actual = pfabs(n);

    assert_continue(actual == expected);
}

void fabs_of_neg_420_69() {
    f64 actual;
    f64 n = -420.69;
    f64 expected = 420.69;

    actual = pfabs(n);

    assert_continue(actual == expected);
}

void fmod_returns_nan() {
    f64 actual;
    f64 x = 5, y = 0;

    actual = pfmod(x, y);

    assert_continue(pisnan(actual));
}

void fmod_10_by_3() {
    f64 actual;
    f64 x = 10.5, y = 3.0;
    f64 expected = 1.5;

    actual = pfmod(x, y);

    assert_continue(actual == expected);
}

void fmod_3_by_10() {
    f64 actual;
    f64 x = 3.0, y = 10.5;
    f64 expected = 3.0;

    actual = pfmod(x, y);

    assert_continue(actual == expected);
}

void premainder_10_by_3() {
    f64 actual;
    f64 x = 10.5, y = 3.0;
    f64 expected = 1.5;

    actual = premainder(x, y);

    assert_continue(actual == expected);
}

void premainder_neg10_by_3() {
    f64 actual;
    f64 x = -10.5, y = 3.0;
    f64 expected = -1.5;

    actual = premainder(x, y);

    assert_continue(actual == expected);
}

void premainder_returns_nan() {
    f64 actual;
    f64 x = 10.0, y = 0.0;

    actual = premainder(x, y);

    assert_continue(pisnan(actual));
}

void premquo_10_by_3() {
    f64 actual_rem;
    i32 actual_quo;
    f64 x = 10.5, y = 3.0;
    f64 expected_rem = 1.5;
    i32 expected_quo = 3;

    actual_rem = premquo(x, y, &actual_quo);

    assert_continue(actual_rem == expected_rem);
    assert_continue(actual_quo == expected_quo);
}

void premquo_3_by_10() {
    f64 actual_rem;
    i32 actual_quo;
    f64 x = 3.0, y = 10.5;
    f64 expected_rem = 3.0;
    i32 expected_quo = 0;

    actual_rem = premquo(x, y, &actual_quo);

    assert_continue(actual_rem == expected_rem);
    assert_continue(actual_quo == expected_quo);
}

void premquo_returns_nan() {
    f64 actual_rem;
    i32 actual_quo;
    f64 x = 10.5, y = 0.0;
    i32 expected_quo = 0;

    actual_rem = premquo(x, y, &actual_quo);

    assert_continue(pisnan(actual_rem));
    assert_continue(actual_quo == expected_quo);
}

void pfma_3_4_5() {
    f64 actual;
    f64 x = 3.25, y = 4.5, z = 5.75;
    f64 expected = 44.5625;

    actual = pfma(x, y, z);

    assert_continue(actual == expected);
}

void pfma_returns_nan() {
    f64 actual;
    f64 x = 3.25, y = 3.5, z = NAN;

    actual = pfma(x, y, z);

    assert_continue(pisnan(actual));
}

void pfma_returns_inf() {
    f64 actual;
    f64 x = 3.25, y = 3.5, z = INFINITY;

    actual = pfma(x, y, z);

    assert_continue(pisinf(actual));
}

void pfmax_10_3() {
    f64 actual;
    f64 x = 10.5, y = 3.0;
    f64 expected = 10.5;

    actual = pfmax(x, y);

    assert_continue(actual == expected);
}

void pfmin_10_3() {
    f64 actual;
    f64 x = 10.5, y = 3.0;
    f64 expected = 3.0;

    actual = pfmin(x, y);

    assert_continue(actual == expected);
}

void pfdim_10_3() {
    f64 actual;
    f64 x = 10.5, y = 3.0;
    f64 expected = 7.5;

    actual = pfdim(x, y);

    assert_continue(actual == expected);
}

// Exponents, Logs, and Rounding

void pexp_0() {
    f64 actual;
    f64 x = 0.0;
    f64 expected = 1.0;

    actual = pexp(0);

    assert_continue(actual == expected);
}

void pexp_1() {
    f64 actual;
    f64 x = 1.0;
    f64 expected = 2.718281828459045; // e

    actual = pexp(x);

    // allow some small tolerance
    assert_continue(pfabs(actual - expected) < 1e-12);
}

void pexp_5() {
    f64 actual;
    f64 x = 5.0;
    f64 expected = 148.4131591025766; // e^5

    actual = pexp(x);

    assert_continue(pfabs(actual - expected) < 1e-10);
}

void pexp2_0() {
    f64 actual;
    f64 x = 0.0;
    f64 expected = 1.0; // 2^0

    actual = pexp2(x);

    assert_continue(actual == expected);
}

void pexp2_1() {
    f64 actual;
    f64 x = 1.0;
    f64 expected = 2.0; // 2^1

    actual = pexp2(x);

    assert_continue(pfabs(actual - expected) < 1e-12);
}

void pexp2_5() {
    f64 actual;
    f64 x = 5.0;
    f64 expected = 32.0; // 2^5

    actual = pexp2(x);

    assert_continue(pfabs(actual - expected) < 1e-12);
}

void plog_0() {
    f64 actual;
    f64 x = 0.0;

    actual = plog(x);

    assert_continue(pisnan(actual));
}

void plog_1() {
    f64 actual;
    f64 x = 1.0;
    f64 expected = 0.0;

    actual = plog(x);

    assert_continue(pfabs(actual - expected) < 1e-12);
}

void plog_5() {
    f64 actual;
    f64 x = 5.0;
    f64 expected = 1.6094379124341003; // ln(5)

    actual = plog(x);

    assert_continue(pfabs(actual - expected) < 1e-12);
}

void plog10_0() {
    f64 actual;
    f64 x = 0.0;

    actual = plog10(x);

    assert_continue(pisnan(actual)); // log10(0) = -Inf, plog10 returns NaN
}

void plog10_1() {
    f64 actual;
    f64 x = 1.0;
    f64 expected = 0.0;

    actual = plog10(x);

    assert_continue(pfabs(actual - expected) < 1e-12);
}

void plog10_50() {
    f64 actual;
    f64 x = 50.0;
    f64 expected = 1.6989700043360187; // log10(50)

    actual = plog10(x);

    assert_continue(pfabs(actual - expected) < 1e-12);
}

void ppow_0_5() {
    f64 actual;
    f64 x = 0.5, y = 2.0;
    f64 expected = 0.25;

    actual = ppow(x, y);

    assert_continue(pfabs(actual - expected) < 1e-12);
}

void ppow_2_3() {
    f64 actual;
    f64 x = 2.0, y = 3.0;
    f64 expected = 8.0;

    actual = ppow(x, y);

    assert_continue(pfabs(actual - expected) < 1e-12);
}

void ppow_neg() {
    f64 actual;
    f64 x = -1.0, y = 2.0;

    actual = ppow(x, y);

    assert_continue(pisnan(actual));
}

void psqrt_0() {
    f64 actual;
    f64 x = 0.0;
    f64 expected = 0.0;

    actual = psqrt(x);

    assert_continue(pfabs(actual - expected) < 1e-12);
}

void psqrt_4() {
    f64 actual;
    f64 x = 4.0;
    f64 expected = 2.0;

    actual = psqrt(x);

    assert_continue(pfabs(actual - expected) < 1e-12);
}

void psqrt_neg() {
    f64 actual;
    f64 x = -1.0;

    actual = psqrt(x);

    assert_continue(pisnan(actual));
}

int main() {

    // Classification & Comparison

    // Add classification tests and such
    test_handler("classify nan", classify_nan);
    test_handler("classify infinity", classify_inf);
    test_handler("classify negative infinity", classify_neg_inf);
    test_handler("classify positive zero", classify_zero);
    test_handler("classify negative zero", classify_neg_zero);
    test_handler("classify sub normal", classify_sub_normal);
    test_handler("classify normal", classify_normal);

    // Basic Operations
    test_handler("abs(3)", abs_of_3);
    test_handler("abs(-3)", abs_of_neg_3);

    test_handler("div 5 by 2", div_5_by_2);
    test_handler("div 2 by 5", div_2_by_5);
    test_handler("div 420 by 69", div_420_by_69);

    test_handler("max(5,2)", max_5_and_2);
    test_handler("min(5,2)", min_5_and_2);

    test_handler("fabs(420.69)", fabs_of_420_69);
    test_handler("fabs(-420.69)", fabs_of_neg_420_69);

    test_handler("fmod(10.5, 0) returns nan", fmod_returns_nan);
    test_handler("fmod(10.5, 3)", fmod_10_by_3);
    test_handler("fmod(3, 10.5)", fmod_3_by_10);

    test_handler("premainder(10.5, 3)", premainder_10_by_3);
    test_handler("premainder(-10.5, 3)", premainder_neg10_by_3);
    test_handler("premainder(10.5, 0) returns nan", premainder_returns_nan);

    test_handler("premquo(10.5, 3)", premquo_10_by_3);
    test_handler("premquo(3, 10.5)", premquo_3_by_10);
    test_handler("premquo(10.5, 0) returns nan", premquo_returns_nan);

    test_handler("pfma(3.25, 4.5, 5.66)", pfma_3_4_5);
    test_handler("pfma(3.25, 4.5, NAN) returns nan", pfma_returns_nan);
    test_handler("pfma(3.25, 4.5, INF) returns inf", pfma_returns_inf);

    test_handler("pfmax(10.5, 3)", pfmax_10_3);
    test_handler("pfmin(10.5, 3)", pfmin_10_3);
    test_handler("pfdim(10.5, 3)", pfdim_10_3);

    // Exponents, Logs, and Rounding
    test_handler("pexp(0)", pexp_0);
    test_handler("pexp(1)", pexp_1);
    test_handler("pexp(5)", pexp_5);

    test_handler("pexp2(0)", pexp2_0);
    test_handler("pexp2(1)", pexp2_1);
    test_handler("pexp2(5)", pexp2_5);

    test_handler("plog(0)", plog_0);
    test_handler("plog(1)", plog_1);
    test_handler("plog(5)", plog_5);

    test_handler("plog10(0)", plog10_0);
    test_handler("plog10(1)", plog10_1);
    test_handler("plog10(50)", plog10_50);

    test_handler("ppow(0.5, 2)", ppow_0_5);
    test_handler("ppow(2, 3)", ppow_2_3);
    test_handler("ppow(-1, 2)", ppow_neg);

    test_handler("psqrt(0)", psqrt_0);
    test_handler("psqrt(4)", psqrt_4);
    test_handler("psqrt(-1)", psqrt_neg);

    // Output
    test_output(__FILE__);
}