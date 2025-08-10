#ifndef INIT_H_
#define INIT_H_

#include "lib/types.h"
#include "memory/mem.h"

// clang-format off
/* Used for FLAG values for the segment descriptor stored in the GDT.
 *    - 4k Granularity:          The segment limit is (4096 * limit) bytes
 *    - Byte Granularity:        The segment limit is (limit) bytes
 *    - 32 Bit stack operations: Stack operations work on 32 bits
 *    - 16 Bit stack operations: Stack operations work on 16 bits
 */
typedef enum {
    gdtg_Page32 = 12,           // Page Granularity, 32 bit stack operations
    gdtg_Page16 = 8,            // Page Granularity, 16 bit stack operations
    gdtg_Byte32 = 4,            // Byte Granularity, 32 bit stack operations
    gdtg_Byte16 = 0,            // Byte Granularity, 16 bit stack operations
} GdtGranularity;

/* Describes the permission and type of the segment descriptor.
 *    - KERNEL:    Permission level 0 (Most priveledged mode of execution)
 *    - USER:      Permission level 3 (Least priveledged mode of execution)
 *    - SYSTEM:    System descriptor entry (Call Gate, TSS Entry, etc)
 *    - CODE_DATA: Descriptor points to a code or data segment
 */
typedef enum {
    gdtpt_KernelSystem   = 8,   // Kernel Level System Descriptor
    gdtpt_KernelCodeData = 9,   // Kernel Level Code/Data Descriptor
    gdtpt_UserSystem     = 14,  // User Level System Descriptor
    gdtpt_UserCodeData   = 15,  // User Level Code/Data Descriptor
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
    gdtseg_RO   = 0,            // Data Read Only
    gdtseg_ROA  = 1,            // Data Read Only Accessed
    gdtseg_RW   = 2,            // Data Read Write
    gdtseg_RWA  = 3,            // Data Read Write Accessed
    gdtseg_ROE  = 4,            // Data Read Only Expand Down
    gdtseg_ROEA = 5,            // Data Read Only Expand Down Accessed
    gdtseg_RWE  = 6,            // Data Read Write Expand Down
    gdtseg_RWEA = 7,            // Data Read Write Expand Down Accessed
    gdtseg_X    = 8,            // Code Executable
    gdtseg_XA   = 9,            // Code Executable Accessed
    gdtseg_XR   = 10,           // Code Executable Readable
    gdtseg_XRA  = 11,           // Code Executable Readable Accessed
    gdtseg_XC   = 12,           // Code Executable Conforming
    gdtseg_XCA  = 13,           // Code Executable Conforming Accessed
    gdtseg_XRC  = 14,           // Code Executable Readable Conforming
    gdtseg_XRCA = 15,           // Code Executable Readable Conforming Accessed
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
    gdtsys_Tss16A = 1,          // Task State Segment 16 Bits Available
    gdtsys_Ldt    = 2,          // Local Descriptor Table
    gdtsys_Tss16B = 3,          // Task State Segment 16 Bits Busy
    gdtsys_Call16 = 4,          // Call Gate 16 Bits
    gdtsys_Task   = 5,          // Task Gate
    gdtsys_Int16  = 6,          // Interrupt Gate 16 Bits.
    gdtsys_Trap16 = 7,          // Trap Gate 16 Bit
    gdtsys_Tss32A = 9,          // Task State Segment 32 Bits Available
    gdtsys_Tss32B = 11,         // Task State Segment 32 Bits Busy
    gdtsys_Call32 = 12,         // Call Gate 32 Bits
    gdtsys_Int32  = 14,         // Interrupt Gate 32 Bits
    gdtsys_Trap32 = 15,         // Trap Gate 32 Bits
} GdtSystemType;
// clang-format on

// NOTE: OS Dev wiki documents this to be 104 bytes even though it seems to
// actually be 108.
#define TSS_SIZE 104

typedef struct __attribute__((packed, aligned(PAGE_SIZE))) {
    u16 prev_task_link;
    u16 reserved1;
    u32 esp0;
    u16 ss0;
    u16 reserved2;
    u32 esp1;
    u16 ss1;
    u16 reserved3;
    u32 esp2;
    u16 ss2;
    u16 reserved4;
    u32 page_directory;
    u32 eip;
    u32 eflags;
    u32 eax;
    u32 ecx;
    u32 edx;
    u32 ebx;
    u32 esp;
    u32 ebp;
    u32 esi;
    u32 edi;
    u16 es;
    u16 reserved5;
    u16 cs;
    u16 reserved6;
    u16 ss;
    u16 reserved7;
    u16 ds;
    u16 reserved8;
    u16 fs;
    u16 reserved9;
    u16 gs;
    u16 reserved10;
    u16 ldt;
    u16 reserved11;
    u16 trap;
    u16 io_base;
    u32 ssp;
} Tss;

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

typedef struct __attribute__((aligned(8))) {
    u64 value;
} SegmentDescriptor;

void init_gdt();
void init_tss();

extern void init_fpu();
extern u8 get_privilege_level();

#endif
