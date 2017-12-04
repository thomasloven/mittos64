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
  memset(vidmem, 0, VGA_SIZE*2);

  format = 0x7;
}

void movecursor()
{
  outb(0x3D4, 0x0F);
  outb(0x3D5, (uint8_t)(cursor & 0xFF));
  outb(0x3D4, 0x0E);
  outb(0x3D5, (uint8_t)((cursor >> 8) & 0xFF));
}

void flush()
{
  memcpy(vidmem, buffer, sizeof(buffer));
}

void scroll()
{
  while(cursor >= VGA_SIZE)
  {
    for(int i = 0; i < VGA_ROWS-1; i++)
      for(int j = 0; j < VGA_COLS; j++)
        buffer[VGA_POS(i, j)] = buffer[VGA_POS(i+1, j)];
    for(int i = 0; i < VGA_COLS; i++)
      buffer[VGA_POS(VGA_ROWS-1, i)] = (struct vga_cell){.c='\0', .f=format};
  cursor -= VGA_COLS;
  }

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
      buffer[cursor] = (struct vga_cell){.c = c, .f=format};
      cursor++;
  }
  scroll();
  flush();
  movecursor();
}
