#include <memory.h>
#include <stdint.h>
#include <stddef.h>

// NOTE: Functions in this file have different names during testing, since they
// are otherwise overridden by standard library functions

#ifdef TTEST
void *my_memcpy(void *dst, const void *src, size_t n)
#else
void *memcpy(void *dst, const void *src, size_t n)
#endif
{
  char *dp = dst;
  const char *sp = src;
  while(n--) *dp++ = *sp++;
  return dst;
}

#ifdef TTEST
void *my_memset(void *s, int c, size_t n)
#else
void *memset(void *s, int c, size_t n)
#endif
{
  unsigned char *p = s;
  while(n--) *p++ = (unsigned char)c;
  return s;
}

#ifdef TTEST
void *my_memmove(void *dest, const void *src, size_t n)
#else
void *memmove(void *dest, const void *src, size_t n)
#endif
{
  // We'll need some form of malloc to implement a good memmove.
  // For now, we'll just use defer to memcpy - WHICH IS UNSAFE!
  // TODO: Write a good implementation of memmove
  memcpy(dest, src, n);
  return dest;
}

#ifdef TTEST
int my_memcmp(const void *s1, const void *s2, size_t n)
#else
int memcmp(const void *s1, const void *s2, size_t n)
#endif
{
  const unsigned char *p1 = s1, *p2 = s2;
  for(; n--; p1++, p2++)
  {
    if(*p1 != *p2)
      return *p1 - *p2;
  }
  return 0;
}

#ifdef TTEST
size_t my_strlen(const char *s)
#else
size_t strlen(const char *s)
#endif
{
  size_t len = 0;
  while(*s++) len++;
  return len;
}
