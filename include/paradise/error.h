#ifndef ERROR_H_
#define ERROR_H_

#include "logging.h"

#define STR_IMPL(x) #x
#define STR(x)      STR_IMPL(x)

/**
 *  Assert that some condition holds. Log and panic otherwise. Note that logging
 *  depends on the terminal or serial port being successfully initialized.
 * */
#define KERNEL_ASSERT(condition)                                               \
    do {                                                                       \
        if (condition) {}                                                      \
        else {                                                                 \
            printk(                                                            \
                CRITICAL,                                                      \
                __FILE__ ":" STR(__LINE__) ": Assertion Failed: " #condition   \
            );                                                                 \
            kernel_panic();                                                    \
        }                                                                      \
    } while (false)

#define RESULT __attribute__((warn_unused_result)) bool

#define ERR_OK            0
#define ERR_OOM           1
#define ERR_PID_NOT_FOUND 2
#define ERR_INVALID_INPUT 3
#define ERR_ENT_NOT_FOUND 4
#define ERR_ENT_EXISTS    5
#define ERR_OP_TOO_LARGE  6

extern __attribute__((noreturn)) void panic_handler();

__attribute__((noreturn)) void kernel_panic();

#endif // ERROR_H_
