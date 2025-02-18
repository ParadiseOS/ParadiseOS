#ifndef MEM_H_
#define MEM_H_

#include "lib/types.h"
#include "boot/multiboot.h"

extern void enable_paging();

void init_paging();
void init_frames();

void map_pages_physical(u32 physical_addr, u32 virtual_addr, u32 count);

#endif // MEM_H_
