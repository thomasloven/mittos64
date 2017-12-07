#pragma once
#include <stddef.h>
#include <stdarg.h>

#ifndef NDEBUG
#define debug(...) debug_printf(__VA_ARGS__)
#define debug_info(...) \
  do{debug("[INFO]    "); debug(__VA_ARGS__);}while(0)
#define debug_ok(...) \
  do{debug("[OK]      "); debug(__VA_ARGS__);}while(0)
#define debug_warning(...) \
  do{debug("[WARNING] "); debug(__VA_ARGS__);}while(0)
#define debug_error(...) \
  do{debug("[ERROR]   "); debug(__VA_ARGS__);}while(0)
#else
#define debug(...)
#define debug_info(...)
#define debug_ok(...)
#define debug_warning(...)
#define debug_error(...)
#endif

void debug_putch(char c);
void debug_putsn(char *s, size_t n);
void debug_puts(char *s);
void debug_vprintf(char *fmt, va_list args);
void debug_printf(char *fmt, ...);
