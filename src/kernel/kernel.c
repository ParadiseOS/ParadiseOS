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

const void *kernel_start_addr = &_kernel_start;
const void *kernel_end_addr = &_kernel_end;

u8 usermode_stack[1024];

void usermode_function() {
    terminal_printf("Usermode!\n");
    terminal_printf("CPL: %u\n", get_privilege_level());
    asm ("int $0x80");
    for (;;) {
        asm ("nop");
    }
}

void kernel_main(void) {
    u32 kernel_size = (u32) kernel_end_addr - (u32) kernel_start_addr;

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
    terminal_printf("Initializing frames...\n");
    init_frames();

    terminal_printf("Initializing paging...\n");
    init_paging();
    map_pages_physical(0, 0, 256); // identity map lower memory
    map_pages_physical((u32) kernel_start_addr, (u32) kernel_start_addr, // identity map our kernel code and data
                       (kernel_size + 4095) / 4096);
    enable_paging();

    terminal_printf("Initializing Timer\n");
    init_timer();
    terminal_printf("Timer Initialized\n");

    terminal_printf("Initializing Keyboard\n");
    init_keyboard();

    terminal_printf("Initializing serial io...\n");
    serial_init();

#ifdef TESTS_ENABLED // Test Flag should be passed to build script
    kernel_test();
#endif

    terminal_printf("CPL: %u\n", get_privilege_level());
    terminal_printf("%x - %x (%u)\n", kernel_start_addr, kernel_end_addr, kernel_size);
    jump_usermode(usermode_function, usermode_stack);

    for (;;) {
        asm ("hlt");
    }
}
