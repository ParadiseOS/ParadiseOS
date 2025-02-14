#ifndef INTERRUPT_H_
#define INTERRUPT_H_

#define PIC1            0x20        /* IO base address for master PIC */
#define PIC2            0xA0        /* IO base address for slave PIC  */
#define PIC1_COMMAND    PIC1
#define PIC1_DATA       (PIC1+1)
#define PIC2_COMMAND    PIC2
#define PIC2_DATA       (PIC2+1)
#define PIC_EOI         0x20         /* End of interrupt signal */

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

typedef struct {
    uint32_t cr2;
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
    uint32_t int_no, err_code;
    uint32_t eip, cs, eflags, useresp, ss;
} InterruptRegisters;

void init_idt();
void irq_install_handler(u32 irq, void (*handler)(InterruptRegisters* reg));
void syscall_handler();

#endif // INTERRUPT_H_