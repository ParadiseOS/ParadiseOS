#ifndef MEM_H_
#define MEM_H_

#include "lib/types.h"

extern void enable_paging();

void init_page_structures();
void map_pages(u32 physical_addr, u32 virtual_addr, u32 count);

#endif // MEM_H_
