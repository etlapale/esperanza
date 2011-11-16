#include <btalloc.h>
#include <kicker.h>
#include <memory.h>
#include <printf.h>
#include <registers.h>
#include <stddef.h>
#include <thread.h>
#include <tss.h>


#ifdef CONFIG_CPU_IA32
#define DEFAULT_LOAD_ADDRESS 0x8048000
#elif defined CONFIG_CPU_AMD64
#define DEFAULT_LOAD_ADDRESS 0x400000
#endif

#define SYSCALL_FLAGMASK (RFLAG_IF | RFLAG_RF | RFLAG_VM)

extern bt_allocator_t *thread_allocator;
extern bt_allocator_t pmem_allocator;
extern void init_memory ();
extern void idle (void);
#ifdef CONFIG_CPU_AMD64
extern void syscall_handler ();
#endif
extern void print_memory_map ();

void
init (struct Kicker_Info *kicker_info)
{
  /* Let the user know we are running */
  printf ("Kernel speaking!\n");
  

  printf ("pmem_allocator at: %p\n", &pmem_allocator);

  /* Init the memory */
  init_memory (kicker_info->memory_chunks, kicker_info->memory_holes);
    
#ifdef CONFIG_CPU_AMD64
  __asm__ __volatile__ ("ltr %%ax" :: "a" (0x30));
#endif

#ifdef CONFIG_CPU_IA32
  tss.ss0 = KERNEL_DATA_SEGMENT;
#endif


  /* Create the thread structures allocator */
  thread_allocator = bt_create_allocator (get_pm_page, free_memory,
					  DEFAULT_PAGE_SIZE, sizeof (thread_t));
  /* Create the first thread (the idle thread) */
  printf ("Kernel thread as cr3: 0x%x\n", kicker_info->cr3);
  current_thread = NULL;
  current_thread = new_thread ("idle",
			       (uintptr_t) idle, KERNEL_LEVEL,
			       kicker_info->cr3);
  /* Insert it in the list */
  current_thread->next = current_thread;
  current_thread->prev = current_thread;
  /* Update the tss */
#ifdef CONFIG_CPU_IA32
  tss.esp0 = current_thread->ksp;
#elif defined CONFIG_CPU_AMD64
  tss.rsp0 = current_thread->ksp;  /* FIXME: is it really needed? */
#endif
  
  /* Init the servers (skip first since it's the kernel) */
  unsigned int i;
  for (i = 1; i < kicker_info->modules_count; i++)
    {
      struct Kicker_Boot_Module *module =
	(struct Kicker_Boot_Module *) (uintptr_t) kicker_info->modules_address
	+ i;

      printf ("**Thread for server %d (%s) at 0x%x (0x%x)\n",
	      i, (char *) (uintptr_t) module->string,
	      module->start, module->end);
      
      /* TODO: ELF-loader for the servers */
      thread_t *t = new_thread ((const char*) (void*) module->string,
				DEFAULT_LOAD_ADDRESS,
				USER_LEVEL, EMPTY_ADDRESS_SPACE);

      uint32_t addr;
      for (addr = module->start; addr < module->end; addr += DEFAULT_PAGE_SIZE)
	{
	  printf ("\t(0x%x,0x%x,0x%x,+0x%x)\n",
		  module->start, addr, module->end, DEFAULT_PAGE_SIZE);
	  associate_page (t->cr3,
			  DEFAULT_LOAD_ADDRESS + (addr - module->start),
			  addr | PAGE_USER_RW);
	}

      /* Map the VIDEO page (0xb8000) at 0xb8b8000 for the console */
      associate_page (t->cr3, 0xb8b8000, 0xb8000 | PAGE_USER_RW);
    }

#ifdef CONFIG_CPU_AMD64
  /* Init the syscall handler */
  wrmsrl (MSR_STAR, ((0x18UL | 3UL) << 48) | (0x08UL << 32));
  wrmsrl (MSR_LSTAR, (uintptr_t) syscall_handler);
  wrmsrl (MSR_SFMASK, SYSCALL_FLAGMASK);
#endif
  
  /* Enable the interrupts */
  printf ("Kernel loaded!\n");
  __asm__ __volatile__ ("sti");

  idle ();
}
