#ifndef INTERRUPT_H_
#define INTERRUPT_H_

#define PIC1        0x20        /* IO base address for master PIC */
#define PIC2        0xA0        /* IO base address for slave PIC */
#define PIC1_COMMAND	PIC1
#define PIC1_DATA	(PIC1+1)
#define PIC2_COMMAND	PIC2
#define PIC2_DATA	(PIC2+1)

#include "lib/types.h"

/* ParadiseOS            Initialization
 * This file contains the initialization structures and constants.
 * These constants are specifically used in the IDT and the GDT.
 */

typedef enum {
    dpl_User   = 3,
    dpl_Kernel = 0,
} DescriptorPrivilegeLevel;

/* Types of entries in the IDT (Interrupt Descriptor Table)
 *   - TASK: Task Gate
 *   - INT:  Interrupt Gate
 *   - TRAP: Trap Gate
 */
typedef enum {
    idtgt_Task =  5, // Task Gate.
    idtgt_Int  = 14, // Interrupt Gate.
    idtgt_Trap = 15, // Trap Gate
} GateType;

typedef struct { u64 value; } GateDescriptor;

void init_idt();

void init_pic();

void isr_handler(int interrupt); // General handler for first 32 system interrupts
void irq_handler(int interrupt); // General handler for IRQ

// IRQ1 - Keyboard on PS/2 Port - handler can be found in src/drivers/keyboard.h

void syscall_handler();

typedef struct {
    u32 edi, esi, ebp, esp, ebx, edx, ecx, eax;
    u32 int_no, eflags, useresp;
} InterruptRegisters;

#endif // INTERRUPT_H_