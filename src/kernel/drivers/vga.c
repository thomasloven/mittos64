#include <vga.h>
#include <memory.h>

void *vidmem;
struct {
  uint8_t character;
  uint8_t format;
}__attribute__((packed)) buffer[VGA_SIZE];

uint64_t cursor;

void vga_init()
{
  vidmem = VGA_MEMORY;
  memset(vidmem, 0, VGA_SIZE*2);
}

void flush()
{
  memcpy(vidmem, buffer, sizeof(buffer));
}

void vga_write(char c)
{
  switch(c)
  {
    case '\n':
      cursor += VGA_COLS;
      cursor -= VGA_COL(cursor);
      break;
    default:
      buffer[cursor].character = c;
      buffer[cursor].format = 0x7;
      cursor++;
  }
  flush();
}
