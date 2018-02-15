#pragma once

#define VGA_MEMORY P2V(0xB8000)

void vga_init();
void vga_write(char c);
