#pragma once
#include <stdint.h>

static __inline void _outb(uint16_t port, uint8_t value)
{
  asm volatile("outb %1, %0" : : "dN" (port), "a" (value));
}

static __inline uint8_t _inb(uint16_t port)
{
  uint8_t ret;
  asm volatile("inb %1, %0" : "=a" (ret) : "dN" (port));
  return ret;
}

#define outb _outb
#define inb _inb
