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
#include "lib/strings.h"
#include "lib/logging.h"

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

    set_loglevel(DEBUG); // Default Log level for now
    #ifdef LOG_DEBUG
    set_loglevel(DEBUG);
    #endif
    #ifdef LOG_INFO
    set_loglevel(INFO);
    #endif
    #ifdef LOG_CRITICAL
    set_loglevel(INFO);
    #endif

    terminal_init(
        multiboot_info->framebuffer_width, multiboot_info->framebuffer_height,
        (u16 *) multiboot_info->framebuffer_addr_lo
    );

    KERNEL_ASSERT(multiboot_info->framebuffer_addr_hi == 0);

    printk( 
        DEBUG,
        "Kernel Size: %u KB\n", (kernel_end_paddr - kernel_start_paddr) / 1024
    );

    printk(DEBUG, "Initializing GDT...\n");
    init_gdt();

    printk(DEBUG, "Initializing IDT...\n");
    init_idt();

    printk(DEBUG, "Initializing TSS...\n");
    init_tss();

    printk(DEBUG, "Initializing FPU...\n");
    init_fpu();

    printk(DEBUG, "Initializing memory...\n");
    mem_init();

    printk(DEBUG, "Initializing timer\n");
    init_timer();

    printk(DEBUG, "Initializing keyboard\n");
    init_keyboard();

    printk(DEBUG, "Initializing serial io...\n");
    serial_init();

    printk(DEBUG, "Initializing the sun...\n");
    sun_init();

#ifdef TESTS_ENABLED // Test Flag should be passed to build script
    kernel_test();
#endif

    processes_init();

    // TODO:
    // -L To set the Log level

    // Add your processes here
    // ex. exec_sun("binary.out", 0)

    asm("sti");

    schedule();

    for (;;) {
        asm("hlt");
    }
}
