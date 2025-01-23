/*ParadiseOS            Initialization
 *This file contains the initialization structures and constants.
 *These constants are specifically used in the IDT and the GDT.
 */

#ifndef INIT_H_
#define INIT_H_

#include "process/processes.h"

#define GET_FIRST_BYTE(x)     (x & 0xFF)
#define GET_SECOND_BYTE(x)    ((x >>  8)  & 0xFF) 
#define GET_THIRD_BYTE(x)     ((x >> 16)  & 0xFF)
#define GET_FOURTH_BYTE(x)    ((x >> 24 ) & 0xFF)
#define GET_LOWER_WORD(x)     (x & 0xFFFF)
#define GET_UPPER_LIMIT(x)    ((x >> 16) & 0xF)

/* Used for FLAG values for the segment descriptor stored in the GDT.
 *    -4k Granularity         : The segment limit is (4096 * limit) bytes.
 *    -Byte Granularity       : The segment limit is (limit) bytes.
 *    -32 Bit stack operations: Stack operations work on 32 bits (push, pop etc)
 *    -16 Bit stack operations: Stack operations work on 16 bits (push, pop etc)
 */
typedef enum {
    gdt_4K32   = 12, //4k Granularity, 32 bit stack operations.
    gdt_4K16   =  8, //4k Granularity, 16 bit stack operations.
    gdt_BYTE32 =  4, //Byte Granularity, 32 bit stack operations.
    gdt_BYTE16 =  0, //Byte Granularity, 16 bit stack operations.
} GDT_FLAGS;

/* Describes the permission and type of the segment descriptor
 *    -KERNEL   : Permission level 0 (Most priveledged mode of execution)
 *    -USER     : Permission level 3 (Least priveledged mode of execution)
 *    -SYSTEM   : System descriptor entry (Call Gate, TSS Entry, etc) described more below.
 *    -CODE_DATA: Descriptor points to a code or data segment.
 */
typedef enum {
    gdt_KERNEL_SYSTEM    =  8, //Kernel Level System Descriptor
    gdt_KERNEL_CODE_DATA =  9, //Kernel Level Code/Data Descriptor
    gdt_USER_SYSTEM      = 14, //User Level System Descriptor
    gdt_USER_CODE_DATA   = 15, //User Level Code/Data Descriptor
} GDT_PRIVLEDGE_TYPE;

/* Describes the specific type of a code / data segment. The
 * firt 8 pertian to data segments, the last 8 code segments.
 *    -R/RO: Read, the segment is readable (RO for read only).
 *    -A   : Accessed, this bit specifies whether the segment
 *           has been accessed since the last time the bit was
 *           cleared.
 *    -W   : Write, the segment is writable.
 *    -E   : The segment is expand down.
 *    -X   : Executable, the segment is executable.
 *    -C   : Conforming, the segment is conforming, 
 *           accessible only by less than or equal to
 *           privledges (without call gates), and preserves
 *           current CPL when jumped/called into.
 */
typedef enum {
    gdt_RO   =  0, //Data Read Only.
    gdt_ROA  =  1, //Data Read Only Accessed.
    gdt_RW   =  2, //Data Read Write.
    gdt_RWA  =  3, //Data Read Write Accessed.
    gdt_ROE  =  4, //Data Read Only Expand Down.
    gdt_ROEA =  5, //Data Read Only Expand Down Accessed.
    gdt_RWE  =  6, //Data Rewad Write Expand Down.
    gdt_RWEA =  7, //Data Rewad Write Expand Down Accessed.
    gdt_X    =  8, //Code Executable.
    gdt_XA   =  9, //Code Executable Accessed.
    gdt_XR   = 10, //Code Executable Readable.
    gdt_XRA  = 11, //Code Executable Readable Accessed.
    gdt_XC   = 12, //Code Executable Conforming.
    gdt_XCA  = 13, //Code ExecutableConforming Accessed.
    gdt_XRC  = 14, //Code Executable Readable Conforming.
    gdt_XRCA = 15, //Code Executable Readable Conforming Accessed.
} GDT_TYPES;

/* Describes the specific type of a system segment
 *    -TSS : Task State Segment.
 *    -LDT : Local Descriptor Table.
 *    -CALL: Call Gate.
 *    -TASK: Task Get.
 *    -TRAP: Trap Gate.
 *    -INT : Interrupt Gate.
 *    -A   : Available (Ready to be executed).
 *    -B   : Busy      (Currently Executing or waiting).
 *    -16  : 16 bit segment.
 *    -32  : 32 bit segment.
 */
typedef enum {
    gdts_TSS16A   =  1, //Task State Segment 16 Bits Available.
    gdts_LDT      =  2, //Local Descriptor Table.
    gdts_TSS16B   =  3, //Task State Segment 16 Bits Busy.
    gdts_CALL16   =  4, //Call Gate 16 Bits.
    gdts_TASK     =  5, //Task Gate.
    gdts_INT16    =  6, //Interrupt Gate 16 Bits.
    gdts_TRAP16   =  7, //Trap Gate 16 Bits.
    gdts_TSS32A   =  9, //Task State Segment 32 Bits Available.
    gdts_TSS32B   = 11, //Task State Segment 32 Bits Busy.
    gdts_CALL32   = 12, //Call Gate 32 Bits.
    gdts_INT32    = 14, //Interrupt Gate 32 Bits.
    gdts_TRAP32   = 15, //Trap Gate 32 Bits.
} GDT_SYSTEM_TYPES;

/*Types of entries in the IDT (Interrupt Descriptor Table).
 *   -TASK: Task Gate.
 *   -INT : Interrupt Gate.
 *   -TRAP:Trap Gate
 */
typedef enum {
    idt_TASK =  5, //Task Gate.
    idt_INT  = 14, //Interrupt Gate.
    idt_TRAP = 15, //Trap Gate
} IDT_TYPES;

/*Generalized Table Entry
 * This table structure is used in multiple processor 
 * tables, mainly the IDT and the GDT. These tables
 * are 8 bytes in size. They must be packed as the
 * layout must be exactly as descibed by the structure.
 * For preformance reasons they should also be aligned
 * to 8 Byte boundaries.
 */
typedef struct __attribute__((packed, aligned(8))) {
    u16 entry1;
    u16 entry2;
    u8  entry3;
    u8  entry4;
    u8  entry5; 
    u8  entry6;
    
} DescriptorEntry;

/*Consists of a pointer to the generalized table.
 * These pointers only need to store the size of the 
 * table and the pointer to the first entry in the table.
 * This pointer will later be stored in a specialized CPU register
 * through instructions such as LGDT, LIDT, etc.
 */
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
