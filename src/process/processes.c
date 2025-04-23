#include "processes.h"
#include "memory/mem.h"
#include "sun/sun.h"

void exec(const char *name) {
    TableEntry *entry = sun_exe_lookup(name);

    u32 stack_pages = 1;
    u8 *stack_bottom = kernel_alloc(stack_pages);
    u8 *stack = stack_bottom + stack_pages * PAGE_SIZE;

    u8 *text = kernel_alloc(1);

    sun_load_text(entry, text);

    jump_usermode((void (*)()) text, stack);
}
