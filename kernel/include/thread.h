#ifndef __THREAD_H
#define __THREAD_H

#include <stdint.h>
#include <thread-bits.h>

#define KERNEL_LEVEL 0
#define USER_LEVEL 1

#define EMPTY_ADDRESS_SPACE 0

#define MAX_THREAD_NAME 32

typedef unsigned long thread_id_t;

/**
 * Kernel thread structure.
 */
typedef struct thread_t
{
  /** Counter. (IA32: +0, AMD64: +0) */
  uintmax_t counter;

  /** (IA32: +4, AMD64: +8) */
  uintptr_t cr3;

  /** (IA32: +8, AMD64: +16 */
  uintptr_t ksp;

  /** (IA32: +12, AMD64: +24 */
  uintptr_t ksp_base;

  /** Next thread. (IA32: +16, AMD64: +32) */
  struct thread_t *next;

  /** Previous thread. (IA32: +20, AMD64: +40) */
  struct thread_t *prev;

  /** Thread buffer (IA32: +24, AMD64: +48) */
  unsigned long buffer;

  /** Thread state. (IA32: +28, AMD64: +56) */
  int state;

  /** Thread name. (IA32: +32, AMD64: +60) */
  char name[MAX_THREAD_NAME];

  /** Sender of last message (IA32: +64, AMD64: +92) */
  thread_id_t from;

  /** Global thread id (IA32: +68, AMD64: +100) */
  thread_id_t global_id;

  /** Local thread id (IA32: +72, AMD64: +108) */
  thread_id_t local_id;

  /** UTCB page (IA32: +76, AMD64: +116) */
  uintptr_t utcb_page;
} __attribute__((packed)) thread_t;


/** Thead currently active. */
extern thread_t *current_thread;


/** The `idle' thread. */
extern thread_t idle_thread;


/**
 * Switch to the next thread.
 * This function is called by the clock interrupt handler when
 * current_thread->counter == 0.
 */
void
schedule (void);


/**
 * Create a new thread.
 *
 * \param name Thread name as a null-terminated string with less than
 *             MAX_THREAD_NAME characters.
 */
thread_t *
new_thread (const char* name, uintptr_t ip, int level, uintptr_t address_space);


/**
 * Create and return a new address space.
 * The first megabytes (2 for AMD64 and 4 for IA32) are mapped to the
 * kernel region.
 */
uintptr_t
create_address_space (void);


/**
 * Setup a kernel stack in a newly created thread.
 * The stack is used for context switching.
 */
void
setup_kernel_stack (thread_t *t, uintptr_t ip, int level);

#endif
