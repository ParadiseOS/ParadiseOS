#include "testing.h"
#include "boot/multiboot.h"
#include "memory/mem.h"
#include "lib/types.h"
#include "terminal/terminal.h"
#include "lib/error.h"
#include "drivers/serial/io.h"

void kernel_test() {
    terminal_printf("\nTesting is enabled!\n");

    /* // Testing that paging works. Here we remap an invalid address. */
    /* map_pages(0x694200, 0xFFFFFFFF, 1); */
    /* u8 *ptr = (u8 *) 0xFFFFFFFF; // this wouldn't work without paging */

    /* *ptr = 69; */
    /* terminal_printf("*ptr: %u\n", (u32) *ptr); */

    terminal_printf("Charset:\n");
    terminal.color = vga_color_create(VGA_COLOR_LIGHT_BLUE, VGA_COLOR_BLACK);
    for (int h = 0; h < 16; ++h) {
      for (int l = 0; l < 16; ++l) {
        terminal_printf("%c", h << 4 | l);
      }
      terminal_printf("\n");
    }
    terminal.color = vga_color_create(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);

    terminal_printf("Colors:\n");
    for (int c = 0; c < 16; ++c) {
      terminal.color = vga_color_create(VGA_COLOR_LIGHT_GREY, c);
      terminal_printf(" ");
    }

    i32 a = -45;
    u32 b = 70;
    const char *message = "Hello!";

    terminal.color = vga_color_create(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    terminal_printf("\n");
    terminal_printf("printf Test: %u, %i, %u, %i, %x, %x, %b, %b, %s, %c, %%\n",
                    a, a, b, b, a, b, a, b, message, *message);
    terminal.color = vga_color_create(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);

    MMapEntry *entries = multiboot_info->mmap_addr;

    KERNEL_ASSERT(multiboot_info->flags & MB_FLAG_MMAP);

    for (u32 i = 0; i < multiboot_info->mmap_length / sizeof (MMapEntry); i++) {
        terminal_printf("%u (%u): %p %u ", i, entries[i].size, entries[i].base_addr_lo, entries[i].length_lo);

        switch (entries[i].type) {
        case MMAP_AVAILABLE:
            terminal_printf("Available\n");
            break;
        case MMAP_RESERVED:
            terminal_printf("Reserved\n");
            break;
        case MMAP_ACPI:
            terminal_printf("ACPI\n");
            break;
        case MMAP_NVS:
            terminal_printf("NVS\n");
            break;
        case MMAP_DEFECTIVE:
            terminal_printf("Defective\n");
            break;
        default:
            terminal_printf("?? (%u)\n", entries[i].type);
            break;
        }
    }

    serial_write('H');
    serial_write('e');
    serial_write('l');
    serial_write('l');
    serial_write('o');

    //asm ("int $0x80"); // test our interrupt handler
}
