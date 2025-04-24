#ifndef TIMER_H_
#define TIMER_H_

#include "interrupts/interrupt.h"

extern void (*timer_callback)(InterruptRegisters *regs);

void init_timer();

#endif // TIMER_H_
