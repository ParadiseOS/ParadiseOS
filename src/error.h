#ifndef ERROR_H_
#define ERROR_H_

#include "terminal.h"

/**
 *  Assert that some condition holds. Log and panic otherwise. Note that logging
 *  depends on the terminal being successfully initialized.
 * */
#define KERNEL_ASSERT(condition)                                        \
    do {                                                                \
        if (!(condition)) {                                             \
            terminal_printf("[ERROR] Assertion Failed: " #condition);   \
            kernel_panic();                                             \
        }                                                               \
    } while (FALSE)

extern __attribute__((noreturn)) void panic_handler();

__attribute__((noreturn))
void kernel_panic();

#endif // ERROR_H_
