#include <btalloc.h>
#include <memory.h>
#include <printf.h>
#include <stddef.h>
#include <thread.h>
#include <tss.h>


// TODO: check the stacks!

TSS tss;

bt_allocator_t *thread_allocator;


#if defined CONFIG_CPU_IA32 && ! defined CONFIG_IA32_PSE
uint32_t kernel_page_table;
#endif


#ifdef CONFIG_CPU_IA32
#ifdef CONFIG_IA32_SYSENTER
#define software_task_switch(old_thread, new_thread)	                \
  __asm__ volatile ("pushfl\n\t"					\
		    "pushl $0x08\n\t"					\
		    "pushl $1f\n\t"					\
		    "pushal\n\t"					\
									\
		    "pushw %ds\n\t"					\
		    "pushw %es\n\t"					\
		    "pushw %fs\n\t"					\
		    "pushw %gs\n\t"					\
									\
		    /* Save ksp */					\
		    "mov old_thread, %ebx\n\t"				\
		    "mov %esp, 12(%ebx)\n\t"				\
									\
		    /* Restore ksp */					\
		    "mov current_thread, %ebx\n\t"			\
		    "mov 12(%ebx), %esp\n\t"				\
									\
		    /* Load the new cr3 */				\
		    "mov 8(%ebx), %eax\n\t"				\
		    "mov %eax, %cr3\n\t"				\
									\
		    /* Put new ksp in the tss */			\
		    "mov 16(%ebx), %eax\n\t"				\
		    "mov $tss, %ebx\n\t"				\
		    "mov %eax, 4(%ebx)\n\t"				\
									\
		    /* Put new ksp in the SYSENTER_ESP_MSR (0x175) */	\
		    "mov $0, %edx\n\t"					\
		    "mov $0x175, %ecx\n\t"				\
		    "wrmsr\n\t"						\
									\
		    /* Restore registers and flags from the stack */	\
		    "popw %gs\n\t"					\
		    "popw %fs\n\t"					\
		    "popw %es\n\t"					\
		    "popw %ds\n\t"					\
		    "popal\n\t"						\
		    "iret\n\t"						\
		    "1:\n\t");
#else /* ! CONFIG_IA32_SYSENTER */
#define software_task_switch(old_thread, new_thread)						\
  __asm__ volatile ("pushfl\n\t"					\
		    "pushl $0x08\n\t"					\
		    "pushl $1f\n\t"					\
		    "pushal\n\t"					\
									\
		    "pushw %%ds\n\t"					\
		    "pushw %%es\n\t"					\
		    "pushw %%fs\n\t"					\
		    "pushw %%gs\n\t"					\
									\
		    /* Save ksp */					\
		    "movl %0, %%ebx\n\t"				\
		    "mov %%esp, 8(%%ebx)\n\t"				\
									\
		    /* Restore ksp */					\
		    "mov %1, %%ebx\n\t"			\
		    "mov 8(%%ebx), %%esp\n\t"				\
									\
		    /* Load the new cr3 */				\
		    "mov 4(%%ebx), %%eax\n\t"				\
		    "mov %%eax, %%cr3\n\t"				\
									\
		    /* Put the new ksp_base in the tss */		\
		    "mov 12(%%ebx), %%eax\n\t"				\
		    "mov $tss, %%ebx\n\t"				\
		    "mov %%eax, 4(%%ebx)\n\t"				\
									\
		    /* Restore registers and flags from the stack */	\
		    "popw %%gs\n\t"					\
		    "popw %%fs\n\t"					\
		    "popw %%es\n\t"					\
		    "popw %%ds\n\t"					\
		    "popal\n\t"						\
		    "iret\n\t"						\
		    "1:\n\t" :: "m"(old_thread), "m"(new_thread));

