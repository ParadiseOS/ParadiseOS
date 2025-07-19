#ifndef KEYBOARD_H_
#define KEYBOARD_H_

//! May want to move interrupt registers to util for convience
#include "interrupts/interrupt.h"

#define SCAN_PORT 0x60

void init_keyboard();
void keyboardHandler(InterruptRegisters *regs);

#endif // KEYBOARD_H_
