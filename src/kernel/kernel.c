#include "kernel.h"
#include "terminal/terminal.h"
#include "boot/multiboot.h"
#include "init.h"
#include "interrupts/interrupt.h"
#include "lib/error.h"
#include "memory/mem.h"
#include "process/processes.h"
#include "tests/testing.h"
#include "drivers/timer/timer.h"
#include "drivers/keyboard/keyboard.h"
#include "drivers/serial/io.h"

const u32 kernel_start_paddr = (u32) &_kernel_start_paddr;
const void *kernel_start_vaddr = &_kernel_start_vaddr;
const u32 kernel_end_paddr = (u32) &_kernel_end_paddr;
const void *kernel_end_vaddr = &_kernel_end_vaddr;

void usermode_function() {
    terminal_printf("Usermode!\n");
    terminal_printf("CPL: %u\n", get_privilege_level());
    asm ("int $0x80");
    terminal_printf("CPL: %u\n", get_privilege_level());
    for (;;) {
        asm ("nop");
    }
}

void kernel_main(void) {
    if (!(multiboot_info->flags & MB_FLAG_FRAMEBUFFER)) {
        // No terminal info is available. Something went wrong and there's no
        // way to report it.
        kernel_panic();
    }

    terminal_init(
        multiboot_info->framebuffer_width,
        multiboot_info->framebuffer_height,
        (u16 *) multiboot_info->framebuffer_addr_lo
    );

    KERNEL_ASSERT(multiboot_info->framebuffer_addr_hi == 0);

    terminal_printf("Initializing GDT...\n");
    init_gdt();

    terminal_printf("Initializing IDT...\n");
    init_idt();

    terminal_printf("Initializing TSS...\n");
    init_tss();

    terminal_printf("Initializing memory...\n");
    mem_init();

    terminal_printf("Initializing timer\n");
    init_timer();

    terminal_printf("Initializing keyboard\n");
    init_keyboard();

    terminal_printf("Initializing serial io...\n");
    serial_init();

#ifdef TESTS_ENABLED // Test Flag should be passed to build script
    kernel_test();
#endif

    u32 usermode_stack_pages = 1;
    u8 *usermode_stack_bottom = kernel_alloc(usermode_stack_pages);
    u8 *usermode_stack_top = usermode_stack_bottom + usermode_stack_pages * 4096;

    terminal_printf("CPL: %u\n", get_privilege_level());
    jump_usermode(usermode_function, usermode_stack_top);

    for (;;) {
        asm ("hlt");
    }
}
