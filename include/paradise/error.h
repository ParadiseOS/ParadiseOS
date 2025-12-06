#ifndef ERROR_H_
#define ERROR_H_

#include "logging.h"

/**
 *  Assert that some condition holds. Log and panic otherwise. Note that logging
 *  depends on the terminal being successfully initialized.
 * */
#define KERNEL_ASSERT(condition)                                               \
    do {                                                                       \
        if (!(condition)) {                                                    \
            printk(                                                            \
                CRITICAL, __FILE__ ":%u Assertion Failed: " #condition,        \
                __LINE__                                                       \
            );                                                                 \
            kernel_panic();                                                    \
        }                                                                      \
    } while (false)

#define RESULT __attribute__((warn_unused_result)) bool

extern __attribute__((noreturn)) void panic_handler();

__attribute__((noreturn)) void kernel_panic();

#endif // ERROR_H_
