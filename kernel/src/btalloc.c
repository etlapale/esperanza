/*
 * Copyright © 2006 The Next-Touch Organization
 *
 *   Neil Dökkalfar <neil@lyua.org>
 *
 * 
 */

#include <stddef.h>
#include <stdint.h>


/** TODO: try not searching in the whole page list */
/** TODO: free the pages when they are empty */
/** TODO: better (real) calculation for the max number of chunks */


struct __bt_allocator 
{
  struct __bt_allocator *next;
  void * (*page_alloc) (void);
  void (*page_free) (void *);
  size_t page_size;
  size_t chunk_size;
  size_t longs;
  size_t chunks;
};


typedef struct __bt_allocator bt_allocator_t;


void
bt_create_allocator_with_first (void *first_page,
				void * (*page_alloc) (void),
				void (*page_free) (void *),
				size_t page_size,
				size_t chunk_size)
{
  bt_allocator_t * allocator;
  
  /* allocator informations */
  allocator = (bt_allocator_t *) first_page;
  allocator->next = NULL;
  allocator->page_alloc = page_alloc;
  allocator->page_free = page_free;
  allocator->page_size = page_size;
  allocator->chunk_size = chunk_size;

  /* TODO: we are loosing a little space 'cause i'm lazy */
  int n = (page_size - sizeof (bt_allocator_t)) / chunk_size;

  /* Number of unsigned longs for the bitmap */
  int b = n / (sizeof (unsigned long) * 8);
  if (n % (sizeof (unsigned long) * 8)) b++;
  allocator->longs = b;
  
  /* Max number of chunks */
  n -= b * sizeof (unsigned long) / chunk_size;
  if (b * sizeof (unsigned long) % chunk_size) n--;
  allocator->chunks = n;

  /* Init the bitmap to zero */
  for (n = 0; n < b; n++)
    *((unsigned long *) ((void *) allocator + sizeof (bt_allocator_t)) + n) =0;
}


bt_allocator_t *
bt_create_allocator (void * (*page_alloc) (void),
		     void (*page_free) (void *),
		     size_t page_size,
		     size_t chunk_size)
{
  void *first_page = page_alloc ();

  if (first_page)
    bt_create_allocator_with_first (first_page,
				    page_alloc, page_free,
				    page_size, chunk_size);
  return first_page;
}


void
bt_destroy_allocator (bt_allocator_t *allocator)
{
  if (allocator->next)
    bt_destroy_allocator (allocator->next);
  allocator->page_free (allocator);
}


void *
bt_alloc (bt_allocator_t *allocator)
{
  if (allocator == NULL)
    return NULL;

  /* Search the good  unsigned long */
  size_t chunk = 0;
  size_t i;
  for (i = 0; i < allocator->longs; i++)
    {
      unsigned long *ptr = (unsigned long *)
	((void *) allocator + sizeof (bt_allocator_t)) + i;
      unsigned long l = *ptr;

      /* There is a 0-bit */
      if (l != ~0UL)
	{
	  unsigned int bit;
	  for (bit = 0; bit < (sizeof (unsigned long) * 8)
		 && chunk < allocator->chunks; bit++)
	    {
	      if ((l & 1UL) == 0)
		{
		  *ptr |= (1L << bit);
		  return (void *) allocator
		    + sizeof (bt_allocator_t)
		    + allocator->longs * sizeof (unsigned long)
		    + chunk * allocator->chunk_size;
		}

	      l = l >> 1;
	      chunk++;
	    }
	}
    }

  /* Search in next allocator (creating it if it does not exists) */
  if (allocator->next == NULL)
    allocator->next = bt_create_allocator (allocator->page_alloc,
					   allocator->page_free,
					   allocator->page_size,
					   allocator->chunk_size);

  return bt_alloc (allocator->next);
}

void
bt_free (bt_allocator_t *allocator, void *chunk)
{
  /* Not on that page */
  if (chunk < (void *) allocator
      || (void *) allocator + allocator->page_size < chunk)
    {
      if (allocator->next)
	bt_free (allocator->next, chunk);
      return;
    }

  size_t offset = chunk - (void *) allocator;
  size_t header = sizeof (bt_allocator_t)
    + allocator->longs * sizeof (unsigned long);

  /* Check it's a chunk */
  if (offset < header
      || (offset - header) % allocator->chunk_size != 0
      || offset + allocator->chunk_size > allocator->page_size)
      return;

  /* Get the chunk number */
  offset = (offset - header) / allocator->chunk_size;

  /* MODIFY the bit */
  unsigned long *ptr = (unsigned long *) 
    ((void *) allocator + sizeof (bt_allocator_t))
    + (offset / (sizeof (unsigned long) * 8));
  *ptr = (*ptr) & ~(1UL << (offset % (sizeof (unsigned long) * 8)));
}
