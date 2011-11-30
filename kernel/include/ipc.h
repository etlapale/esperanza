#ifndef __IPC_H
#define __IPC_H

#include <stddef.h>
#include <thread.h>
#include <thread-bits.h>

#define KEYBOARD_PORT 0x21
#define MAX_PORT_COUNT 0x100

typedef unsigned long from_specifier_t;
typedef unsigned long word_t;
typedef unsigned long msg_tag_t;

void
send (unsigned int dest, unsigned long msg);

static inline unsigned long __attribute__((always_inline))
receive (int port)
{
  unsigned long __result;

#ifdef CONFIG_CPU_IA32
  __asm__ __volatile__ ("\tpushl %%ebp\n"
			"\tmovl $0x02, %%ebp\n"
			"\tint $0x80\n"
			"\tpopl %%ebp\n"
			: "=a" (__result)
			: "d" (port)
			: /*"ebp"*/);
#elif defined CONFIG_CPU_AMD64
  __asm__ __volatile__ ("pushq %%rcx\n"
			"\tpushq %%r11\n"
			"\tpushq %%rbp\n"
			"\tmovq $0x02, %%rbp\n"
			"\tsyscall\n"
			"\tpopq %%rbp\n"
			"\tpopq %%r11\n"
			"\tpopq %%rcx\n"
			: "=a" (__result)
			: "d" (port)
			: "rcx", "r11"
//, "rbp"
);
#endif
  
  return __result;
}

static inline msg_tag_t __attribute__((always_inline))
ipc (thread_id_t to, from_specifier_t specifier,
     word_t timeouts, thread_id_t* from)
{
  msg_tag_t mr0 = 123;
  thread_id_t rfrom;

#ifdef CONFIG_CPU_IA32
#error "IA32::IPC NYI"
#elif defined CONFIG_CPU_AMD64
  // FIXME: Check that's the way to return %r9
  __asm__ __volatile__ ("pushq %%rcx\n"
			"\tpushq %%r11\n"
			"\tpushq %%rbp\n"
			"\tmovq $0x03, %%rbp\n"
			"\tsyscall\n"
			"\tpopq %%rbp\n"
			"\tpopq %%r11\n"
			"\tpopq %%rcx\n"
			: "=a"(mr0), "=S"(rfrom)
			: "S"(to), "d"(specifier)
			: "rcx", "r11");
/*asm volatile ("pushq	%%rbp		;\
	       movq	$0xface, %%rbp	;\
	       syscall			;\
	       popq	%%rbp		;\
	       popq	%%rbp		;\
	       movq     %%r9, %1	;\
              "
	      : "=m"(mr0)
	      : "d"(specifier),"S"(to)
	      : "r9");*/
#endif
  if (from != NULL)
    *from = rfrom;
  return mr0;
}

#endif

// vim: sw=2:
