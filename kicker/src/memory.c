#include <elf.h>
#include <malloc.h>
#include <memory.h>
#include <printf.h>
#include <stddef.h>
#include <stdint.h>


Memory_Hole *memory_holes;
Memory_Chunk *memory_chunks;


void
init_memory (multiboot_info_t * mbi)
{
  uint32_t start, end;
  Elf32_Shdr *shdr;
  elf_section_header_table_t *elf_sec;
  Multiboot_Memory_Map *mmap;
  module_t *mod;
  int i;
  Memory_Hole *hole;
  
  /* We need the ELF header and the memory map from the boot loader */
  if (!CHECK_FLAG (mbi->flags, 5)
       || !CHECK_FLAG (mbi->flags, 6))
    panic ("Need more info from the multiboot loader\n");
  
  /* Init the memory map with sentinels */
  memory_holes = (Memory_Hole *) malloc (sizeof (Memory_Hole));
  memory_holes->size = 0;
  memory_holes->prev = memory_holes;
  memory_holes->next = memory_holes;
  memory_chunks = (Memory_Chunk *) malloc (sizeof (Memory_Chunk));
  memory_chunks->prev = memory_chunks;
  memory_chunks->next = memory_chunks;

  /* Parse memory map */
  for (mmap = (Multiboot_Memory_Map *) mbi->mmap_addr;
      (uint32_t) mmap < mbi->mmap_addr + mbi->mmap_length;
      mmap = (Multiboot_Memory_Map *) ((uint32_t) mmap + mmap->size + sizeof (mmap->size)))
    {
      /* Not available RAM */
      if (mmap->type != 1)
        continue;
      
      hole = (Memory_Hole *) malloc (sizeof (Memory_Hole));
      
      /* Only 32bits address are managed */
      if (mmap->base_addr_high != 0 || mmap->length_high != 0)
        panic ("Cannot handle more than 4GB\n");
      
      hole->address = mmap->base_addr_low;
      hole->size = mmap->length_low;
      
      /* Add the hole to the list */
      memory_holes->prev->next = hole;
      hole->prev = memory_holes->prev;
      hole->next = memory_holes;
      memory_holes->prev = hole;
    }
  
  /* Fetch kernel size and address */
  elf_sec = &(mbi->u.elf_sec);
  if (sizeof (Elf32_Shdr) != elf_sec->size)
    panic ("Cannot handle not 32bits ELF images\n");
  
  /* We don't parse each section, we suppose that the first non-null
   * section is the second and the last is the last section.
   * All the pages allocated between this two section are unavailable. */
  /*
   * TODO: we should explicitly set the ELF sections and parse them
   * (deleting the init section at the end of the kernel initialization). */
  shdr = (Elf32_Shdr *) (elf_sec->addr + sizeof (Elf32_Shdr));
  start = PAGE_START_ADDRESS (shdr->sh_addr);
  shdr = (Elf32_Shdr *) (elf_sec->addr
			 + sizeof (Elf32_Shdr) * (elf_sec->num - 1));
  end = PAGE_END_ADDRESS (shdr->sh_addr + shdr->sh_size);
  mark_memory_as_used (start, end - start);
  
  /* Mark the page used by the modules as used */
  for (i = 0, mod = (module_t *) mbi->mods_addr;
       i < (int) mbi->mods_count;
       i++, mod++)
    mark_memory_as_used (PAGE_START_ADDRESS (mod->mod_start),
			 PAGE_END_ADDRESS (mod->mod_end - mod->mod_start));
  
  /* Display the memory map of all available memory */
  printf ("Memory map including kernel/modules"
	  "(kernel ends at: 0x%x)\n", (uint32_t) end);
  for (hole = memory_holes->next; hole != memory_holes; hole = hole->next)
    printf ("\tMemory hole: 0x%x - 0x%x\n",
	    hole->address, hole->address + hole->size);
}

void
mark_memory_as_used (uint32_t address, uint32_t size)
{
  Memory_Hole *hole;
  uint32_t end;

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
              Memory_Hole *new_hole = (Memory_Hole *) malloc (sizeof (Memory_Hole));
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
get_clean_memory (uint32_t size)
{           
  void *ptr;
  uint32_t *i;
              
              
  /* Align it on 4KB pages */
  size = PAGE_END_ADDRESS (size);

  ptr = get_memory (size);
  if (ptr == NO_MORE_MEMORY)
    return NO_MORE_MEMORY;
            
  for (i = (uint32_t *) ptr; i < (uint32_t *) ptr + size; i++)
    *i = 0;
          
  return ptr;
}

void *
get_memory (uint32_t size)
{         
  Memory_Hole *hole;
    
  hole = memory_holes->next;
  
  /* Align it on 4KB pages */
  size = PAGE_END_ADDRESS (size);
  
  while (hole != memory_holes)
    {
      Memory_Chunk *chunk = NULL;

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
              chunk = (Memory_Chunk *) malloc (sizeof (Memory_Chunk));
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

          printf ("\tAllocating memory at 0x%x\n", chunk->address);
          return (void *) chunk->address;;
        }
    }

  printf ("No more memory!\n");
  return NO_MORE_MEMORY;
}

void
free_memory (void *addr)
{
  int *i;

  i = (int *) addr;
}