#endif /* CONFIG_IA32_SYSENTER */
#elif defined CONFIG_CPU_AMD64
#define software_task_switch(old_thread, new_thread)     \
    __asm__ __volatile__ ("pushq  $0               \n\t" \
                          "pushq  %%rsp            \n\t" \
			  "pushfq                  \n\t" \
                          "pushq  $0x08            \n\t" \
                          "pushq  $1f              \n\t" \
                                                         \
                          /* Save the registers */       \
                          "pushq  %%rax            \n\t" \
                          "pushq  %%rbx            \n\t" \
                          "pushq  %%rcx            \n\t" \
                          "pushq  %%rdx            \n\t" \
                          "pushq  %%rbp            \n\t" \
                          "pushq  %%rdi            \n\t" \
                          "pushq  %%rsi            \n\t" \
                          "pushq  %%r8             \n\t" \
                          "pushq  %%r9             \n\t" \
                          "pushq  %%r10            \n\t" \
                          "pushq  %%r11            \n\t" \
                          "pushq  %%r12            \n\t" \
                          "pushq  %%r13            \n\t" \
                          "pushq  %%r14            \n\t" \
                          "pushq  %%r15            \n\t" \
                          "pushw  %%fs             \n\t" \
                          "pushw  %%gs             \n\t" \
                          "subq   $4, %%rsp        \n\t" \
                                                         \
                          /* Save rsp0 */                \
                          "movq   %0, %%rbx        \n\t" \
                          "movq   %%rsp, 16(%%rbx) \n\t" \
                                                         \
                          /* Load the new ksp */         \
                          "movq   %1, %%rbx        \n\t" \
			  "movq   16(%%rbx), %%rsp \n\t" \
                                                         \
                          /* Load the address space */   \
                          "movq   8(%%rbx), %%rax  \n\t" \
                          "movq   %%rax, %%cr3     \n\t" \
                                                         \
                          /* Put rsp0_base in tss */     \
                          "movq   24(%%rbx), %%rax \n\t" \
                          "movq   $tss, %%rbx      \n\t" \
                          "movq   %%rax, 4(%%rbx)  \n\t" \
                                                         \
                          /* Restore the registers */    \
                          "addq   $4, %%rsp        \n\t" \
                          "popw   %%gs             \n\t" \
                          "popw   %%fs             \n\t" \
                          "popq   %%r15            \n\t" \
                          "popq   %%r14            \n\t" \
                          "popq   %%r13            \n\t" \
                          "popq   %%r12            \n\t" \
                          "popq   %%r11            \n\t" \
                          "popq   %%r10            \n\t" \
                          "popq   %%r9             \n\t" \
                          "popq   %%r8             \n\t" \
                          "popq   %%rsi            \n\t" \
                          "popq   %%rdi            \n\t" \
                          "popq   %%rbp            \n\t" \
                          "popq   %%rdx            \n\t" \
                          "popq   %%rcx            \n\t" \
                          "popq   %%rbx            \n\t" \
                          "popq   %%rax            \n\t" \
                                                         \
                          /* Switch back */              \
                          "iretq                   \n\t" \
			  "1:                      \n\t" \
                          :: "m"(old_thread), "m"(new_thread));
#endif


thread_t *current_thread;
thread_t *old_thread;

#define DEFAULT_THREAD_PRIORITY 10

void
schedule (void)
{
  old_thread = current_thread;

  do
    {
      current_thread = current_thread->next;
    }
  while (current_thread != old_thread && current_thread->state != THREAD_RUNNING);
  
  current_thread->counter = DEFAULT_THREAD_PRIORITY;

  /* Same thread, do not switch */
  if (current_thread == old_thread)
    {
      /*printf ("Same thread (0x%lx)\n", (unsigned long) current_thread);*/
      return;
    }

  /* Switch */
  /*printf ("Switching from 0x%lx to 0x%lx\n",
    (unsigned long) old_thread, (unsigned long) current_thread);*/
  software_task_switch (old_thread, current_thread);
}

thread_t *
new_thread (uintptr_t ip, int level, uintptr_t address_space)
{
  uintptr_t kstack;
  uintptr_t *ptr;
  
  /* Kernel stack at kernel level*/
  kstack = (uintptr_t) get_memory (DEFAULT_PAGE_SIZE);
  printf ("\tKernel stack at %p\n", (void *) kstack);
  
  thread_t *t = (thread_t *) bt_alloc (thread_allocator);
  t->ksp_base = kstack + DEFAULT_PAGE_SIZE;
  t->ksp = t->ksp_base;
  
  /* Process schedule informations */
  t->counter = DEFAULT_THREAD_PRIORITY;
  t->state = THREAD_RUNNING;
  
  /* Address space */
  if (address_space == EMPTY_ADDRESS_SPACE)
    t->cr3 = create_address_space ();
  else
    t->cr3 = address_space;

  /* Init the kernel stack for first context switch */
  setup_kernel_stack (t, ip, level);
      
  /* Insert it in the list */
  if (current_thread)
    {
      current_thread->prev->next = t;
      t->prev = current_thread->prev;
      t->next = current_thread;
      current_thread->prev = t;
    }
  
  printf ("\tNew thread: %p (CR3: %p)\n", t, (void *) t->cr3);
  return t;
}

