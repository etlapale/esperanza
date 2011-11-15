/**
 * \file kernel64.lds Kernel LD script for AMD64.
 */

OUTPUT_FORMAT("elf64-x86-64","elf64-x86-64","elf64-x86-64");

ENTRY(init)

SECTIONS
{
  . = 0;

  .text :
  {
    *(.text)
  }

  .rodata :
  {
    *(.rodata)
  }

  .data :
  {
    *(.data)
  }

  .bss :
  {
    *(.bss)
    *(COMMON)
  }

  .first_pmem_page ALIGN(0x1000) :
  {
    pmem_allocator = .;
    . += 0x1000;
  }

  .kip.data ALIGN(0x1000) :
  {
    *(.kip.data)
  }

  .kip.text :
  {
    *(.kip.text)
  }

  /DISCARD/ :
  {
    *(.note*)
    *(.comment)
  }
}
