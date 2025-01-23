#include "error.h"

void kernel_panic() {
    panic_handler();
    __builtin_unreachable();
}
