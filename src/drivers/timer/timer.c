#include "timer.h"
#include "lib/types.h"
#include "lib/util.h"
#include "terminal/terminal.h" // Used for testing

u64 ticks;
const u32 freq = 100;

void timer_handler() {
    ++ticks;

    #ifdef TESTS_ENABLED
    if (ticks % 100 == 0) // Prints every second
        terminal_printf("Timer ticked..\n");
    #endif
}

void init_timer() {
    ticks = 0;
    irq_install_handler(0, &timer_handler);

    // PIT oscillates 1.1931816666 Mhz
    u32 divisor = 1193180 / freq;

    // Use Square Wave Generator Mode : 0011 0110
    outb(0x43, 0x36);
    outb(0x40, (u8)(divisor & 0xFF));
    outb(0x40, (u8)((divisor >> 8) & 0xFF));
}
