#include "terminal.h"
#include "multiboot.h"

extern const u32 *multiboot_info;

void kernel_main(void) {
  usize width = multiboot_info[MB_FRAME_BUFFER_WIDTH_OFFSET];
  usize height = multiboot_info[MB_FRAME_BUFFER_HEIGHT_OFFSET];
  u16 *buffer = (u16 * ) multiboot_info[MB_FRAME_BUFFER_ADDR_OFFSET];

  terminal_init(width, height, buffer);

  terminal_write_string("Hello, Paradise!\n");
  terminal_write_string("Multiboot flags: ");
  terminal_print_hex(multiboot_info[MB_FLAGS_OFFSET]);
}
