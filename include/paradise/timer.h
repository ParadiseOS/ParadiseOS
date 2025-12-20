#ifndef TIMER_H_
#define TIMER_H_

#include "interrupt.h"
#include "syscall.h"
#include "types.h"

extern void (*sched_callback)(InterruptRegisters *regs);
extern void (*last_callback)(InterruptRegisters *regs);

// This function is here for testing until we have all the syscalls for user
// mode schedulers.
SyscallResult
syscall_reg_tmr_cb(void (*callback)(InterruptRegisters *regs), u32 ticks);

// Used to toggle timer callback on and off without a system call.
void toggle_timer_callback(bool enable);

void init_timer();

#endif // TIMER_H_
