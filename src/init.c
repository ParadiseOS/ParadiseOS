#include "types.h"
#include "init.h"

 GdtEntry gdt[3];
 GdtPointer p_gdt = {sizeof(gdt)-1, gdt};


void set_entry(GdtEntry *entry, u32 base, u32 limit, u8 type, u8 flags){
    entry->limit      = GET_LOWER_WORD(limit);
    entry->base_low   = GET_LOWER_WORD(base);
    entry->base_mid   = GET_MID_BASE(base);
    entry->type       = type;
    entry->flags      = (flags << 4) | GET_UPPER_LIMIT(limit);
    entry->base_upper = GET_UPPER_BASE(base);
}

void init_gdt(){
    //Set null descriptor & zero out memory
    for (int i=0; i<3; i++){
        set_entry(&gdt[i], 0, 0,  0, 0);
    }

    //Code Descriptor
    set_entry(&gdt[1], 0, 0xFFFFF, SEG_CODE_USER, FLAG_4k);
    //Data Descriptor
    set_entry(&gdt[2], 0, 0xFFFFF, SEG_DATA_USER, FLAG_4k);
    //Load gdt into gdtr and flush previous segment registers
    load_gdt(p_gdt);
}

