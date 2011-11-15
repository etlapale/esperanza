#ifndef __IO_H
#define __IO_H

#include <stdint.h>

inline uint8_t
inb (const uint16_t port)
{
  uint8_t val;
  __asm__ __volatile__ ("inb  %w1, %0" : "=a" (val) : "dN" (port));
  return val;
}
inline void
outb (const uint16_t port, const uint8_t val)
{
  __asm__ __volatile__ ("outb %0, %w1" :: "a" (val), "dN" (port));
}

#endif
