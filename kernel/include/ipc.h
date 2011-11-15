#ifndef __IPC_H
#define __IPC_H

#define KEYBOARD_PORT 0x21
#define MAX_PORT_COUNT 0x100

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
			: "ebp");
#elif defined CONFIG_CPU_AMD64
  __asm__ __volatile__ ("pushq %%rcx\n"
			"\tpushq %%r11\n"
			"\tpushq %%rbp\n"
			"\tmovl $0x02, %%ebp\n"
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

#endif
