#include <serial.h>
#include <ports.h>

void serial_init(uint16_t port)
{
  outb(port + 1, 0x00);
  outb(port + 3, 0x80);
  outb(port + 0, 0x03);
  outb(port + 1, 0x00);
  outb(port + 3, 0x03);
  outb(port + 2, 0xC7);
  outb(port + 4, 0x0B);
}

void serial_write(uint16_t port, uint8_t c)
{
  while(!(inb(port + 5)&0x20));
  outb(port, c);
}
