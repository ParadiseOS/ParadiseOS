#ifndef TERMINAL_H_
#define TERMINAL_H_

#include "types.h"

typedef struct {
    usize row;
    usize col;
    usize width;
    usize height;
    u8 color;
    u16 *buffer;
} Terminal;

typedef enum {
    VGA_COLOR_BLACK         = 0,
    VGA_COLOR_BLUE          = 1,
    VGA_COLOR_GREEN         = 2,
    VGA_COLOR_CYAN          = 3,
    VGA_COLOR_RED           = 4,
    VGA_COLOR_MAGENTA       = 5,
    VGA_COLOR_BROWN         = 6,
    VGA_COLOR_LIGHT_GREY    = 7,
    VGA_COLOR_DARK_GREY     = 8,
    VGA_COLOR_LIGHT_BLUE    = 9,
    VGA_COLOR_LIGHT_GREEN   = 10,
    VGA_COLOR_LIGHT_CYAN    = 11,
    VGA_COLOR_LIGHT_RED     = 12,
    VGA_COLOR_LIGHT_MAGENTA = 13,
    VGA_COLOR_LIGHT_BROWN   = 14,
    VGA_COLOR_WHITE         = 15,
} VgaColor;

extern Terminal terminal;

void terminal_init(usize width, usize height, u16 *buffer);
void terminal_putchar(u8 c);
void terminal_write_string(const char *string);
void terminal_print_hex(u32 n);

#endif // TERMINAL_H_
