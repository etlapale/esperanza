#include <gdt.h>
#include <loader.h>
#include <memory.h>
#include <printf.h>
#include <registers.h>
#include <stdint.h>


#define PAGE_PRESENT (1 << 0)
#define PAGE_READ_WRITE (1 << 1)
#define PAGE_SUPERVISOR (1 << 2)
#define PAGE_WRITETHROUGHT (1 << 3)
#define PAGE_CACHE_DISABLED (1 << 4)
#define PAGE_ACCESSED (1 << 5)
#define PAGE_DIRTY (1 << 6)
#define PAGE_SIZE (1 << 7)
#define PAGE_4K_PAT (1 << 7)
#define PAGE_GLOBAL (1 << 8)
#define PAGE_2M_PAT (1 << 12)

#define ADDRESS(x) ((x) & 0xfffff000)


#define FIRST_SEGMENT 0x08


uint32_t idle_cr3;


#ifdef CONFIG_CPU_IA32
uint32_t kernel_page_table;
#endif

static void
init_segment_descriptor (uint32_t address,
                         uint32_t size,
                         uint8_t type,
                         int segment,
                         uint8_t level,
                         int present,
                         int available,
                         int instructions_length,
                         int size_unit,
                         struct Segment_Descriptor *descriptor)
{
  descriptor->size_first = size & 0xffff;
  descriptor->address_first = address & 0xffff;
  descriptor->address_second = (address & 0xff0000) >> 16;
  descriptor->type = type & 0xf;
  descriptor->segment = segment & 1;
  descriptor->level = level & 3;
  descriptor->present = present & 1;
  descriptor->size_second = (size & 0xf0000) >> 16;
  descriptor->available = available & 1;
  descriptor->long_mode = 0;
  descriptor->instructions_length = instructions_length & 1;
  descriptor->size_unit = size_unit & 1;
  descriptor->address_third = (address & 0xff000000) >> 24;
}

/**
 * Init the tables used by the Memory Management Unit (MMU).
 */
