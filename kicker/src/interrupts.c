#include <elf.h>
#include <gdt.h>
#include <io.h>
#include <loader.h>
#include <memory.h>
#include <printf.h>
#include <stddef.h>


#if defined CONFIG_CPU_IA32 || defined CONFIG_CPU_AMD64

#define FIRST_PIC 0x20
#define FIRST_PIC_ 0x21
#define SECOND_PIC 0xa0
#define SECOND_PIC_ 0xa1

#define JMP() __asm__ __volatile__ ("jmp 1f; 1:\n\t")

/**
 * Initialize the Programmable Interrupt Controller.
 */
void
init_pic (void)
{
  /* ICW1 */
  outb (FIRST_PIC, 0x11);
  JMP ();
  outb (SECOND_PIC, 0x11);
  JMP ();

  /* ICW2 */
  outb (FIRST_PIC_, 0x20);
  JMP ();
  outb (SECOND_PIC_, 0x28);
  JMP ();

  /* ICW3 */
  outb (FIRST_PIC_, 0x04);
  JMP ();
  outb (SECOND_PIC_, 0x02);
  JMP ();

  /* ICW4 */
  outb (FIRST_PIC_, 0x01);
  JMP ();
  outb (SECOND_PIC_, 0x01);
  JMP ();

  /* No mask */
  outb (FIRST_PIC_, 0x00);
  JMP ();
  outb (SECOND_PIC_, 0x00);
  JMP ();
}

void
set_pit_counter (uint16_t count)
{
  outb (0x43, 0x34);
  JMP ();
  outb (0x40, count & 0xff);
  JMP ();
  outb (0x40, count >> 8);
}

#endif /* CONFIG_CPU_IA32 || CONFIG_CPU_AMD64 */


#define INT_GATE     0x8e00
#define TRAP_GATE    0x8f00
#define TASK_GATE    0x8500
#define SYSCALL_GATE 0xee00

typedef struct
{
  uint16_t offset_first;
  uint16_t selector;
  uint16_t type;
  uint16_t offset_second;
#ifdef CONFIG_CPU_AMD64
  uint32_t offset_third;
  uint32_t reserved;
#endif
} __attribute__((packed)) IDT_Entry;

#define IDT_ENTRIES 0xff


struct {uint16_t size; uint32_t address;} __attribute__((packed)) idtr;

void
init_idt_entry (void * function,
		uint16_t selector,
		uint16_t type,
		uint32_t n)
{
  IDT_Entry *entry = (IDT_Entry *) (idtr.address + n * sizeof (IDT_Entry));
  uint32_t offset = (uint32_t) function ;
    
  entry->offset_first = offset & 0xffff ;
  entry->selector = selector ;
  entry->type = type ;
  entry->offset_second = offset >> 16 ;
}


void
init_interrupts (uint32_t kload_addr, struct Boot_Module *kernel)
{
  unsigned int i;
  Elf_Shdr *shdr;
  IDT_Entry *entry;
  uint32_t handler;
  char buff[22];

  idtr.size = sizeof (IDT_Entry) * IDT_ENTRIES;

  printf ("Initializing the interrupts\n");

#if defined CONFIG_CPU_IA32 || defined CONFIG_CPU_AMD64
  init_pic ();
#endif

  /* Search the symbol table */
  shdr = find_section ((Elf_Ehdr *) kernel->start, ".strtab");
  if (shdr == NULL)
    panic ("Symbol table not found");

  /* Init the IDT */
  idtr.address = (uint32_t) get_clean_memory (DEFAULT_PAGE_SIZE);
  printf ("\tIDT built at 0x%x\n", idtr.address);

  /* Default entries */
  handler = find_symbol_easily (kernel->start, "int_handler_default");
  if (handler == 0)
    panic ("Could not find the default interrupt handler");
  for (i = 0; i < IDT_ENTRIES; i++)
    init_idt_entry ((void *) (kload_addr + handler),
		    KERNEL_CODE_SEGMENT, INT_GATE, i);

  /* Default num entries */
  /*for (i = 0; i < IDT_ENTRIES; i++)
    {
      sprintf (buff, "int_num_handler_%d", i);
      handler = find_symbol_easily (kernel->start, buff);
      if (handler == 0)
	panic ("Could not find the %dth default handler (%s)", i, buff);
      init_idt_entry ((void *) (kload_addr + handler),
		      KERNEL_CODE_SEGMENT, INT_GATE, i);
		      }*/

#define SETUP_INT_HANDLER(name,num) \
  handler = find_symbol_easily (kernel->start, "int_handler_" name); \
  if (handler == 0) \
    panic ("Could not find " name " interrupt handler"); \
  init_idt_entry ((void *) (kload_addr + handler), \
		  KERNEL_CODE_SEGMENT, INT_GATE, num); \

  SETUP_INT_HANDLER ("zero",        0x00);

  //SETUP_INT_HANDLER ("opcode",      0x06);
  
  SETUP_INT_HANDLER ("double",      0x08);
  /* -- Cocpu seg over --           0x09 */
  SETUP_INT_HANDLER ("tss",         0x0a);
  SETUP_INT_HANDLER ("not_present", 0x0b);
  SETUP_INT_HANDLER ("stack",       0x0c);
  SETUP_INT_HANDLER ("gp",          0x0d);
  SETUP_INT_HANDLER ("page_fault",  0x0e);
  /* -- Reserved --                 0x0f */

  SETUP_INT_HANDLER ("clock",       0x20);
  SETUP_INT_HANDLER ("keyboard",    0x21);

#define SETUP_SYSCALL_HANDLER(name,num) \
  handler = find_symbol_easily (kernel->start, "syscall_" name); \
  if (handler == 0) \
    panic ("Could not find " name " syscall handler"); \
  init_idt_entry ((void *) (kload_addr + handler), \
		  KERNEL_CODE_SEGMENT, SYSCALL_GATE, num); \

#ifdef CONFIG_CPU_IA32
  SETUP_SYSCALL_HANDLER("0x80", 0x80);
#endif
    
  /* Define the Programmable interrupt controller frequency */
#ifdef CONFIG_TIMER_FREQ_100
#define CONFIG_PIT_FREQUENCY 100
#elif defined CONFIG_TIMER_FREQ_250
#define CONFIG_PIT_FREQUENCY 250
#elif defined CONFIG_TIMER_FREQ_500
#define CONFIG_PIT_FREQUENCY 500
#elif defined CONFIG_TIMER_FREQ_1000
#define CONFIG_PIT_FREQUENCY 1000
#endif
  set_pit_counter ((uint16_t) (0x1234dd / CONFIG_PIT_FREQUENCY));

  /* Load the IDTR */
  __asm__ __volatile__ ("lidt %0"::"m"(idtr):"memory","eax");
  
  printf ("Interrupts initialized\n");
}