void
delete_thread (thread_t *t)
{
  printf ("Destroying thread %p (prev: %p, next: %p)\n",
	  t, t->prev, t->next);

  /* Remove it from the list */
  t->prev->next = t->next;
  t->next->prev = t->prev;

  /* Free the used memory */
  
  /* Free the thread structure */
  bt_free (thread_allocator, t);
  
  /* Schedule to the next thread */
  /* We trust the structures aren't modified after be freed */
  current_thread = t->next;
  software_task_switch (t, current_thread);
}


/**
 * Create and return a new address space.
 * The first megabytes (2 on AMD64, 4 on IA32) are directly mapped to the
 * kernel region.
 */
uintptr_t
create_address_space (void)
{
  uintptr_t space = (uintptr_t) get_clean_memory (DEFAULT_PAGE_SIZE);

#ifdef CONFIG_CPU_IA32

  /* On IA32, `space' is the Page Directory Table */

#ifdef CONFIG_IA32_PSE
  /* 4MB page */
  *((uintptr_t *) space) = PAGE_GLOBAL | PAGE_SIZE | PAGE_READ_WRITE | PAGE_PRESENT;
#else /* ! CONFIG_IA32_PSE */
  /* First Page Table is the kernel's one */
  *((uintptr_t *) space) = kernel_page_table | PAGE_READ_WRITE | PAGE_PRESENT;
#endif /* ! CONFIG_IA32_PSE */


#elif defined CONFIG_CPU_AMD64

  /* On AMD64, `space' is the PML-4 table */

  /* Create a Page Directory Pointer table */
  uintptr_t *addr = (uintptr_t *) space;
  *addr = (uintptr_t) get_clean_memory (DEFAULT_PAGE_SIZE)
    | PAGE_USER_LEVEL | PAGE_READ_WRITE | PAGE_PRESENT;
  
  /* Create a Page Directory table */
  addr = (uintptr_t *) PAGE_START_ADDRESS (*addr);
  *addr = (uintptr_t) get_clean_memory (DEFAULT_PAGE_SIZE)
    | PAGE_USER_LEVEL | PAGE_READ_WRITE | PAGE_PRESENT;
  
  /* Direct map of the first 2MB */
  addr = (uintptr_t *) PAGE_START_ADDRESS (*addr);
  *addr = PAGE_GLOBAL | PAGE_SIZE
    | PAGE_READ_WRITE | PAGE_PRESENT;
  
#endif /* CONFIG_CPU_AMD64 */
  
  
  return space;
}


