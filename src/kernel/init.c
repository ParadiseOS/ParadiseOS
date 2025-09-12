#include "init.h"
#include "kernel/kernel.h"
#include "lib/error.h"
#include "lib/libp.h"
#include "lib/types.h"
#include "process/processes.h"
#include "terminal/terminal.h"

extern void load_gdt(TablePointer *);
extern void load_tss(u32 gdt_index);
extern u32 get_eflags();

SegmentDescriptor gdt[128];

TablePointer gdt_ptr = {sizeof(gdt) - 1, gdt};

Tss initial_tss;

SegmentDescriptor
make_segment_descriptor(u32 base, u32 limit, u8 type, u8 flags) {
    u64 descriptor = 0;

    descriptor |= base >> 24;
    descriptor <<= 4;
    descriptor |= flags & 0xF;
    descriptor <<= 4;
    descriptor |= (limit >> 16) & 0xF;
    descriptor <<= 8;
    descriptor |= type;
    descriptor <<= 24;
    descriptor |= base & 0xFFFFFF;
    descriptor <<= 16;
    descriptor |= limit & 0xFFFF;

    return (SegmentDescriptor) {descriptor};
}

#define SEGMENT(type, flags) make_segment_descriptor(0, 0xFFFFFFFF, type, flags)

void init_gdt() {
    for (int i = 0; i < 128; i++)
        gdt[i] = make_segment_descriptor(0, 0, 0, 0);

    gdt[1] = SEGMENT((gdtpt_KernelCodeData << 4) | gdtseg_XR, gdtg_Page32);
    gdt[2] = SEGMENT((gdtpt_KernelCodeData << 4) | gdtseg_RW, gdtg_Page32);
    gdt[3] = SEGMENT((gdtpt_UserCodeData << 4) | gdtseg_XR, gdtg_Page32);
    gdt[4] = SEGMENT((gdtpt_UserCodeData << 4) | gdtseg_RW, gdtg_Page32);
    gdt[5] = make_segment_descriptor(
        (u32) &initial_tss, TSS_SIZE - 1,
        (gdtpt_KernelSystem << 4) | gdtsys_Tss32A, gdtg_Byte16
    );

    load_gdt(&gdt_ptr);
}

void init_tss() {
    pmemset(&initial_tss, 0, sizeof(initial_tss));

    initial_tss.ss0 = 0x10; // kernel data GDT entry
    initial_tss.esp0 = (u32) kernel_stack;
    initial_tss.io_base = TSS_SIZE;
    // interrupts should be off at this point so this ensures that they are
    // never enabled when re-entering the kernel
    initial_tss.eflags = get_eflags();

    load_tss(0x28); // TSS GDT entry
}
