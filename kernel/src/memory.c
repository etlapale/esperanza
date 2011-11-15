#include <btalloc.h>
#include <memory.h>
#include <printf.h>
#include <stddef.h>
#include <stdint.h>


/* First page for the Memory_Chunk structures allocator set by the linker. */
extern bt_allocator_t pmem_allocator;

memory_hole_t *memory_holes;
memory_chunk_t *memory_chunks;

typedef struct memory_chunk32
{
  uint32_t address;
  uint32_t size;
  struct memory_chunk32 *prev;
  struct memory_chunk32 *next;
} __attribute__ ((packed)) memory_chunk32_t;

typedef memory_chunk32_t memory_hole32_t;

static void
copy_chunks (memory_chunk32_t *src, memory_chunk_t *dst)
{
  memory_chunk32_t *first = src->prev;
  memory_chunk_t *first_dst = dst;
  
  while (src != first)
    {
      dst->next = (memory_chunk_t *) bt_alloc (&pmem_allocator);
      dst->next->prev = dst;
      dst->next->next = first_dst;

      dst->next->address = src->address;
      dst->next->size = src->size;

      dst = dst->next;
      src = src->next;
    }
}

static memory_chunk_t *
create_sentinel ()
{
  memory_chunk_t *chunk = (memory_chunk_t *) bt_alloc (&pmem_allocator);
  chunk->address = chunk->size = 0;
  chunk->prev = chunk->next = chunk;
  return chunk;
}

void
print_memory_map ()
{
  memory_hole_t *hole;
  printf ("**Kernel memory map**\n");
  for (hole = memory_holes->next; hole != memory_holes; hole = hole->next)
    printf ("\tMemory hole: %p - %p\n",
	    (void *) hole->address,
	    (void *) hole->address + hole->size);
}

void *
get_pm_page (void) {
  return get_memory (DEFAULT_PAGE_SIZE);
}

void
init_memory (memory_chunk32_t *chunks, memory_hole32_t *holes)
{
  /* Init the pm_entry allocator */
  bt_create_allocator_with_first (&pmem_allocator,
				  get_pm_page, free_memory,
				  DEFAULT_PAGE_SIZE, sizeof (memory_chunk_t));

  /* init the memory map with sentinels */
  memory_chunks = create_sentinel ();
  memory_holes = create_sentinel ();

  /* Parse memory the given map */
  copy_chunks (chunks->next, memory_chunks);
  copy_chunks (holes->next, memory_holes);
  
  /* Display the memory map of all available memory */
  print_memory_map ();
}

void
mark_memory_as_used (uintptr_t address, uintmax_t size)
{
  memory_hole_t *hole;
  uintptr_t end;

  /* Align it on 4KB pages */
  address = PAGE_START_ADDRESS (address);
  size = PAGE_END_ADDRESS (size);

  end = address + size;
  
  /*
   * Search the memory portion in each hole.
   * Remember that the first hole is a sentinel.
   */
  for (hole = memory_holes->next; hole != memory_holes; hole = hole->next)
    {
      /* TODO: Check if only a portion of the memory is in the hole */
      if (hole->address <= address && end <= hole->address + hole->size)
        {
          if (hole->address == address)
            {
              hole->address += size;
              hole->size -= size;
            }
          else if (hole->address + hole->size == end)
            {
              hole->size -= end - address;
            }
          else
            {
              memory_hole_t *new_hole =
		(memory_hole_t *) bt_alloc (&pmem_allocator);
              new_hole->address = end;
              new_hole->size = hole->address + hole->size - end;
              
              new_hole->prev = hole;
              new_hole->next = hole->next;
              hole->next->prev = new_hole;
              hole->next = new_hole;
              
              hole->size = address - hole->address;
            }
          break;
        }
    }
  
  
  if (hole == memory_holes)
    panic ("Hole not found for [0x%x,0x%x]\n", address, end);
}

