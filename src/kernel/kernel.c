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
    terminal_printf("DEBUG\n");
    set_loglevel(DEBUG);
    #endif
    #ifdef LOG_INFO
    set_loglevel(INFO);
    terminal_printf("INFO\n");
    #endif
    #ifdef LOG_CRITICAL
    set_loglevel(INFO);
    terminal_printf("CRITICAL\n");
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
    char buffer[256];
    char tiny_buffer[12];
    u32 ret;

    terminal_printf("--- Starting comprehensive snprintf test suite ---\n\n");

    // --- Basic Format Specifier Tests ---
    terminal_printf("Testing: Basic formats (%%, %%c, %%s)\n");
    ret = snprintf(buffer, 256, "A literal percent sign: %%");
    KERNEL_ASSERT(strcmp(buffer, "A literal percent sign: %") == 0);
    KERNEL_ASSERT(ret == 25);

    ret = snprintf(buffer, 256, "Character: %c", 'Z');
    KERNEL_ASSERT(strcmp(buffer, "Character: Z") == 0);
    KERNEL_ASSERT(ret == 12);

    ret = snprintf(buffer, 256, "String: %s", "ParadiseOS");
    KERNEL_ASSERT(strcmp(buffer, "String: ParadiseOS") == 0);
    KERNEL_ASSERT(ret == 18);


    // --- Integer Format Tests ---
    terminal_printf("Testing: Integer formats (%%i, %%u)\n");
    ret = snprintf(buffer, 256, "Signed positive: %i", 12345);
    KERNEL_ASSERT(strcmp(buffer, "Signed positive: 12345") == 0);
    KERNEL_ASSERT(ret == 22);

    ret = snprintf(buffer, 256, "Signed negative: %i", -6789);
    KERNEL_ASSERT(strcmp(buffer, "Signed negative: -6789") == 0);
    KERNEL_ASSERT(ret == 22);

    ret = snprintf(buffer, 256, "Signed zero: %i", 0);
    KERNEL_ASSERT(strcmp(buffer, "Signed zero: 0") == 0);
    KERNEL_ASSERT(ret == 14);

    ret = snprintf(buffer, 256, "Unsigned: %u", 4294967295u);
    KERNEL_ASSERT(strcmp(buffer, "Unsigned: 4294967295") == 0);
    KERNEL_ASSERT(ret == 20);


    // --- Hex, Binary, and Pointer Tests ---
    terminal_printf("Testing: Base formats (%%x, %%b, %%p)\n");
    ret = snprintf(buffer, 256, "Hex: 0x%x", 0xDEADBEEF);
    KERNEL_ASSERT(strcmp(buffer, "Hex: 0xDEADBEEF") == 0);
    KERNEL_ASSERT(ret == 15);

    ret = snprintf(buffer, 256, "Binary: %b", 42);
    KERNEL_ASSERT(strcmp(buffer, "Binary: 101010") == 0);
    KERNEL_ASSERT(ret == 14);

    ret = snprintf(buffer, 256, "Pointer: %p", (void*)0x1000);
    KERNEL_ASSERT(strcmp(buffer, "Pointer: 0x1000") == 0);
    KERNEL_ASSERT(ret == 15);


    // --- Float Format Tests (Uncomment if your kernel supports floating point) ---
    terminal_printf("Testing: Float formats (%%f, %%.2f)\n");
    ret = snprintf(buffer, 256, "Float: %f", 3.14159);
    KERNEL_ASSERT(strcmp(buffer, "Float: 3.141590") == 0); // Default 6 decimal places
    KERNEL_ASSERT(ret == 15);

    ret = snprintf(buffer, 256, "Precision (rounding up): %.3f", 9.8765);
    KERNEL_ASSERT(strcmp(buffer, "Precision (rounding up): 9.877") == 0);
    KERNEL_ASSERT(ret == 30);
    
    terminal_printf("Testing: Special float values (NaN, Infinity)\n");
    ret = snprintf(buffer, 256, "Value is %f", NAN);
    KERNEL_ASSERT(strcmp(buffer, "Value is nan") == 0);

    ret = snprintf(buffer, 256, "Value is %f", INFINITY);
    KERNEL_ASSERT(strcmp(buffer, "Value is inf") == 0);


    // --- Truncation and Return Value Tests ---
    terminal_printf("Testing: Truncation and return value logic\n");
    // Test 1: Output is longer than the buffer
    ret = snprintf(tiny_buffer, 12, "This is a long string");
    KERNEL_ASSERT(strcmp(tiny_buffer, "This is a l") == 0); // Buffer size 12 -> 11 chars + null
    KERNEL_ASSERT(ret == 21); // Should return the length that *would have* been written

    // Test 2: Output fits exactly
    ret = snprintf(tiny_buffer, 12, "0123456789A"); // 11 characters
    KERNEL_ASSERT(strcmp(tiny_buffer, "0123456789A") == 0);
    KERNEL_ASSERT(ret == 11);

    // Test 3: Output is one character too long
    ret = snprintf(tiny_buffer, 12, "0123456789AB"); // 12 characters
    KERNEL_ASSERT(strcmp(tiny_buffer, "0123456789A") == 0);
    KERNEL_ASSERT(ret == 12);


    // --- Edge Case Tests ---
    terminal_printf("Testing: Edge cases (NULL buffer, zero-size buffer, NULL string)\n");
    // Test 1: NULL buffer for length calculation
    ret = snprintf(NULL, 0, "Calculate length of this: %u", 123);
    KERNEL_ASSERT(ret == 29);

    // Test 2: Zero-size buffer
    ret = snprintf(tiny_buffer, 0, "hello");
    KERNEL_ASSERT(ret == 5); // Should write nothing and return calculated length

    // Test 3: Buffer of size 1
    char single_char_buffer[1];
    ret = snprintf(single_char_buffer, 1, "hello");
    KERNEL_ASSERT(single_char_buffer[0] == '\0'); // Should only write null terminator
    KERNEL_ASSERT(ret == 5);

    // Test 4: NULL string argument
    ret = snprintf(buffer, 256, "The pointer is %s.", (char*)NULL);
    KERNEL_ASSERT(strcmp(buffer, "The pointer is (null).") == 0);
    KERNEL_ASSERT(ret == 22);
    
    // --- Complex Combination Test ---
    terminal_printf("Testing: Complex combination of specifiers\n");
    ret = snprintf(buffer, 256, "User '%s' (ID: %u) logged in from 0x%x", "lk3ond", 1003, 0xC0DE);
    KERNEL_ASSERT(strcmp(buffer, "User 'lk3ond' (ID: 1003) logged in from 0xC0DE") == 0);
    KERNEL_ASSERT(ret == 46);


    terminal_printf("\n--- All snprintf tests passed successfully! ---\n");


    asm("sti");

    //schedule();

    for (;;) {
        asm("hlt");
    }
}
