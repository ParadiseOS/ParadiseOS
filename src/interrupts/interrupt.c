#include "interrupt.h"
#include "kernel/init.h"
#include "lib/error.h"
#include "terminal/terminal.h"
#include "drivers/keyboard/keyboard.h"
#include "lib/util.h"

extern u32 cpu_interrupts;

extern void load_idt(TablePointer *);
extern void keyboard_wrapper();
extern void syscall_wrapper();

const u32 *cpu_interrupt_table = &cpu_interrupts;

GateDescriptor idt[256];

TablePointer idt_ptr = { sizeof (idt) - 1, idt };

// || handler[31:16] || present || privilege level || zero || gate type || reserved || segment selector || handler[15:0] ||
// || 63          48 || 47      || 46           45 || 44   || 43     40 || 39    32 || 31            16 || 15          0 ||
GateDescriptor make_gate_descriptor(u32 handler, u8 type, u8 privilege_level) {
    u64 descriptor = 0;
    u8 pdpl0 = 0b1000 | ((privilege_level & 0b11) << 1); // present bit, privilege level, and zero bit

    descriptor |= handler >> 16; descriptor <<= 4;
    descriptor |= pdpl0;         descriptor <<= 4;
    descriptor |= type & 0xF;    descriptor <<= 24;
    descriptor |= 8;             descriptor <<= 16; // select first segment
    descriptor |= handler & 0xFFFF;

    return (GateDescriptor) { descriptor };
}

void* irq_routines[16] = {
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0
};

void syscall_handler() {
    terminal_printf("syscalled!\n");
    terminal_printf("CPL: %u\n", get_privilege_level());
}

void init_pic() {
    outb(PIC1_COMMAND, 0x11);
    outb(PIC2_COMMAND, 0x11);

    outb(PIC1_DATA, 0x20);
    outb(PIC2_DATA, 0x28);

    outb(PIC1_DATA, 0x04);
    outb(PIC2_DATA, 0x02);

    outb(PIC1_DATA, 0x01);
    outb(PIC2_DATA, 0x01);

    outb(PIC1_DATA, 0x0);
    outb(PIC2_DATA, 0x0);

    outb(PIC1_DATA,0xfd);
    outb(PIC2_DATA,0xff);
    asm("sti");
}

void init_idt() {
    for(u32 i = 0; i < 256; i++) idt[i] = (GateDescriptor) { 0 };

    init_pic();
    
    for (u32 i = 0; i < 32; ++i) {
        idt[i] = make_gate_descriptor((u32)cpu_interrupt_table[i], idtgt_Trap, dpl_Kernel);
    }

    for (u32 i = 32; i < 48; i++){
        idt[i] = make_gate_descriptor((u32)cpu_interrupt_table[i], idtgt_Int, dpl_Kernel);
    }
    idt[0x80] = make_gate_descriptor((u32) syscall_wrapper, idtgt_Int, dpl_User);

    load_idt(&idt_ptr);
}

void pic_eoi(u8 irq) {
    if (irq > 8)
        outb(PIC2_COMMAND, PIC_EOI);
    outb(PIC1_COMMAND, PIC_EOI);
}

const char *INTERRUPT_NAMES[32] = {
    "Division Error",
    "Debug",
    "Non-maskable Interrupt",
    "Breakpoint",
    "Overflow",
    "Bound Range Exceeded",
    "Invalid Opcode",
    "Device Not Available",
    "Double Fault",
    "Coprocessor Segment Overrun",
    "Invalid TSS",
    "Segment Not Present",
    "Stack-Segment Fault",
    "General Protection Fault",
    "Page Fault",
    "Reserved",
    "x87 Floating-Point Exception",
    "Alignment Check",
    "Machine Check",
    "SIMD Floating-Point Exception",
    "Virtualization Exception",
    "Control Protection Exception",
    "Reserved",
    "Hypervisor Injection Exception",
    "VMM Communication Exception",
    "Security Exception",
    "Reserved",
};

void irq_install_handler(u32 irq, void (*handler)(InterruptRegisters* reg)) {
    irq_routines[irq] = handler;
}

void irq_uninstall_handler(u32 irq) {
    irq_routines[irq] = 0;
}

void irq_handler(InterruptRegisters* regs) {
    void (*handler)(InterruptRegisters* regs);

    u8 irq = regs->int_no - 32;

    handler = irq_routines[irq];

    if (handler)
        handler(regs);
    pic_eoi(irq);
}

void isr_handler(InterruptRegisters* regs) {
    u32 interrupt = regs->int_no;
    terminal_printf("Interrupt - %i\n", interrupt);
    KERNEL_ASSERT(interrupt < 48);
    if (interrupt >= 32)
        irq_handler(regs);
    else {
        terminal_printf("[INT] %s\n", INTERRUPT_NAMES[interrupt]);
        kernel_panic();
    }
}