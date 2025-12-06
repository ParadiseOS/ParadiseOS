#include <paradise/error.h>
#include <paradise/init.h>
#include <paradise/interrupt.h>
#include <paradise/kernel.h>
#include <paradise/keyboard.h>
#include <paradise/logging.h>
#include <paradise/mem.h>
#include <paradise/multiboot.h>
#include <paradise/processes.h>
#include <paradise/serial.h>
#include <paradise/strings.h>
#include <paradise/sun.h>
#include <paradise/syscall.h>
#include <paradise/terminal.h>
#include <paradise/testing.h>
#include <paradise/timer.h>

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

    printk(
        DEBUG, "Kernel Size: %u KB\n",
        (kernel_end_paddr - kernel_start_paddr) / 1024
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

    syscalls_init();
    processes_init();

    // Add your processes here
    // ex. exec_sun("binary.out", 0)

    asm("sti");

    schedule();

    for (;;) {
        asm("hlt");
    }
}
