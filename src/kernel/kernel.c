#include "terminal/terminal.h"
#include "boot/multiboot.h"
#include "init.h"
#include "lib/error.h"
#include "memory/mem.h"
#include "process/processes.h"
#include "tests/testing.h"

extern const u32 *multiboot_info;
extern const u32 *stack_top;
extern const u32 *_start;

extern DescriptorEntry gdt[3];
extern DescriptorEntry idt[256];
extern Tss initial_tss;


void kernel_main(void) {
    MultibootInfo *mb_info = (MultibootInfo *) multiboot_info;
    u32 mb_flags = mb_info->flags;

    if (!(mb_info->flags & MB_FLAG_FRAMEBUFFER)) {
        // No terminal info is available. Something went wrong and there's no
        // way to report it.
        kernel_panic();
    }

    terminal_init(
        mb_info->framebuffer_width,
        mb_info->framebuffer_height,
        (u16 *) mb_info->framebuffer_addr_lo
    );

    KERNEL_ASSERT(mb_info->framebuffer_addr_hi == 0);


    terminal_printf("Hello, Paradise!\n");

    terminal_printf("Multiboot flags: %b\n", mb_flags);

    terminal_printf("Initializing GDT\n");
    init_gdt();
    terminal_printf("GDT Initialized\n");

    terminal_printf("Initializing IDT\n");
    init_idt();
    terminal_printf("IDT Initialized\n");

    init_page_structures();
    map_pages(0, 0, 256); // identity map lower memory
    map_pages(0x200000, 0x200000, 256); // identity map our kernel code and data
    enable_paging();
    terminal_printf("Enabled paging\n");

    terminal_printf("Loading Initial TSS into GDT\n");
    u32 eax = load_tss(&initial_tss);
    terminal_printf("Loaded TSS\n");
    terminal_printf("EAX: %u\n", eax);

#ifdef TESTS_ENABLED // Test Flag should be passed to build script
    kernel_test(mb_info);
#endif

    for (;;) {
        asm ("hlt");
    }
}
