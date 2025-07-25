#include "kernel.h"
#include "boot/multiboot.h"
#include "drivers/keyboard/keyboard.h"
#include "drivers/serial/io.h"
#include "drivers/timer/timer.h"
#include "init.h"
#include "interrupts/interrupt.h"
#include "lib/error.h"
#include "memory/mem.h"
#include "process/processes.h"
#include "sun/sun.h"
#include "terminal/terminal.h"
#include "tests/testing.h"

const u32 kernel_start_paddr = (u32) &_kernel_start_paddr;
const void *kernel_start_vaddr = &_kernel_start_vaddr;
const u32 kernel_end_paddr = (u32) &_kernel_end_paddr;
const void *kernel_end_vaddr = &_kernel_end_vaddr;

void kernel_main(void) {
    if (!(multiboot_info->flags & MB_FLAG_FRAMEBUFFER)) {
        // No terminal info is available. Something went wrong and there's no
        // way to report it.
        kernel_panic();
    }

    terminal_init(
        multiboot_info->framebuffer_width, multiboot_info->framebuffer_height,
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

    terminal_printf("Initializing the sun...\n");
    sun_init();

#ifdef TESTS_ENABLED // Test Flag should be passed to build script
    kernel_test();
#endif

    // Add your processes here
    // ex. exec_sun("binary.out", 0)

    scheduler_init();
    schedule();

    for (;;) {
        asm("hlt");
    }
}
