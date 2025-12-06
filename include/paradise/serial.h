#ifndef SERIAL_H_
#define SERIAL_H_

#include "types.h"

#define PORT_COM1 0x3F8

void serial_init();

u8 serial_read();
void serial_write(u8 data);

#endif // SERIAL_H_
