#ifndef INTERRUPT_H_
#define INTERRUPT_H_

#include "lib/types.h"

void interrupt_handler(int interrupt);

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

#endif // INTERRUPT_H_