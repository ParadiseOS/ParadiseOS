#include <paradise/error.h>
#include <paradise/interrupt.h>
#include <paradise/syscall.h>
#include <paradise/terminal.h>
#include <paradise/timer.h>
#include <paradise/types.h>
#include <paradise/util.h>

#define DEFAULT_SCHED_TICKS 10

static u64 system_ticks; // Total number of system ticks
static u32 sched_ticks;  // Ticks between scheduler function call
static u32 sched_tick_cur;

void (*sched_callback)(InterruptRegisters *regs) = NULL;
const u32 freq = 1000;

void timer_no_op() {
    return;
}

void timer_handler(InterruptRegisters *regs) {
    if (sched_tick_cur == 0) {
        sched_tick_cur = sched_ticks;
        sched_callback(regs);
    }
    ++system_ticks;
    --sched_tick_cur;
}

SyscallResult
syscall_reg_tmr_cb(void (*callback)(InterruptRegisters *regs), u32 ticks) {
    if (ticks == 0)
        SYSCALL_ERR(1);
    sched_callback = callback;
    sched_ticks = ticks;
    SYSCALL_RET(0);
}

void init_timer() {
    irq_install_handler(0, timer_handler);

    system_ticks = 0;
    sched_ticks = DEFAULT_SCHED_TICKS; // Until user override w/ syscall
    sched_tick_cur = DEFAULT_SCHED_TICKS;
    sched_callback = timer_no_op;

    register_syscall(5, syscall_reg_tmr_cb);

    // PIT oscillates 1.1931816666 Mhz
    u32 divisor = 1193180 / freq;

    // Use Square Wave Generator Mode : 0011 0110
    outb(0x43, 0x36);
    outb(0x40, (u8) (divisor & 0xFF));
    outb(0x40, (u8) ((divisor >> 8) & 0xFF));
}
