#include "interrupt.h"
#include "drivers/keyboard/keyboard.h"
#include "kernel/init.h"
#include "kernel/kernel.h"
#include "lib/error.h"
#include "lib/util.h"
#include "terminal/terminal.h"

extern u32 cpu_interrupts;

extern void load_idt(TablePointer *);
extern void keyboard_wrapper();
extern void syscall_wrapper();

const u32 *cpu_interrupt_table = &cpu_interrupts;

GateDescriptor idt[256];

TablePointer idt_ptr = {sizeof(idt) - 1, idt};

GateDescriptor make_gate_descriptor(u32 handler, u8 type, u8 privilege_level) {
    u64 descriptor = 0;
    // present bit, privilege level, and zero bit
    u8 pdpl0 = 0b1000 | ((privilege_level & 0b11) << 1);

    descriptor |= handler >> 16;
    descriptor <<= 4;
    descriptor |= pdpl0;
    descriptor <<= 4;
    descriptor |= type & 0xF;
    descriptor <<= 24;
    descriptor |= 8;
    descriptor <<= 16; // select first segment
    descriptor |= handler & 0xFFFF;

    return (GateDescriptor) {descriptor};
}

/**
 * @brief Function pointers to all 16 external interrupts
 */
void *irq_routines[16] = {0};

/**
 * @brief Enables Programmable Interrupt Controller
 */
void init_pic() {
    // Spin up chips (ICW1)
    outb(PIC1_COMMAND, 0x11);
    outb(PIC2_COMMAND, 0x11);

    // Sets up interrupt vector table (ICW2)
    outb(PIC1_DATA, 0x20);
    outb(PIC2_DATA, 0x28);

    // Wiring (ICW3)
    outb(PIC1_DATA, 0x04);
    outb(PIC2_DATA, 0x02);

    // Set x86 mode (ICW4)
    outb(PIC1_DATA, 0x01);
    outb(PIC2_DATA, 0x01);

    // Mask all interrupts
    outb(PIC1_DATA, 0xff);
    outb(PIC2_DATA, 0xff);
    asm("sti"); // Enable interrupts
}

/**
 * @brief Sets mask for PIC
 * @param irq The external interrupt
 * @param enable if true enables mask otherwise disables mask
 */
void pic_mask(u8 irq, bool enable) {
    u16 port;
    u16 value;

    if (irq < 8) {
        port = PIC1_DATA;
    }
    else {
        port = PIC2_DATA;
        irq -= 8;
    }

    if (enable)
        value = inb(port) | (1 << irq);
    else
        value = inb(port) & ~(1 << irq);
    outb(port, value);
}

/**
 * @brief
 * @param irq The external interrupt
 */
void pic_eoi(u8 irq) {
    if (irq > 8)
        outb(PIC2_COMMAND, PIC_EOI);
    outb(PIC1_COMMAND, PIC_EOI);
}

/**
 * @brief Initialize the interrupt descriptor table
 */
void init_idt() {
    for (u32 i = 0; i < 256; i++)
        idt[i] = (GateDescriptor) {0};

    init_pic();

    for (u32 i = 0; i < 32; ++i) {
        idt[i] = make_gate_descriptor(
            (u32) cpu_interrupt_table[i], idtgt_Trap, dpl_Kernel
        );
    }

    for (u32 i = 32; i < 48; i++) {
        idt[i] = make_gate_descriptor(
            (u32) cpu_interrupt_table[i], idtgt_Int, dpl_Kernel
        );
    }
    idt[0x80] =
        make_gate_descriptor((u32) syscall_wrapper, idtgt_Int, dpl_User);

    load_idt(&idt_ptr);
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

/**
 * @brief Sets irq routine for an irq to a function pointer
 * @param irq The external interrupt
 * @param handler Pointer to handler function
 */
void irq_install_handler(u32 irq, void (*handler)(InterruptRegisters *reg)) {
    irq_routines[irq] = handler;
    pic_mask(irq, false);
}

/**
 * @brief Resets irq routine for an irq
 * @param irq The external interrupt
 */
void irq_uninstall_handler(u32 irq) {
    pic_mask(irq, true);
    irq_routines[irq] = 0;
}

/**
 * @brief Handles external interrupt with irq routine function
 * @param regs Interrupt registers
 */
void irq_handler(InterruptRegisters *regs) {
    void (*handler)(InterruptRegisters *);

    u8 irq = regs->int_no - 32;

    handler = irq_routines[irq];

    if (handler)
        handler(regs);
    pic_eoi(irq);
}

/**
 * @brief Handles software interrupts. Routes to irq if external.
 * @param regs Interrupt registers
 */
void isr_handler(InterruptRegisters *regs) {
    u32 interrupt = regs->int_no;
    KERNEL_ASSERT(interrupt < 48);
    if (interrupt >= 32)
        irq_handler(regs);
    else {
        terminal_printf("[INT] %s\n", INTERRUPT_NAMES[interrupt]);
        kernel_panic();
    }
}
