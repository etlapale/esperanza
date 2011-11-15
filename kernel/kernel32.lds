/**
 * \file kernel32.lds Kernel LD script for IA32.
 */

OUTPUT_FORMAT("elf32-i386","elf32-i386","elf32-i386");

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
