#include <paradise/error.h>
#include <paradise/interrupt.h>
#include <paradise/logging.h>
#include <paradise/mem.h>
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
void (*last_callback)(InterruptRegisters *regs
) = NULL; // Used for toggling callback
const u32 freq = 1000;

void preempt(InterruptRegisters *regs
); // used to store the state of the last process

void timer_handler(InterruptRegisters *regs) {
    pic_eoi(regs->int_no - 32); // Enable Interrupts again
    if (sched_tick_cur == 0) {
        sched_tick_cur = sched_ticks;
        if (sched_callback) {
            toggle_timer_callback(false); // Turn Timer Callbacks off 
            preempt(regs);
        }
    }
    ++system_ticks;
    --sched_tick_cur;
}

void toggle_timer_callback(bool enable) {
    if (enable) {
        sched_callback = last_callback;
    }
    else {
        sched_callback = NULL;
    }
}

SyscallResult
syscall_reg_tmr_cb(void (*callback)(InterruptRegisters *regs), u32 ticks) {
    if (ticks == 0)
        SYSCALL_ERR(1);
    sched_callback = callback;
    last_callback = callback;
    sched_ticks = ticks;
    SYSCALL_RET(0);
}

void init_timer() {
    irq_install_handler(0, timer_handler);

    system_ticks = 0;
    sched_ticks = DEFAULT_SCHED_TICKS; // Until user override w/ syscall
    sched_tick_cur = DEFAULT_SCHED_TICKS;
    sched_callback = NULL;
    last_callback = NULL;

    register_syscall(5, syscall_reg_tmr_cb);

    // PIT oscillates 1.1931816666 Mhz
    u32 divisor = 1193180 / freq;

    // Use Square Wave Generator Mode : 0011 0110
    outb(0x43, 0x36);
    outb(0x40, (u8) (divisor & 0xFF));
    outb(0x40, (u8) ((divisor >> 8) & 0xFF));
}