void *        
get_clean_memory (uintmax_t size)
{           
  void *ptr;
  uintmax_t *i;
  
  size = PAGE_END_ADDRESS (size);
  ptr = get_memory (size);
  
  if (ptr) /* TODO: a function! */
    for (i = (uintmax_t *) ptr; i < (uintmax_t *) ptr + size; i++)
      *i = 0;
          
  return ptr;
}

void *
get_memory (uintmax_t size)
{         
  memory_hole_t *hole;
    
  hole = memory_holes->next;
  
  /* Align it on 4KB pages */
  size = PAGE_END_ADDRESS (size);
  
  while (hole != memory_holes)
    {
      memory_chunk_t *chunk = NULL;

      if (size <= hole->size)
        {
          if (size == hole->size)
            {
              /* Remove the hole */
              hole->prev->next = hole->next;
              hole->next->prev = hole->prev;

              /* Convert the hole in a chunk */
              chunk = hole;
            }
          else if (size < hole->size)
            {
              /* Create a new memory chunk */
              chunk = (memory_chunk_t *) bt_alloc (&pmem_allocator);
              chunk->address = hole->address;
              chunk->size = size;

              /* Resize the hole */
              hole->address += size;
              hole->size -= size;
            }

          /* Add the hole to the list */
          memory_chunks->prev->next = chunk;
	  chunk->prev = memory_chunks->prev;
          chunk->next = memory_chunks;
          memory_chunks->prev = chunk;

          printf ("\tAllocating memory at %p\n",
		  (void *) chunk->address);
          return (void *) chunk->address;;
        }
    }
  
  printf ("***No more memory!***\n");
  return 0;
}

/* TODO: allow to free the memory! */
void
free_memory (void *addr)
{
  printf ("***Freeing memory is not yet implemented***\n");
}

void
associate_page (uintptr_t space, uintptr_t virtual, uintptr_t physical)
{
  printf ("\tassociate_page (%p, %p, %p)\n",
	  (void *) space, (void *) virtual, (void *) physical);

  uintptr_t *ptr;

#ifdef CONFIG_CPU_AMD64

  // Find the PML4 table
  ptr = (uintptr_t *) space + PAGE_PML4(virtual);
  // If not present, create it
  if ((*ptr & PAGE_PRESENT) == 0)
    *ptr = (uintptr_t) get_clean_memory (DEFAULT_PAGE_SIZE)
      | PAGE_USER_LEVEL | PAGE_READ_WRITE | PAGE_PRESENT;

  // Find the PDP table
  ptr = (uintptr_t *) PAGE_START_ADDRESS (*ptr)
    + PAGE_DIRECTORY_POINTER (virtual);
  // If not present, create it  
  if ((*ptr & PAGE_PRESENT) == 0)
    *ptr = (uintptr_t) get_clean_memory (DEFAULT_PAGE_SIZE)
      | PAGE_USER_LEVEL | PAGE_READ_WRITE | PAGE_PRESENT;

  // Find the PD table
  ptr = (uintptr_t *) PAGE_START_ADDRESS (*ptr)
    + PAGE_DIRECTORY (virtual);

#elif defined CONFIG_CPU_IA32

  // Find the PD table
  ptr = (uintptr_t *) space + PAGE_DIRECTORY (virtual);
  printf ("Trying PD at: 0x%x\n", (uintptr_t) ptr);
#endif
  
  // If not present, create it
  if ((*ptr & PAGE_PRESENT) == 0)
    *ptr = (uintptr_t) get_clean_memory (DEFAULT_PAGE_SIZE)
      | PAGE_USER_LEVEL | PAGE_READ_WRITE | PAGE_PRESENT;
  
  // Find the Page Table
  ptr = (uintptr_t *) PAGE_START_ADDRESS (*ptr)
    + PAGE_TABLE (virtual);
  
  // Modify the page table entry
  *ptr = physical;

  printf ("\t\tModing %p to %p\n",
	  ptr, (void *) physical);
}
