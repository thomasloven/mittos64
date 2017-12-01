#pragma once
#include <stdint.h>

#define PORT_COM1 0x3F8

void serial_init(uint16_t port);
void serial_write(uint16_t port, uint8_t c);
