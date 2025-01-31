#include "kernel.h"
#include "terminal/terminal.h"
#include "boot/multiboot.h"
#include "init.h"
#include "lib/error.h"
#include "memory/mem.h"
#include "process/processes.h"
#include "tests/testing.h"
#include "drivers/serial/io.h"

extern const u32 *multiboot_info;

void usermode_function() {
    terminal_printf("Usermode!\n");
    terminal_printf("CPL: %u\n", get_privilege_level());
    asm ("int $0x80");
    for (;;) {
        asm ("nop");
    }
}

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

    terminal_printf("Initializing TSS\n");
    init_tss();
    terminal_printf("TSS Initialized\n");

    init_page_structures();
    map_pages(0, 0, 256); // identity map lower memory
    map_pages(0x200000, 0x200000, 256); // identity map our kernel code and data
    map_pages(0x600000, 0x600000, 256); // identity map our kernel code and data
    enable_paging();
    terminal_printf("Enabled paging\n");

    serial_init();

#ifdef TESTS_ENABLED // Test Flag should be passed to build script
    kernel_test(mb_info);
#endif

    terminal_printf("CPL: %u\n", get_privilege_level());
    jump_usermode(usermode_function, (u32 *) 0x680000);

    for (;;) {
        asm ("hlt");
    }
}
