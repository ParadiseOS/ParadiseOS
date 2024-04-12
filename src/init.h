/*Contains all structures/headers related to the initialization of the OS:
 *  -GDT
 */

//Pointer to the gdt initialized in boot.s
#ifndef INIT_H_
#define INIT_H_


#define GET_FIRST_BYTE(x)     (x & 0xFF)
#define GET_SECOND_BYTE(x)    ((x >>  8)  & 0xFF) 
#define GET_THIRD_BYTE(x)     ((x >> 16)  & 0xFF)
#define GET_FOURTH_BYTE(x)    ((x >> 24 ) & 0xFF)
#define GET_LOWER_WORD(x)     (x & 0xFFFF)
#define GET_UPPER_LIMIT(x)    ((x >> 16) & 0xF)

typedef enum {
    gdt_4K32   = 12,
    gdt_4K16   =  8,
    gdt_BYTE32 =  4,
    gdt_BYTE16 =  0,
} GDT_FLAGS;

typedef enum {
    gdt_KERNEL_SYSTEM    =  8,
    gdt_KERNEL_CODE_DATA =  9,
    gdt_USER_SYSTEM      = 14,
    gdt_USER_CODE_DATA   = 15,
} GDT_PRIVLEDGE_TYPE;

typedef enum {
    gdt_RO   =  0,
    gdt_ROA  =  1,
    gdt_RW   =  2,
    gdt_RWA  =  3,
    gdt_ROE  =  4,
    gdt_ROEA =  5,
    gdt_RWE  =  6,
    gdt_RWEA =  7,
    gdt_X    =  8,
    gdt_XA   =  9,
    gdt_XR   = 10,
    gdt_XRA  = 11,
    gdt_XC   = 12,
    gdt_XCA  = 13,
    gdt_XRC  = 14,
    gdt_XRCA = 15,
} GDT_TYPES;

typedef enum {
    gdts_TSS16A   =  1,
    gdts_LDT      =  2,
    gdts_TSS16B   =  3,
    gdts_CALL16   =  4,
    gdts_TASK     =  5,
    gdts_INT16    =  6,
    gdts_TRAP16   =  7,
    gdts_TSS32A   =  9,
    gdts_TSS32B   = 11,
    gdts_CALL32   = 12,
    gdts_INT32    = 14,
    gdts_TRAP32   = 15,
} GDT_SYSTEM_TYPES;

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

