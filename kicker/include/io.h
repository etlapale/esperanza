#ifndef __IO_H
#define __IO_H

#include <stdint.h>

#define inb(port) \
do \
  { \
    uint8_t val; \
    __asm__ __volatile__ ("inb  %w1, %%al" : "=a" (val) : "dN" (port)); \
    val; \
  } \
while (0);

#define outb(port, val) \
do \
{ \
  __asm__ __volatile__ ("outb %%al, %w1" :: "a" (val), "dN" (port)); \
} \
while (0);

#endif
