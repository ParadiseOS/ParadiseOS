#ifndef INTERRUPT_H_
#define INTERRUPT_H_

#define PIC1         0x20 // IO base address for master PIC
#define PIC2         0xA0 // IO base address for slave PIC
#define PIC1_COMMAND PIC1
#define PIC1_DATA    (PIC1 + 1)
#define PIC2_COMMAND PIC2
#define PIC2_DATA    (PIC2 + 1)
#define PIC_EOI      0x20 // End of interrupt signal

#include "types.h"

typedef enum {
    dpl_User = 3,
    dpl_Kernel = 0,
} DescriptorPrivilegeLevel;

typedef enum {
    idtgt_Task = 5,  // Task Gate.
    idtgt_Int = 14,  // Interrupt Gate.
    idtgt_Trap = 15, // Trap Gate
} GateType;

typedef struct {
    u64 value;
} GateDescriptor;

typedef struct {
    u32 cr2;
    u32 edi, esi, ebp, esp, ebx, edx, ecx, eax;
    u32 int_no, err_code;
    u32 eip, cs, eflags, useresp, ss;
} InterruptRegisters;

void init_idt();
void irq_install_handler(u32 irq, void (*handler)(InterruptRegisters *reg));

void pic_eoi(u8 irq);

#endif // INTERRUPT_H_
