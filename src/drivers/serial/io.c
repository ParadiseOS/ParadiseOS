#include "io.h"
#include "lib/error.h"
#include "lib/util.h"

void serial_init() {
    outb(PORT_COM1 + 1, 0x00); // Disable all interrupts
    outb(PORT_COM1 + 3, 0x80); // Enable DLAB (set baud rate divisor)
    outb(PORT_COM1 + 0, 0x03); // Set divisor to 3 (lo byte) 38400 baud
    outb(PORT_COM1 + 1, 0x00); //                  (hi byte)
    outb(PORT_COM1 + 3, 0x03); // 8 bits, no parity, one stop bit
    outb(PORT_COM1 + 2, 0xC7); // Enable FIFO, clear them, with 14-byte
                               // threshold
    outb(PORT_COM1 + 4, 0x0B); // IRQs enabled, RTS/DSR set
    outb(PORT_COM1 + 4, 0x1E); // Set in loopback mode, test the serial chip
    outb(PORT_COM1 + 0, 0xAE); // Test serial chip (send byte 0xAE and check if
                               // serial returns same byte)

    // Check if serial is faulty (i.e: not same byte as sent)
    KERNEL_ASSERT(inb(PORT_COM1 + 0) == 0xAE);

    // If serial is not faulty set it in normal operation mode
    // (not-loopback with IRQs enabled and OUT#1 and OUT#2 bits enabled)
    outb(PORT_COM1 + 4, 0x0F);
}

bool serial_received() {
    return inb(PORT_COM1 + 5) & 1;
}

u8 serial_read() {
    while (!serial_received()) {}
    return inb(PORT_COM1);
}

bool is_transmit_empty() {
    return inb(PORT_COM1 + 5) & 0x20;
}

void serial_write(u8 data) {
    while (!is_transmit_empty()) {}
    outb(PORT_COM1, data);
}
