#include "timer.h"
#include "interrupts/interrupt.h"
#include "lib/types.h"
#include "lib/util.h"
#include "terminal/terminal.h"

void (*timer_callback)(InterruptRegisters *regs) = NULL;
const u32 freq = 1000;

void timer_handler(InterruptRegisters *regs) {
    if (timer_callback)
        timer_callback(regs);
}

void init_timer() {
    irq_install_handler(0, timer_handler);

    // PIT oscillates 1.1931816666 Mhz
    u32 divisor = 1193180 / freq;

    // Use Square Wave Generator Mode : 0011 0110
    outb(0x43, 0x36);
    outb(0x40, (u8) (divisor & 0xFF));
    outb(0x40, (u8) ((divisor >> 8) & 0xFF));
}
