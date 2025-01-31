#ifndef IO_H_
#define IO_H_

#include "lib/types.h"

#define PORT_COM1 0x3F8

void serial_init();

u8 serial_read();
void serial_write(u8 data);

#endif // IO_H_
