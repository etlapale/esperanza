#ifndef __LOADER_H
#define __LOADER_H

#include <elf.h>
#include <kicker.h>
#include <multiboot.h>
#include <stdint.h>


/** Saved boot module information. */
struct Boot_Module
{
  uint32_t start;
  uint32_t end;
  char *string;
};


/**
 * Save the modules informations from the Multiboot.
 */
struct Boot_Module *
save_modules_info (multiboot_info_t *mbi, struct Kicker_Info *info);


/**
 * Load the kernel and return its entry point address.
 *
 * \param The module information.
 * \return The kernel entry point.
 */
uint32_t
load_kernel (struct Boot_Module *kernel);


/**
 * Initialize the interrupts with the kernel functions.
 */
void
init_interrupts (uint32_t kload_addr, struct Boot_Module *kmod);

void
init_kernel (struct Boot_Module *kernel, uint32_t kernel_load_address);


Elf_Shdr *
find_section (Elf_Ehdr *ehdr, const char *name);


/**
 * Return the offset of the given symbol in the kernel.
 */
uint32_t
find_symbol_easily (uint32_t kstart, const char *symbol_name);

#endif
