/**
 * \file memory.h Manage allocation of memory.
 * The memory allocated by the functions defined in this file are
 * big chinks of memory, multiple of page size.
 *
 * \see kmalloc.h If you want to allocate little chunks of memory.
 */
#ifndef __MEMORY_H
#define __MEMORY_H

#include <multiboot.h>

/** Default page size. */
#define DEFAULT_PAGE_SIZE 0x1000
#define BIG_PAGE_SIZE (1 << 21)

#define PAGE_START_ADDRESS(addr) ((addr) & 0xfffff000)
#define PAGE_END_ADDRESS(addr) (((addr) & 0xfff) ? PAGE_START_ADDRESS (addr) + 0x1000 : PAGE_START_ADDRESS (addr))

#define PAGE_DIRECTORY(addr) ((addr) >> 22)
#define PAGE_TABLE(addr) (((addr) & 0x3ff000) >> 12)
#define PAGE_ENTRY_SIZE 4
#define PAGE_TABLE_ENTRY_COUNT (DEFAULT_PAGE_SIZE / PAGE_ENTRY_SIZE)

#define PAGE_PRESENT (1 << 0)
#define PAGE_READ_WRITE (1 << 1)
#define PAGE_USER_LEVEL (1 << 2)
#define PAGE_SIZE (1 << 7)
#define PAGE_GLOBAL (1 << 8)


#define NO_MORE_MEMORY ((void *) 0xcafebabe)

struct __Memory_Chunk
{
  uint32_t address;
  uint32_t size;
  struct __Memory_Chunk *prev;
#ifdef CONFIG_CPU_AMD64
  uint32_t unused1;
#endif
  struct __Memory_Chunk *next;
#ifdef CONFIG_CPU_AMD64
  uint32_t unused2;
#endif
} __attribute__ ((packed));

typedef struct __Memory_Chunk Memory_Chunk;
typedef struct __Memory_Chunk Memory_Hole;

extern Memory_Hole *memory_holes;
extern Memory_Chunk *memory_chunks;


#ifndef CONFIG_IA32_PSE
/** The kernel page table shared by all the processes. */
extern uint32_t kernel_page_table;
#endif


/** Init the pagination for the idle thread. */
void
init_paging (uint32_t pagedir);
    
/**
 * Initialize the memory allocator with the multiboot info.
 */
void
init_memory (multiboot_info_t *mbi);

/**
 * Get a chunk of memory of the given size.
 * The memory allocated with this function is page-aligned.
 * Return NULL if the chunk could not be allocated.
 */
void *
get_memory (uint32_t size);

/**
 * Get a chunk of memory initialized to zero.
 * \see get_memory(uint32_t)
 */
void *
get_clean_memory (uint32_t size);


/**
 * Free a chunk of memory previously allocated.
 */
void
free_memory (void *addr);

uint32_t
find_unused_pages (uint32_t space, uint32_t count);

/** Mark a memory portion as used. */
void
mark_memory_as_used (uint32_t address, uint32_t size);


#endif
