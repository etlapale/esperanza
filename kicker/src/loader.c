#include <elf.h>
#include <gdt.h>
#include <loader.h>
#include <memory.h>
#include <printf.h>
#include <string.h>

extern uint32_t kernel_page_table;

uint32_t
load_kernel (struct Boot_Module *kernel)
{
  Elf_Ehdr *ehdr = (Elf_Ehdr *) kernel->start;
  Elf_Phdr *phdr = (Elf_Phdr *) (VAL_32 (ehdr->e_phoff) + kernel->start);
  unsigned int i;

  printf ("Loading the kernel\n");


  /* Check the ELF header */
  if (memcmp (ehdr, ELFMAG, SELFMAG))
    panic ("Invalid kernel format: ELF magic not found");
#ifdef CONFIG_CPU_IA32
  if (ehdr->e_ident[EI_CLASS] != ELFCLASS32)
    panic ("Not a 32-bits ELF kernel");
#elif defined CONFIG_CPU_AMD64
  if (ehdr->e_ident[EI_CLASS] != ELFCLASS64)
    panic ("Not a 64-bits ELF kernel");
#endif
  

  /* Load the ELF program segments */
  for (i = 0; i < ehdr->e_phnum; i++)
    {
      uint32_t addr = VAL_32 (phdr->p_vaddr);
      uint32_t fsize = VAL_32 (phdr->p_filesz);
      uint32_t msize = VAL_32 (phdr->p_memsz);

      if (phdr->p_type == PT_LOAD)
	{
	  printf ("\tLoading program segment %u at 0x%x (0x%x-0x%x)\n",
		  i, addr, fsize, msize);
	  mark_memory_as_used (addr, msize);
	  /* Copy the kernel */
	  memcpy ((void *) addr,
		  (void *) (kernel->start + VAL_32 (phdr->p_offset)),
		  fsize);
	  /* Extra bytes */
	  if (msize > fsize)
	    memset ((void *) (addr + fsize), 0, msize - fsize);
	}
      else
	{
	  printf ("\tSkipped program segment %u (type: 0x%x)\n",
		  i, phdr->p_type);
	}

      phdr++;
    }

  printf ("Kernel loaded\n");
  
  return VAL_32 (ehdr->e_entry);
}

Elf_Shdr *
find_section (Elf_Ehdr *ehdr, const char *name)
{
  unsigned int i;
  Elf_Shdr *shstrtab, *ans;
  uint32_t shstraddr;

  /* Find the section containing the section names */
  if (ehdr->e_shstrndx == SHN_UNDEF)
    panic ("No section for section names");

  shstrtab = (Elf_Shdr *) ((uint32_t) ehdr + VAL_32 (ehdr->e_shoff))
    + ehdr->e_shstrndx;
  shstraddr = shstrtab->sh_offset + (uint32_t) ehdr;

  /* Check if we want the .shstrtab */
  if (strcmp (name, ".shstrtab") == 0)
    return (Elf_Shdr *) shstrtab;

  /* Search the section */
   for (i = 0, ans = (Elf_Shdr *) ((uint32_t) ehdr + VAL_32 (ehdr->e_shoff));
       i < ehdr->e_shnum;
       i++, ans++)
       if (strcmp (name, (char *) (shstraddr + ans->sh_name)) == 0)
	return ans;
 	
  /* Not found */
  return NULL;
}

uintptr_t
find_symbol (uint32_t kstart, Elf_Shdr *symbols,
	     Elf_Shdr *names, const char *symname)
{
  Elf_Sym *sym;

  sym = (Elf_Sym *) (kstart + VAL_32 (symbols->sh_offset));
  while ((uint32_t) sym - kstart - symbols->sh_offset < symbols->sh_size)
    {
      if (strcmp ((char *) (kstart + VAL_32 (names->sh_offset) + sym->st_name),
		  symname) == 0)
	return sym->st_value;
      sym++;
    }

  /* Not found */
  return 0;
}

uintptr_t
require_symbol (uintptr_t kstart, Elf_Shdr *symbols,
		Elf_Shdr *names, const char *symname)
{
  uintptr_t ans = find_symbol (kstart, symbols, names, symname);
  if (ans == 0)
    panic ("Symbol not found (%s)", symname);
  return ans;
}

uint32_t
find_symbol_easily (uint32_t kstart, const char *symname)
{
  
  Elf_Shdr *symbols, *names;

  /* Search the symbol table */
  symbols = find_section ((Elf_Ehdr *) kstart, ".symtab");
  names = find_section ((Elf_Ehdr *) kstart, ".strtab");
  if (symbols == NULL || names == NULL)
    panic ("Symbol tables not found");

  return find_symbol (kstart, symbols, names, symname);
}

void
init_kernel (struct Boot_Module *kernel, uint32_t kernel_load_address)
{
  Elf_Shdr *symbols, *names;
  uint32_t idle, idle2, tss;

  /* Search the symbol table */
  symbols = find_section ((Elf_Ehdr *) kernel->start, ".symtab");
  names = find_section ((Elf_Ehdr *) kernel->start, ".strtab");
  if (symbols == NULL || names == NULL)
    panic ("Symbol tables not found");

#define KERNEL_SET_SYMBOL(symbol,value)				\
  * ((uint32_t *) (kernel_load_address + require_symbol		\
		   (kernel->start, symbols, names, (symbol))))	\
    = (value)

#if (defined CONFIG_CPU_IA32) && (! defined CONFIG_IA32_PSE)
  KERNEL_SET_SYMBOL ("kernel_page_table", kernel_page_table);
#endif

#ifdef CONFIG_SCREEN_DEBUG
  /* Set the kernel console cursor to the kicker console cursor */
  KERNEL_SET_SYMBOL ("console_video", VIDEO_ADDRESS);
  KERNEL_SET_SYMBOL ("console_xpos", console_xpos);
  KERNEL_SET_SYMBOL ("console_ypos", console_ypos);
#endif
}
