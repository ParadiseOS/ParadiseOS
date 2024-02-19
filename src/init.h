/*Contains all structures/headers related to the initialization of the OS:
 *  -GDT
 */

//Pointer to the gdt initialized in boot.s
#ifndef INIT_H_
#define INIT_H_

#define SEG_DATA_USER     0xF2
#define SEG_CODE_USER     0xFA
#define FLAG_4k           0xC

#define GET_LOWER_WORD(x)     (x & 0xFFFF)
#define GET_MID_BASE(x)       ((x >> 16) & 0xFF)
#define GET_UPPER_BASE(x)     ((x >> 24) & 0xFF)
#define GET_UPPER_LIMIT(x)    ((x >> 16) & 0xF)

/* FLAG BITS:
 * FLAG[0:3] = Type
 * FLAG[4]   = S (code=0/data=1)
 * FLAG[5:6] = DPL (Privelage)
 * FLAG[7]   = Present
 * FLAG[8]   = Available
 * FLAG[9]   = 0
 * FLAG[10]  = Default Operation Size
 * FLAG[11]  = Granularity */

typedef struct __attribute__((packed)) {
    u16 limit;
    u16 base_low;
    u8  base_mid;
    u8  type;
    u8  flags; 
    u8  base_upper;
    
} GdtEntry;

typedef struct __attribute__((packed)) {
    u16 limit;
    void *base;
} GdtPointer;


void set_entry(GdtEntry *entry, u32 base, u32 limit, u8 type, u8 flags);
void init_gdt();
void load_gdt(GdtPointer);

#endif

