#include <kicker.h>
#include <loader.h>
#include <malloc.h>
#include <memory.h>
#include <multiboot.h>
#include <printf.h>
#include <stdint.h>


#define KICKER_MAJOR_VERSION 0
#define KICKER_MINOR_VERSION 1

extern void
init_mmu (uint32_t kload_addr, uint32_t elf_header);


/* The values are loaded by the assembly code in crt0.S */
uint32_t multiboot_magic;
multiboot_info_t *multiboot_info;
extern uint32_t idle_cr3;

/* Contains the segment and address of the kernel */
static struct { uint32_t addr; uint16_t seg; } ksegaddr;

int
main (uintptr_t heap_addr, uintmax_t heap_size)
{
  struct Boot_Module *modules;
  /* Kicker information passed to the kernel */
  struct Kicker_Info *info;

  /* Init the print-on-screen function */
  init_printf ();

  /* Say hello to the world */
  printf ("Kicker %d.%d loaded by %s\n",
          KICKER_MAJOR_VERSION, KICKER_MINOR_VERSION,
          (char *) multiboot_info->boot_loader_name);

  /* Init the dynamic memory allocator with its heap */
  init_malloc (heap_addr, heap_size);

  /* Check we are booting from a multiboot bootloader */
  if (multiboot_magic != MULTIBOOT_BOOTLOADER_MAGIC)
    panic ("Not a multiboot bootloader");

  info = (struct Kicker_Info *) malloc (sizeof (struct Kicker_Info));
  printf ("Kicker info allocated at 0x%x\n", (uint32_t) (void*) info);

  /* Save the modules (loaded by the bootloader) informations */
  modules = save_modules_info (multiboot_info, info);

  /* Init the physical page allocator */
  init_memory (multiboot_info);

  /* Load the kernel */
  ksegaddr.seg = 0x08;
  ksegaddr.addr = load_kernel (modules);
  printf ("Ksegaddr at 0x%x [0x%x,0x%x]\n",
	  (uint32_t) (void*) &ksegaddr, ksegaddr.seg, ksegaddr.addr);

  /* Init the Memory Management Unit */
  init_mmu (ksegaddr.addr, modules->start);

  /* Init the interrupts */
  init_interrupts (ksegaddr.addr, modules);

  /* New kernel heap */
  info->heap_address = (uintptr_t) get_memory (DEFAULT_PAGE_SIZE);
  info->heap_size = DEFAULT_PAGE_SIZE;

  /* Init the kernel */
  init_kernel (modules, ksegaddr.addr);

  info->memory_chunks = (uint32_t) memory_chunks;
  info->memory_holes = (uint32_t) memory_holes;
  
  info->cr3 = idle_cr3;

  /* TODO: Delete : free the init resources */

  /* Switch the kernel */
#ifdef CONFIG_CPU_IA32
  /*__asm__ __volatile__ ("pushl %0; pushl $0; ljmp *%1"
			:: "m"(info), "m"(ksegaddr));*/
  __asm__ __volatile__ ("pushl %0; pushl $0" :: "m"(info));
#elif defined CONFIG_CPU_AMD64
  __asm__ __volatile__ ("movl %0, %%edi" :: "m" (info));
#endif
  __asm__ __volatile__ ("ljmp *%0" :: "m"(ksegaddr));

  return 0;
}
