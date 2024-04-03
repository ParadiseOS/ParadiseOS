#include "terminal.h"
#include "interrupt.h"
#include "error.h"

void test_interrupt() {
    terminal_printf("HAHAHAHHAHAHAHA 696969696996\n");
    return;
}
void general_protection() {
    terminal_printf("GENERAL PROTECTION ERROR\n");
    KERNEL_ASSERT(0);
}
