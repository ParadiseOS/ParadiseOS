#include "lib/types.h"
#include "terminal/terminal.h"
#include "process/processes.h"
#include "init.h"
#include "lib/error.h"

extern u32 cpu_interrupts;

extern void load_gdt(TablePointer *);
extern void load_idt(TablePointer *);
extern void syscall_wrapper();

const u32 *cpu_interrupt_table = &cpu_interrupts;

SegmentDescriptor gdt[128];
GateDescriptor idt[256];

TablePointer gdt_ptr = { sizeof (gdt) - 1, gdt };
TablePointer idt_ptr = { sizeof (idt) - 1, idt };

Tss initial_tss;

// || base[31:24] || flags || limit[19:16] || access byte || base[23:16] || base[15:0] || limit[15:0] ||
// || 63       56 || 55 52 || 51        48 || 47       40 || 39       32 || 31      16 || 15        0 ||
SegmentDescriptor make_segment_descriptor(u32 base, u32 limit, u8 type, u8 flags) {
    u64 descriptor = 0;

    descriptor |= base >> 24;          descriptor <<= 4;
    descriptor |= flags & 0xF;         descriptor <<= 4;
    descriptor |= (limit >> 16) & 0xF; descriptor <<= 8;
    descriptor |= type;                descriptor <<= 24;
    descriptor |= base & 0xFFFFFF;     descriptor <<= 16;
    descriptor |= limit & 0xFFFF;

    return (SegmentDescriptor) { descriptor };
}

// || handler[31:16] || present || privilege level || zero || gate type || reserved || segment selector || handler[15:0] ||
// || 63          48 || 47      || 46           45 || 44   || 43     40 || 39    32 || 31            16 || 15          0 ||
GateDescriptor make_gate_descriptor(u32 handler, u8 type) {
    u64 descriptor = 0;

    descriptor |= handler >> 16; descriptor <<= 4;
    descriptor |= 0b1000;        descriptor <<= 4;  // present bit, privilege level, and zero bit
    descriptor |= type & 0xF;    descriptor <<= 24;
    descriptor |= 8;             descriptor <<= 16; // select first segment
    descriptor |= handler & 0xFFFF;

    return (GateDescriptor) { descriptor };
}

void init_gdt() {
    for (int i = 0; i < 128; i++) gdt[i] = make_segment_descriptor(0, 0, 0, 0);
    gdt[1] = make_segment_descriptor(0, 0xFFFFFFFF, (gdtpt_KernelCodeData << 4) | gdtseg_XR, gdtg_Page32);
    gdt[2] = make_segment_descriptor(0, 0xFFFFFFFF, (gdtpt_KernelCodeData << 4) | gdtseg_RW, gdtg_Page32);
    gdt[3] = make_segment_descriptor(0, 0xFFFFFFFF, (gdtpt_UserCodeData   << 4) | gdtseg_XR, gdtg_Page32);
    gdt[4] = make_segment_descriptor(0, 0xFFFFFFFF, (gdtpt_UserCodeData   << 4) | gdtseg_RW, gdtg_Page32);
    gdt[5] = make_segment_descriptor((u32) &initial_tss, 0x67, (gdtpt_KernelSystem << 4) | gdtsys_Tss32A, gdtg_Page16);
    load_gdt(&gdt_ptr);
}

void syscall_handler() {
    terminal_printf("syscalled!\n");
}

void init_idt() {
    for(int i = 0; i < 256; i++) idt[i] = (GateDescriptor) { 0 };

    for (int i = 0; i < 32; ++i) {
        idt[i] = make_gate_descriptor(cpu_interrupt_table[i], idtgt_Trap);
    }

    idt[0x80] = make_gate_descriptor((u32) syscall_wrapper, idtgt_Int);

    load_idt(&idt_ptr);
}
