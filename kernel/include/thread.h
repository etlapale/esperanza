#ifndef __THREAD_H
#define __THREAD_H

#include <stdint.h>

#define KERNEL_LEVEL 0
#define USER_LEVEL 1

#define EMPTY_ADDRESS_SPACE 0

/** Thread states. */
#define THREAD_STOPPED 0
#define THREAD_RUNNING 1
#define THREAD_SENDING 2
#define THREAD_RECEIVING 3
#define THREAD_DEAD 4

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
 */
thread_t *
new_thread (uintptr_t ip, int level, uintptr_t address_space);


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