void
init_mmu (uint32_t kload_addr, uint32_t elf_header)
{
  uint32_t *addr;
  uint32_t pml4_addr;
  struct GDTR gdtr;
  struct Segment_Descriptor *desc;
  
  /*
   * Setup a GDT that will be later used
   */

  /* Allocate a place for the GDT */
  gdtr.address = (uint32_t) get_clean_memory (DEFAULT_PAGE_SIZE);

  /* And initialize it, null descriptor */
  desc = (struct Segment_Descriptor *) gdtr.address;
  desc++;

#ifdef CONFIG_CPU_IA32
  gdtr.size = sizeof (struct Segment_Descriptor) * 9;
  /* Kernel code */
  init_segment_descriptor (0,
                           0xfffff,
                           CODE_SEGMENT,
                           SEGMENT_DESCRIPTOR,
                           KERNEL_LEVEL,
                           PRESENT,
                           0,
                           INSTRUCTIONS_32,
                           UNIT_4KB,
                           desc);
  desc++;

  init_segment_descriptor (0,
                           0xfffff,
                           DATA_SEGMENT,
                           SEGMENT_DESCRIPTOR,
                           KERNEL_LEVEL,
                           PRESENT,
                           0,
                           INSTRUCTIONS_32,
                           UNIT_4KB,
                           desc) ;
  desc++;

  init_segment_descriptor (0,
                           0xfffff,
                           CODE_SEGMENT,
                           SEGMENT_DESCRIPTOR,
                           USER_LEVEL,
                           PRESENT,
                           0,
                           INSTRUCTIONS_32,
                           UNIT_4KB,
                           desc) ;
  desc++;

  init_segment_descriptor (0,
                           0xfffff,
                           DATA_SEGMENT,
                           SEGMENT_DESCRIPTOR,
                           USER_LEVEL,
                           PRESENT,
                           0,
                           INSTRUCTIONS_32,
                           UNIT_4KB,
                           desc) ;
  desc++;

  init_segment_descriptor (0,
                           0xfffff,
                           CODE_SEGMENT,
                           SEGMENT_DESCRIPTOR,
                           USER_LEVEL,
                           PRESENT,
                           0,
                           INSTRUCTIONS_32,
                           UNIT_4KB,
                           desc) ;
  desc++;

  init_segment_descriptor (find_symbol_easily (elf_header, "tss"),
                           MINIMAL_TSS_SIZE,
                           AVAILABLE_TSS,
                           SYSTEM_DESCRIPTOR,
                           KERNEL_LEVEL,
                           PRESENT,
                           0,
                           INSTRUCTIONS_16,
                           UNIT_BYTE,
                           desc);

#elif defined CONFIG_CPU_AMD64
  gdtr.size = sizeof (struct Segment_Descriptor) * 9;

  /* Code descriptor */
  desc->present = 1;
  desc->long_mode = 1;
  desc->level = KERNEL_LEVEL;
  desc->segment = 1;
  desc->type = CODE_SEGMENT;

  desc++;
  
  desc->present = 1;
  desc->long_mode = 1;
  desc->level = KERNEL_LEVEL;
  desc->segment = 1;
  desc->type = DATA_SEGMENT;

  desc++;

  /* Code descriptor */
  desc->present = 1;
  desc->long_mode = 1;
  desc->level = USER_LEVEL;
  desc->segment = 1;
  desc->type = CODE_SEGMENT;

  desc++;
  
  desc->present = 1;
  desc->long_mode = 1;
  desc->level = USER_LEVEL;
  desc->segment = 1;
  desc->type = DATA_SEGMENT;

  desc++;
  
  desc->present = 1;
  desc->long_mode = 1;
  desc->level = USER_LEVEL;
  desc->segment = 1;
  desc->type = CODE_SEGMENT;

  desc++;
  
  init_segment_descriptor (find_symbol_easily (elf_header, "tss"),
			   MINIMAL_TSS_SIZE,
                           AVAILABLE_TSS,
                           SYSTEM_DESCRIPTOR,
                           KERNEL_LEVEL,
                           PRESENT,
                           0,
                           INSTRUCTIONS_16,
                           UNIT_BYTE,
                           desc);
#endif

  /* Load the GDT */
  __asm__ __volatile__ ("lgdt %0\n" :: "m"(gdtr));
  
#ifdef CONFIG_CPU_IA32
  /* Load the TSS */
  __asm__ __volatile__ ("ltr %%ax" :: "a" (TSS_SEGMENT));
#endif

  /*
   * We want to directly map the first megabytes of physical memory
   * to virtual memory to be used by the kernel.
   */

#ifdef CONFIG_CPU_IA32
  
  uint32_t pagedir = (uint32_t) get_clean_memory (DEFAULT_PAGE_SIZE);

#ifdef CONFIG_IA32_PSE

  *((uint32_t *) pagedir) = PAGE_GLOBAL | PAGE_SIZE | PAGE_READ_WRITE | PAGE_PRESENT;
  
  /* Enable the page size extension */
  reg_set (cr4, CR4_PSE);
  
  
#else /* ! CONFIG_IA32_PSE */
  int i;
  
  /* Allocate memory for the first page table */
  kernel_page_table = (uint32_t) get_memory (DEFAULT_PAGE_SIZE);
  *((uint32_t *) pagedir) = kernel_page_table | PAGE_READ_WRITE | PAGE_PRESENT;
  
  /* First 4MB are directly mapped on memory and reserved for the kernel usage */
  for (i = 0; i < PAGE_TABLE_ENTRY_COUNT; i++)
    *((uint32_t *) (kernel_page_table + i * PAGE_ENTRY_SIZE))
      = (i << 12) | PAGE_GLOBAL | PAGE_READ_WRITE | PAGE_PRESENT;

#endif /* #if CONFIG_IA32_PSE */

  /* Load current the page directory */
  reg_set_value (cr3, pagedir);

  idle_cr3 = pagedir;

#endif /* #if CONFIG_CPU_IA32 */

#ifdef CONFIG_CPU_AMD64

  /* Find a free page for the Page-Map Level 4 Table (PML4) */
  pml4_addr = (uint32_t) get_clean_memory (DEFAULT_PAGE_SIZE);
  reg_set_value (cr3, pml4_addr);
  //printf ("PML4: 0x%x\n", pml4_addr);
  idle_cr3 = pml4_addr;

  /* Create a Page Directory Pointer table */
  addr = (uint32_t *) pml4_addr;
  *addr = (uint32_t) get_clean_memory (DEFAULT_PAGE_SIZE)
    | PAGE_READ_WRITE | PAGE_PRESENT;
  //printf ("First PDP: 0x%x\n", *addr);

  /* Create a Page Directory table */
  addr = (uint32_t *) ADDRESS (*addr);
  *addr = (uint32_t) get_clean_memory (DEFAULT_PAGE_SIZE)
    | PAGE_READ_WRITE | PAGE_PRESENT;
  //printf ("First PD: 0x%x\n", *addr);

  /* Direct map of the first 2MB */
  addr = (uint32_t *) ADDRESS (*addr);
  *addr = PAGE_GLOBAL | PAGE_SIZE | PAGE_SUPERVISOR
    | PAGE_READ_WRITE | PAGE_PRESENT;

  /*
   * Now, let's go to Long Mode
   */

  /* Enable 64-bits page translation tables (CR4.PAE = 1) */
  reg_set (cr4, CR4_PAE);
  
  /* Enable long mode */
  msr_set (MSR_EFER, MSR_EFER_LME);

  /* TODO: add a compilation option for that */

  /* Enable syscall/sysret */
  msr_set (MSR_EFER, MSR_EFER_SCE);

#endif /* #if CONFIG_CPU_AMD64 */

  /* Enable paging to activate long mode (compatibility mode in fact) */
  reg_set (cr0, CR0_PAGING);
  asm ("jmp 1f ; 1:\n");

#ifdef CONFIG_CPU_AMD64

  /* Check if Long Mode is activated */
  if (!(rdmsr (MSR_EFER) & MSR_EFER_LME))
    panic ("Unable to switch to Long Mode");

#endif /* #if CPU_AMD64 */
}
