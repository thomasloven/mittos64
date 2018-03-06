#pragma once

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

void debug_printf(char *fmt, ...);
void debug_puts(char *s);
void debug_putsn(char *s, size_t n);
void debug_putch(char c);

#define S(x) #x
#define S_(x) S(x)
#define S__LINE__ S_(__LINE__)
#define PANIC(...) \
  do{ \
    debug("\n\nKernel panic!\n%s:%d\n", __FILE__, __LINE__); \
    debug(__VA_ARGS__); \
    volatile int _override = 0; \
    while(1){ \
      asm("panic_breakpoint_" S__LINE__ ":"); \
      if(_override) break; \
    } \
  }while(0)
