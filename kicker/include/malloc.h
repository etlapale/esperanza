/*
 * \file malloc.h
 * Dynamic memory allocator for the kicker.
 * There is no free() function as the kicker heap will be deleted after
 * switching to the kernel.
 */

#ifndef __MALLOC_H
#define __MALLOC_H

#include <stdint.h>

/**
 * Init the heap at the given address with the given size.
 * \param addr The heap address.
 * \param size The heap size.
 */
void
init_malloc (uintptr_t addr, size_t size);

/**
 * Allocate and return a chunk of memory of the given size.
 * Return NULL on error.
 * \param size Size of the memory block to be allocated.
 * \return The address of the created block or \c NULL.
 */
void * __attribute__ ((malloc))
malloc (size_t size);

#endif
