/*Contains all structures/headers related to the initialization of the OS:
 *  -GDT
 */

//Pointer to the gdt initialized in boot.s
#ifndef INIT_H_
#define INIT_H_

#define SEG_DATA_USER     0x92
#define SEG_CODE_USER     0x9E
#define FLAG_4k           0xC

#define GET_THIRD_BYTE(x)     ((x >>16)  & 0xFF)
#define GET_FOURTH_BYTE(x)    ((x >>24)  & 0xFF)
#define GET_LOWER_WORD(x)     (x & 0xFFFF)
#define GET_UPPER_BASE(x)     ((x >> 24) & 0xFF)
#define GET_UPPER_LIMIT(x)    ((x >> 16) & 0xF)

typedef enum {
    idt_TASK =  5,
    idt_INT  = 14,
    idt_TRAP = 15,
} IDT_TYPES;

typedef struct __attribute__((packed, aligned(8))) {
    u16 entry1;
    u16 entry2;
    u8  entry3;
    u8  entry4;
    u8  entry5; 
    u8  entry6;
    
} DescriptorEntry;

typedef struct __attribute__((packed)) {
    u16 limit;
    DescriptorEntry *base;
} TablePointer;


void set_entry_gdt(DescriptorEntry *entry, u32 base, u32 limit, u8 type, u8 flags);
void set_entry(DescriptorEntry * entry, u16 entry1, u16 entry2, 
               u8 entry3, u8 entry4, u8 entry5, u8 entry6);
void init_gdt();
void init_idt();

#endif

