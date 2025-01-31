#include "keyboard.h"
#include "terminal/terminal.h"
#include "lib/util.h"

void key_press() {
    char scanCode = inb(SCAN_PORT) & 0x7F; // Key
    char press = inb(SCAN_PORT) & 0x80; // Press down or release
    terminal_printf("Scan code: %x, Press: %i\n", scanCode, press);
}