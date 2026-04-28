#ifndef TESTLIB_H_
#define TESTLIB_H_
#include "stdio.h"

extern int tests_run;
extern int tests_failed;

#ifndef NO_ASSERT_PRINT
#define assert_continue(cond)                                                  \
    do {                                                                       \
        if (!(cond)) {                                                         \
            extern int test_failed;                                            \
            test_failed = 1;                                                   \
            printf("Assertion Failed - %s (%s)\n", __func__, #cond);           \
        }                                                                      \
    } while (0)

#define assert_continue_msg(cond, msg)                                         \
    do {                                                                       \
        if (!(cond)) {                                                         \
            extern int test_failed;                                            \
            test_failed = 1;                                                   \
            printf("Assertion Failed - %s (%s) : %s\n", __func__, #cond, msg); \
        }                                                                      \
    } while (0)
#else
#define assert_continue(cond)                                                  \
    do {                                                                       \
        if (!(cond)) {                                                         \
            extern int test_failed;                                            \
            test_failed = 1;                                                   \
        }                                                                      \
    } while (0)

#define assert_continue_msg(cond, msg)                                         \
    do {                                                                       \
        if (!(cond)) {                                                         \
            extern int test_failed;                                            \
            test_failed = 1;                                                   \
        }                                                                      \
    } while (0)
#endif

typedef void (*test_func_t)();

// Handles testing a function
// Increments testlib global vars
// Tests should have no arguments
void test_handler(const char *func_name, test_func_t func);

// Outputs results of tested file
void test_output(const char *filename);

// Testing output
// Use flags --compact or --verbose to set
void test_print(const char *func_name);

// Verbose print for extra info when --verbose set
void verbose_print(const char *func_name, const char *str);

#endif // TESTLIB_H_