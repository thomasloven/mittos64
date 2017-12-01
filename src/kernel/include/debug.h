#pragma once
#include <stddef.h>
#include <stdarg.h>

void debug_putch(char c);
void debug_putsn(char *s, size_t n);
void debug_puts(char *s);
void debug_vprintf(char *fmt, va_list args);
void debug_printf(char *fmt, ...);
