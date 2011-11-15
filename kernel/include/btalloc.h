/**
 * \file btalloc.h Bitmap-based fixed-size memory allocator.
 *
 * The defined allocator could be useful if you need to dynamically
 * allocate fixed-sized chunks. It uses a page allocator from which
 * it grabs big-chunks of memory (pages) which are then splitted into
 * multiple same-size pieces. When the page is full, it automatically
 * starts a new one using the page allocating function you provided.
 *
 * \author Neil Dokkalfar <neil@lyua.org>
 */

#ifndef __BT_ALLOC_H
#define __BT_ALLOC_H

#include <stdint.h>


/**
 * bt_allocator_t is an opaque structure.
 */
typedef void * bt_allocator_t;


/**
 * Create a new bt_allocator_t relying on the given page allocator.
 * \param chunk_size Chunk size must neither be null nor too big.
 */
bt_allocator_t *
bt_create_allocator (void * (*page_alloc) (void),
		     void (*page_free) (void *),
		     size_t page_size,
		     size_t chunk_size);

/**
 * This is like bt_create_allocator except that the first page is
 * explicitely provided.
 */
void
bt_create_allocator_with_first (void *first_page,
				void * (*page_alloc) (void),
				void (*page_free) (void *),
				size_t page_size,
				size_t chunk_size);

/**
 * Destroy the given allocator, as well as all the data it contains.
 * The created pages are also freed using the function you provided.
 */
void
bt_destroy_allocator (bt_allocator_t *allocator);


/**
 * Allocate a new memory chunk in the given allocator.
 * If there is no more space in the current page, a new one is created.
 */
void *
bt_alloc (bt_allocator_t *allocator);


/**
 * Free the memory chunk at the given address in an allocator.
 */
void
bt_free (bt_allocator_t *allocator, void *item);

#endif
