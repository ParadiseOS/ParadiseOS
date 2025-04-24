#include "processes.h"
#include "kernel/kernel.h"
#include "lib/error.h"
#include "memory/mem.h"
#include "sun/sun.h"

#define PROCESS_ORG ((void *) 0x401000)
#define STACK_TOP ((void *) 0xbfc00000)
#define STACK_SIZE (4 * PAGE_SIZE)

void exec_sun(const char *name) {
    TableEntry *entry = sun_exe_lookup(name);

    KERNEL_ASSERT(entry->text_size);

    ProcessControlBlock *pcb = PROCESS_ORG - PAGE_SIZE;
    void *text = PROCESS_ORG;
    void *rodata = align_next_page(text + entry->text_size);
    void *data = align_next_page(rodata + entry->rodata_size);
    void *bss = align_next_page(data + entry->data_size);
    void *heap = align_next_page(data + entry->bss_size);
    void *stack = STACK_TOP;

    u32 page_dir = new_page_dir();
    load_page_dir(page_dir);

    alloc_pages(text, PAGE_WRITABLE | PAGE_USER_MODE, (rodata - text) / PAGE_SIZE);
    if (entry->rodata_size) alloc_pages(rodata, PAGE_WRITABLE | PAGE_USER_MODE, (data - rodata) / PAGE_SIZE);
    if (entry->data_size) alloc_pages(data, PAGE_WRITABLE | PAGE_USER_MODE, (bss - data) / PAGE_SIZE);
    if (entry->bss_size) alloc_pages(bss, PAGE_WRITABLE | PAGE_USER_MODE, (heap - bss) / PAGE_SIZE);
    alloc_pages(stack - STACK_SIZE, PAGE_WRITABLE | PAGE_USER_MODE, STACK_SIZE / PAGE_SIZE);
    alloc_pages(pcb, PAGE_WRITABLE, 1);

    sun_load_text(entry, text);
    sun_load_rodata(entry, rodata);
    sun_load_data(entry, data);

    pcb->prog_brk = heap;

    jump_usermode(entry->entry_point, stack);
}
