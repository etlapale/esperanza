/**
 * \file memory.h Manage allocation of memory.
 * The memory allocated by the functions defined in this file are
 * big chinks of memory, multiple of page size.
 *
 * \see kmalloc.h If you want to allocate little chunks of memory.
 */
#ifndef __MEMORY_H
#define __MEMORY_H

#include <stdint.h>

/** Default page size. */
#define DEFAULT_PAGE_SIZE 0x1000
#define BIG_PAGE_SIZE (1 << 21)

#ifdef CONFIG_CPU_IA32
#define PAGE_START_ADDRESS(addr) ((addr) & 0xfffff000)
#elif defined CONFIG_CPU_AMD64
#define PAGE_START_ADDRESS(addr) ((addr) & 0xfffffffffffff000)
#endif
#define PAGE_END_ADDRESS(addr) \
    (((addr) & 0xfff) ? \
    PAGE_START_ADDRESS (addr) + 0x1000 : \
    PAGE_START_ADDRESS (addr))

#ifdef CONFIG_CPU_IA32
#define PAGE_DIRECTORY(addr) ((addr) >> 22)
#define PAGE_TABLE(addr) (((addr) & 0x3ff000) >> 12)
#elif defined CONFIG_CPU_AMD64
#define PAGE_PML4(addr) (((addr) >> 39) & 0x1ff)
#define PAGE_DIRECTORY_POINTER(addr) (((addr) >> 30) & 0x1ff)
#define PAGE_DIRECTORY(addr) (((addr) >> 21) & 0x1ff)
#define PAGE_TABLE(addr) (((addr) >> 12) & 0x1ff)
#endif
#define PAGE_OFFSET(addr) ((addr) & 0xfff)

#define PAGE_ENTRY_SIZE 4
#define PAGE_TABLE_ENTRY_COUNT (DEFAULT_PAGE_SIZE / PAGE_ENTRY_SIZE)

#ifdef CONFIG_CPU_IA32
#define KERNEL_CODE_SEGMENT 0x08
#define KERNEL_DATA_SEGMENT 0x10
#define USER_CODE_SEGMENT   0x18
#define USER_DATA_SEGMENT   0x20
#elif defined CONFIG_CPU_AMD64
#define KERNEL_CODE_SEGMENT 0x08
#define KERNEL_DATA_SEGMENT 0x08
#define USER_CODE_SEGMENT   0x18 // TODO: mod-me!
#define USER_DATA_SEGMENT   0x20
#endif

#define USER_LEVEL_SEGMENT 3

#define PAGE_PRESENT (1 << 0)
#define PAGE_READ_WRITE (1 << 1)
#define PAGE_USER_LEVEL (1 << 2)
#define PAGE_WRITETHROUGHT (1 << 3)
#define PAGE_CACHE_DISABLED (1 << 4)
#define PAGE_ACCESSED (1 << 5)
#define PAGE_DIRTY (1 << 6)
#define PAGE_SIZE (1 << 7)
#define PAGE_4K_PAT (1 << 7)
#define PAGE_GLOBAL (1 << 8)
#define PAGE_2M_PAT (1 << 12)

#define PAGE_USER_RO (PAGE_USER_LEVEL | PAGE_PRESENT)
#define PAGE_USER_RW (PAGE_USER_LEVEL | PAGE_READ_WRITE | PAGE_PRESENT)


#ifdef CONFIG_CPU_IA32
#define KIP_VIRTUAL_ADDRESS 0x400000
#else /* CONFIG_CPU_AMD64 */
#define KIP_VIRTUAL_ADDRESS 0x200000
#endif


typedef struct memory_chunk
{
  uintptr_t address;
  size_t size;
  struct memory_chunk *prev;
  struct memory_chunk *next;
} memory_chunk_t;

typedef memory_chunk_t memory_hole_t;

extern memory_hole_t *memory_holes;
extern memory_chunk_t *memory_chunks;


void *
get_pm_page (void);

/**
 * Get a chunk of memory of the given size.
 * The memory allocated with this function is page-aligned.
 * Return NULL if the chunk could not be allocated.
 */
void *
get_memory (uintptr_t size);

/**
 * Get a chunk of memory initialized to zero.
 */
void *
get_clean_memory (uintptr_t size);

/**
 * Free a chunk of memory previously allocated.
 */
void
free_memory (void *addr);

uintptr_t
find_unused_pages (uintptr_t space, uintmax_t count);

/** Mark a memory portion as used. */
void
mark_memory_as_used (uintptr_t address, uintmax_t size);

void
associate_page (uintptr_t space, uintptr_t virtual, uintptr_t physical);


#endif
