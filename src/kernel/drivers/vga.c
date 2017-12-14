#include <vga.h>
#include <memory.h>
#include <ports.h>

void *vidmem;
struct vga_cell{
  uint8_t c;
  uint8_t f;
}__attribute__((packed));

struct vga_cell buffer[VGA_SIZE];

uint64_t cursor;
uint8_t format;

void vga_init()
{
  vidmem = VGA_MEMORY;
  memset(vidmem, 0, VGA_SIZE*sizeof(struct vga_cell));

  // White text on black background
  format = 0x07;
}

void movecursor()
{
  outb(VGA_ADDRESS_PORT, VGA_REGISTER_CURSOR_POS_LOW);
  outb(VGA_DATA_PORT, (uint8_t)(cursor & 0xFF));
  outb(VGA_ADDRESS_PORT, VGA_REGISTER_CURSOR_POS_HIGH);
  outb(VGA_DATA_PORT, (uint8_t)((cursor >> 8) & 0xFF));
}

void flush()
{
  memcpy(vidmem, buffer, sizeof(buffer));
}

void scroll()
{
  while(cursor >= VGA_SIZE)
  {
    // Move everything up one row
    memmove(buffer, &buffer[VGA_POS(1,0)], VGA_COLS*(VGA_ROWS-1)*sizeof(struct vga_cell));
    // Clear last row
    memset(&buffer[VGA_POS(VGA_ROWS-1, 0)], 0, VGA_COLS*sizeof(struct vga_cell));
    cursor -= VGA_COLS;
  }
}

void vga_write(char c)
{
  switch(c)
  {
    case '\n':
      cursor += VGA_COLS - VGA_COL(cursor);
      break;
    default:
      buffer[cursor++] = (struct vga_cell){.c = c, .f=format};
  }
  scroll();
  flush();
  movecursor();
}
