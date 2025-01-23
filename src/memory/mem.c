#include "mem.h"
#include "lib/error.h"

extern u32 page_directory_start;
extern u32 page_tables_start;

u32 *page_directory = &page_directory_start;
u32 *page_tables = &page_tables_start;

#define NUM_TABLES 12

#define PAGE_DIRECTORY_FLAGS 0b000000000111
#define PAGE_TABLE_FLAGS 0b000000000111

u32 current_table = 0;

void init_page_structures() {
    for (u32 i = 0; i < 1024; ++i) {
        page_directory[i] = 0; // the present flag gets cleared

        for (u32 table = 0; table < NUM_TABLES; ++table) {
            page_tables[table * 1024 + i] = 0;
        }
    }
}

// an extremely simplistic paging approach
void map_page(u32 physical_addr, u32 virtual_addr) {
    u32 directory_offset = (virtual_addr >> 22) & 0x3ff;
    u32 table_offset = (virtual_addr >> 12) & 0x3ff;

    if (!page_directory[directory_offset]) {
        if (current_table >= NUM_TABLES)
            KERNEL_ASSERT(0); // we ran out of tables so just give up

        page_directory[directory_offset] =
            (u32) &page_tables[current_table * 1024] | PAGE_DIRECTORY_FLAGS;
        current_table += 1;
    }

    u32 *table = (u32 *) (page_directory[directory_offset] & 0xFFFFF000);

    if (table[table_offset])
        KERNEL_ASSERT(0);

    table[table_offset] =
        (physical_addr & 0xFFFFF000) | PAGE_TABLE_FLAGS;
}

void map_pages(u32 physical_addr, u32 virtual_addr, u32 count) {
    while (count--) {
        map_page(physical_addr, virtual_addr);
        physical_addr += 4096;
        virtual_addr += 4096;
    }
}
