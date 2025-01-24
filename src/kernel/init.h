#ifndef INIT_H_
#define INIT_H_


#include "lib/types.h"

/* ParadiseOS            Initialization
 * This file contains the initialization structures and constants.
 * These constants are specifically used in the IDT and the GDT.
 */

/* Used for FLAG values for the segment descriptor stored in the GDT.
 *    - 4k Granularity:          The segment limit is (4096 * limit) bytes
 *    - Byte Granularity:        The segment limit is (limit) bytes
 *    - 32 Bit stack operations: Stack operations work on 32 bits (push, pop etc)
 *    - 16 Bit stack operations: Stack operations work on 16 bits (push, pop etc)
 */
typedef enum {
    gdtg_Page32   = 12, // Page Granularity, 32 bit stack operations
    gdtg_Page16   =  8, // Page Granularity, 16 bit stack operations
    gdtg_Byte32   =  4, // Byte Granularity, 32 bit stack operations
    gdtg_Byte16   =  0, // Byte Granularity, 16 bit stack operations
} GdtGranularity;

/* Describes the permission and type of the segment descriptor.
 *    - KERNEL:    Permission level 0 (Most priveledged mode of execution)
 *    - USER:      Permission level 3 (Least priveledged mode of execution)
 *    - SYSTEM:    System descriptor entry (Call Gate, TSS Entry, etc) described more below
 *    - CODE_DATA: Descriptor points to a code or data segment
 */
typedef enum {
    gdtpt_KernelSystem    =  8, // Kernel Level System Descriptor
    gdtpt_KernelCodeData  =  9, // Kernel Level Code/Data Descriptor
    gdtpt_UserSystem      = 14, // User Level System Descriptor
    gdtpt_UserCodeData    = 15, // User Level Code/Data Descriptor
} GdtPrivilegeType;

/* Describes the specific type of a code / data segment. The
 * firt 8 pertian to data segments, the last 8 code segments.
 *    - R/RO: Read, the segment is readable (RO for read only).
 *    - A:    Accessed, this bit specifies whether the segment
 *            has been accessed since the last time the bit was
 *            cleared.
 *    - W:    Write, the segment is writable.
 *    - E:    The segment is expand down.
 *    - X:    Executable, the segment is executable.
 *    - C:    Conforming, the segment is conforming,
 *            accessible only by less than or equal to
 *            privledges (without call gates), and preserves
 *            current CPL when jumped/called into.
 */
typedef enum {
    gdtseg_RO   =  0, // Data Read Only
    gdtseg_ROA  =  1, // Data Read Only Accessed
    gdtseg_RW   =  2, // Data Read Write
    gdtseg_RWA  =  3, // Data Read Write Accessed
    gdtseg_ROE  =  4, // Data Read Only Expand Down
    gdtseg_ROEA =  5, // Data Read Only Expand Down Accessed
    gdtseg_RWE  =  6, // Data Read Write Expand Down
    gdtseg_RWEA =  7, // Data Read Write Expand Down Accessed
    gdtseg_X    =  8, // Code Executable
    gdtseg_XA   =  9, // Code Executable Accessed
    gdtseg_XR   = 10, // Code Executable Readable
    gdtseg_XRA  = 11, // Code Executable Readable Accessed
    gdtseg_XC   = 12, // Code Executable Conforming
    gdtseg_XCA  = 13, // Code Executable Conforming Accessed
    gdtseg_XRC  = 14, // Code Executable Readable Conforming
    gdtseg_XRCA = 15, // Code Executable Readable Conforming Accessed
} GdtSegmentType;

/* Describes the specific type of a system segment
 *    - TSS:  Task State Segment
 *    - LDT:  Local Descriptor Table
 *    - CALL: Call Gate
 *    - TASK: Task Get
 *    - TRAP: Trap Gate
 *    - INT:  Interrupt Gate
 *    - A:    Available (Ready to be executed)
 *    - B:    Busy      (Currently Executing or waiting)
 *    - 16:   16 bit segment
 *    - 32:   32 bit segment
 */
typedef enum {
    gdtsys_Tss16A   =  1, // Task State Segment 16 Bits Available
    gdtsys_Ldt      =  2, // Local Descriptor Table
    gdtsys_Tss16B   =  3, // Task State Segment 16 Bits Busy
    gdtsys_Call16   =  4, // Call Gate 16 Bits
    gdtsys_Task     =  5, // Task Gate
    gdtsys_Int16    =  6, // Interrupt Gate 16 Bits.
    gdtsys_Trap16   =  7, // Trap Gate 16 Bit
    gdtsys_Tss32A   =  9, // Task State Segment 32 Bits Available
    gdtsys_Tss32B   = 11, // Task State Segment 32 Bits Busy
    gdtsys_Call32   = 12, // Call Gate 32 Bits
    gdtsys_Int32    = 14, // Interrupt Gate 32 Bits
    gdtsys_Trap32   = 15, // Trap Gate 32 Bits
} GdtSystemType;

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

/* Consists of a pointer to the generalized table.
 * These pointers only need to store the size of the 
 * table and the pointer to the first entry in the table.
 * This pointer will later be stored in a specialized CPU register
 * through instructions such as LGDT, LIDT, etc.
 */
typedef struct __attribute__((packed)) {
    u16 limit;
    void *base;
} TablePointer;

typedef struct __attribute__((aligned(8))) { u64 value; } SegmentDescriptor;
typedef struct { u64 value; } GateDescriptor;

void init_gdt();
void init_idt();

#endif
