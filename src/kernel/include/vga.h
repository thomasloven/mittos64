#pragma once

#define VGA_COLS 80
#define VGA_ROWS 24
#define VGA_SIZE (VGA_COLS*VGA_ROWS)

#define VGA_ROW(pos) ((pos)/VGA_COLS)
#define VGA_COL(pos) ((pos)%VGA_COLS)
#define VGA_POS(row, col) ((row)*VGA_COLS + (col))

#define VGA_MEMORY P2V(0xB8000)

void vga_init();
void vga_write(char c);

#define VGA_ADDRESS_PORT 0x3D4
#define VGA_DATA_PORT 0x3D5
#define VGA_REGISTER_CURSOR_POS_LOW 0xF
#define VGA_REGISTER_CURSOR_POS_HIGH 0xE