void
setup_kernel_stack (thread_t *t, uintptr_t ip, int level)
{  
  uintptr_t *ptr;

#ifdef CONFIG_CPU_IA32
  uint16_t *ptr2;
      
  /*
   * Define a stack layout which can be used in the schedule() function,
   * during the stack switch.
   *
   *   ss3    (only for user-level threads)
   *   esp3   (only for user-level threads)
   *   eflags
   *   cs
   *   eip
   *   eax
   *   ecx
   *   edx
   *   ebx
   *   esp
   *   ebp
   *   esi
   *   edi
   *   ds
   *   es
   *   fs
   *   gs
   */
      
  ptr = (uintptr_t *) t->ksp;
  ptr--;
      
  /* ss3:esp3 to switch stacks during iret */
  if (level == USER_LEVEL)
    {
      *ptr = USER_DATA_SEGMENT | USER_LEVEL_SEGMENT; /* ss3 */
      ptr--;
      *ptr = 0xcafebabe; /* We don't manage user-level stacks */
      ptr--;
    }
      
  /* EFLAGS: For now, all threads can access the IO ports. */
  /* TODO: implement IO-flexpages and put this to 0x212 */
  *ptr = 0x3212;
  ptr--;
  if (level == KERNEL_LEVEL)
    *ptr = KERNEL_CODE_SEGMENT; /* cs */
  else
    *ptr = USER_CODE_SEGMENT | USER_LEVEL_SEGMENT; /* cs */

  ptr--;
  *ptr = ip; /* eip */
  ptr--;
  *ptr = 0; /* eax */
  ptr--;
  *ptr = 0; /* ecx */
  ptr--;
  *ptr = 0; /* edx */
  ptr--;
  *ptr = 0; /* ebx */
  ptr--;
  *ptr = t->ksp_base; /* esp */
  ptr--;
  *ptr = t->ksp; /* ebp */
  ptr--;
  *ptr = 0; /* esi */
  ptr--;
  *ptr = 0; /* edi */
  ptr2 = (uint16_t *) ptr;
  ptr2--;
  if (level == KERNEL_LEVEL)
    *ptr2 = KERNEL_DATA_SEGMENT; /* ds */
  else
    *ptr2 = USER_DATA_SEGMENT | USER_LEVEL; /* ds */
  ptr2--;
  *ptr2 = *(ptr2 + 1); /* es */
  ptr2--;
  *ptr2 = *(ptr2 + 1); /* fs */
  ptr2--;
  *ptr2 = *(ptr2 + 1); /* gs */
      
  if (level == KERNEL_LEVEL)
    t->ksp -= 52;
  else
    t->ksp -= 60;

#elif defined CONFIG_CPU_AMD64

  uint32_t *ptr2;
      
  /*
   * Define a stack layout which can be used in the schedule() function,
   * during the stack switch.
   *
   *   ss
   *   rsp
   *   rflags
   *   cs
   *   rip
   *   rax
   *   rbx
   *   rcx
   *   rdx
   *   rbp
   *   rdi
   *   rsi
   *   r8
   *   r9
   *   r10
   *   r11
   *   r12
   *   r13
   *   r14
   *   r15
   *   fs
   *   gs
   *   xx xx (32 bits padding)
   */
      
  ptr = (uintptr_t *) t->ksp;
  ptr--;
      
  /* ss:esp to switch stacks during iret */
  if (level == KERNEL_LEVEL)
    {
      *ptr = 0x00;
      //*ptr = KERNEL_DATA_SEGMENT; /* ss */
      ptr--;
      *ptr = t->ksp; /* TODO: check this is correct! */
    }
  else
    {
      *ptr = USER_DATA_SEGMENT | USER_LEVEL_SEGMENT; /* ss */
      ptr--;
      *ptr = 0xcafebabe; /* We don't manage user-level stacks */
    }
  ptr--;
      
  /* RFLAGS: For now, all threads can access the IO ports. */
  /* TODO: implement IO-flexpages and put this to 0x212 */
  *ptr = 0x3212;
  ptr--;

  if (level == KERNEL_LEVEL)
    *ptr = KERNEL_CODE_SEGMENT; /* cs */
  else
    *ptr = USER_CODE_SEGMENT | USER_LEVEL_SEGMENT; /* cs */
  ptr--;
  *ptr = ip; /* rip */
  ptr--;

  *ptr = 0; /* rax */
  ptr--;
  *ptr = 0; /* rbx */
  ptr--;
  *ptr = 0; /* rcx */
  ptr--;
  *ptr = 0; /* rdx */
  ptr--;
  *ptr = 0; /* rbp */
  ptr--;
  *ptr = 0; /* rdi */
  ptr--;
  *ptr = 0; /* rsi */
  ptr--;
  *ptr = 0; /* r8 */
  ptr--;
  *ptr = 0; /* r9 */
  ptr--;
  *ptr = 0; /* r10 */
  ptr--;
  *ptr = 0; /* r11 */
  ptr--;
  *ptr = 0; /* r12 */
  ptr--;
  *ptr = 0; /* r13 */
  ptr--;
  *ptr = 0; /* r14 */
  ptr--;
  *ptr = 0; /* r15 */
      
  ptr2 = (uint32_t *) ptr;
  ptr2--;
  *ptr2 = *(ptr2 + 1); /* fs */
  ptr2--;
  *ptr2 = *(ptr2 + 1); /* gs */
      
  t->ksp -= 168;
#endif
}
