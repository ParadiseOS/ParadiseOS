#include "types.h"
#include "init.h"

extern u32 cpu_interrupts;

const u32 *cpu_interrupt_table = &cpu_interrupts;

DescriptorEntry gdt[3];
TablePointer p_gdt = {sizeof (gdt) - 1, gdt};

DescriptorEntry idt[256];
TablePointer p_idt = {sizeof (idt) - 1, idt};

extern void load_gdt(TablePointer *);
extern void load_idt(TablePointer *);

void set_entry_gdt(DescriptorEntry *entry, u32 base, u32 limit, u8 type, u8 flags) {
    entry->entry1 = GET_LOWER_WORD(limit);
    entry->entry2 = GET_LOWER_WORD(base);
    entry->entry3 = GET_THIRD_BYTE(base);
    entry->entry4 = type;
    entry->entry5 = (flags << 4) | GET_UPPER_LIMIT(limit);
    entry->entry6 = GET_FOURTH_BYTE(base);
}

void set_entry_idt(DescriptorEntry* entry, u32  handler, u8 type) {
    entry->entry1 = GET_LOWER_WORD(handler);
    entry->entry2 = 0x8;
    entry->entry4 = type | (1 << 7);
    entry->entry5 = GET_THIRD_BYTE(handler);
    entry->entry6 = GET_FOURTH_BYTE(handler);
}

void set_entry(DescriptorEntry * entry, u16 entry1, u16 entry2, 
               u8 entry3, u8 entry4, u8 entry5, u8 entry6) {
    entry->entry1 = entry1;
    entry->entry2 = entry2;
    entry->entry3 = entry3;
    entry->entry4 = entry4;
    entry->entry5 = entry5;
    entry->entry6 = entry6;
}

void init_gdt() {
    //Set null descriptor & zero out memory
    for (int i = 0; i < 3; i++) set_entry_gdt(&gdt[i], 0, 0,  0, 0);
    //Code Descriptor
    set_entry_gdt(&gdt[1], 0, 0xFFFFFFFF, SEG_CODE_USER, FLAG_4k);
    //Data Descriptor
    set_entry_gdt(&gdt[2], 0, 0xFFFFFFFF, SEG_DATA_USER, FLAG_4k);
    //Load gdt into gdtr and flush previous segment registers
    load_gdt(&p_gdt);
}

void init_idt() {
   for(int i = 0; i < 256; i++) set_entry(&idt[i], 0, 0, 0, 0, 0, 0);

   for (int i = 0; i < 32; ++i) {
       set_entry_idt(&idt[i], cpu_interrupt_table[i], idt_TRAP);
   }

   load_idt(&p_idt);
}

