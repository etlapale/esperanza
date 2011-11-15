#include <malloc.h>
#include <printf.h>
#include <stddef.h>

struct __Kernel_Memory_Allocation
{
  struct __Kernel_Memory_Allocation *previous;
  struct __Kernel_Memory_Allocation *next;
  uintptr_t size ;
};

typedef struct __Kernel_Memory_Allocation Kernel_Memory_Allocation;
typedef struct __Kernel_Memory_Allocation Kernel_Memory_Free;

static Kernel_Memory_Allocation * kmem_first;
static Kernel_Memory_Free * kfree_first;

void
init_malloc (uintptr_t addr, size_t size)
{
  printf ("Initializing a heap at 0x%x (size: 0x%x)\n", addr, size);

  kmem_first = NULL;
  kfree_first = (Kernel_Memory_Free *) addr;
  kfree_first->previous = kfree_first;
  kfree_first->next = kfree_first;
  kfree_first->size = size - sizeof (Kernel_Memory_Free);
}

void *
malloc (size_t size)
{

  Kernel_Memory_Free * current ;

  current = kfree_first ;

  /* No more free space */
  if (current == NULL)
    {
      printf ("No more free space\n") ;
      return NULL ;
    }

  do
    {

      /* Enough space found */
      if (current->size >= size)
        {
          uintptr_t rest ;
          Kernel_Memory_Allocation * alloc ;

          /* New free space size */
          rest = current->size - size ;
          /* Allocate */
          alloc = current ;

          /* Enough space to create a free space */
          if (rest >= sizeof (Kernel_Memory_Free))
            {

              Kernel_Memory_Free * new_free ;

              new_free = (Kernel_Memory_Free *) ((uintptr_t) alloc
                                                 + sizeof (Kernel_Memory_Allocation)
                                                 + size) ;

              if (current == kfree_first)
                {
                  kfree_first = new_free ;
                  new_free->previous = new_free ;
                  new_free->next = new_free ;
                }

              else
                {
                  current->previous->next = new_free ;
                  new_free->previous = current->previous ;
                  new_free->next = current->next ;
                  current->next->previous = new_free ;
                }

              new_free->size = rest - sizeof (Kernel_Memory_Free) ;

              alloc->size = size ;
            }

          /* Delete free space (all used) */
          else
            {
              if (current == kfree_first)
                {
                  kfree_first = NULL ;
                }
              else
                {
                  current->previous->next = current->next ;
                  current->next->previous = current->previous ;
                }
            }

          /* Create a new allocations chain */
          if (kmem_first == NULL)
            {
              kmem_first = alloc ;
              alloc->previous = alloc ;
              alloc->next = alloc ;
            }

          /* Insert allocation into kernel allocations chain */
          else
            {
              Kernel_Memory_Allocation * mem_alloc ;
              mem_alloc = kmem_first ;
              while (TRUE)
                {

                  if (((uintptr_t) mem_alloc->previous < (uintptr_t) alloc
                       && (uintptr_t) alloc < (uintptr_t) mem_alloc)
                      || (mem_alloc == kmem_first
                          && (uintptr_t) alloc < (uintptr_t) mem_alloc)
                      || (mem_alloc == kmem_first
                          && (uintptr_t) mem_alloc->previous < (uintptr_t) alloc))
                    {

                      /* Insertion */
                      mem_alloc->previous->next = alloc ;
                      alloc->previous = mem_alloc->previous ;
                      alloc->next = mem_alloc ;
                      mem_alloc->previous = alloc ;

                      /* Change allocations list head */
                      if (mem_alloc == kmem_first
                          && (uintptr_t) alloc < (uintptr_t) mem_alloc)
                        kmem_first = alloc ;

                      break ;
                    }

                  mem_alloc = mem_alloc->next ;
                  if (mem_alloc == kmem_first)
                    panic ("Cannot insert in kernel memory allocations list") ;
                }
            }

          return (void *) ((uintptr_t) alloc + sizeof (Kernel_Memory_Allocation)) ;
        }

      current = current->next ;
    }
  while (current != kfree_first) ;

  /* Not enough contiguous free space */
  printf ("Not enough contiguous free space\n") ;
  return NULL ;
}

static int check_fusion (Kernel_Memory_Free * prev,
			 Kernel_Memory_Free * next)
{

  /* Cannot fusion, space between */
  if ((uintptr_t) prev
      + sizeof (Kernel_Memory_Free)
      + prev->size
      != (uintptr_t) next)
    return FALSE ;

  prev->size += sizeof (Kernel_Memory_Free) + next->size ;
  prev->next = next->next ;
  prev->next->previous = prev ;

  return TRUE ;
}

