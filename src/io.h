#ifndef IO_H_
#define IO_H_

#include "types.h"

#define PORT_COM1 0x3F8

extern u8 inb(u16 port);
extern void outb(u16 port, u8 data);

void serial_init();

u8 serial_read();
void serial_write(u8 data);

#endif // IO_H_
