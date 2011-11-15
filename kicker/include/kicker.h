#ifndef __KICKER_H
#define __KICKER_H

#include <stdint.h>

struct Kicker_Boot_Module
{
  uint32_t start;
  uint32_t end;
  uint32_t string;
};

struct Kicker_Info
{
  uint32_t heap_address;
  uint32_t heap_size;

  uint32_t memory_chunks;
  uint32_t memory_holes;

  uint32_t modules_count;
  uint32_t modules_address;

  uint32_t cr3;
};

#endif
