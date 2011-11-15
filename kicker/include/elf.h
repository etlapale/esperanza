/**
 * \file elf.h
 * The ELF file format.
 * Based on man 5 elf.
 */
#ifndef __ELF_H
#define __ELF_H


#include <stdint.h>

/* Elf64 types */
typedef uint64_t Elf64_Addr;
typedef uint64_t Elf64_Off;
typedef uint16_t Elf64_Section;
typedef uint16_t Elf64_Versym;
typedef unsigned char Elf_Byte;
typedef uint16_t Elf64_Half;
typedef int32_t  Elf64_Sword;
typedef uint32_t Elf64_Word;
typedef int64_t  Elf64_Sxword;
typedef uint64_t Elf64_Xword;

/* Elf32 types */
typedef uint32_t Elf32_Addr;
typedef uint32_t Elf32_Off;
typedef uint16_t Elf32_Section;
typedef uint16_t Elf32_Versym;
typedef uint16_t Elf32_Half;
typedef int32_t  Elf32_Sword;
typedef uint32_t Elf32_Word;
typedef int64_t  Elf32_Sxword;
typedef uint64_t Elf32_Xword;


#define EI_NIDENT 16

/** The Elf64 header. */
typedef struct
{
  unsigned char e_ident[EI_NIDENT];
  uint16_t   etype;
  uint16_t   e_machine;
  uint32_t   e_version;
  Elf64_Addr e_entry;
  Elf64_Off  e_phoff;
  Elf64_Off  e_shoff;
  uint32_t   e_flags;
  uint16_t   e_ehsize;
  uint16_t   e_phentsize;
  uint16_t   e_phnum;
  uint16_t   e_shentsize;
  uint16_t   e_shnum;
  uint16_t   e_shstrndx;
} Elf64_Ehdr;

/** The Elf32 header. */
typedef struct
{
  unsigned char e_ident[EI_NIDENT];
  uint16_t   etype;
  uint16_t   e_machine;
  uint32_t   e_version;
  Elf32_Addr e_entry;
  Elf32_Off  e_phoff;
  Elf32_Off  e_shoff;
  uint32_t   e_flags;
  uint16_t   e_ehsize;
  uint16_t   e_phentsize;
  uint16_t   e_phnum;
  uint16_t   e_shentsize;
  uint16_t   e_shnum;
  uint16_t   e_shstrndx;
} Elf32_Ehdr;


#define EI_MAG0 0
#define ELFMAG0 0x7f
#define EI_MAG1 1
#define ELFMAG1 'E'
#define EI_MAG2 2
#define ELFMAG2 'L'
#define EI_MAG3 3
#define ELFMAG3 'F'

#define ELFMAG  "\177ELF"
#define SELFMAG 4

#define EI_CLASS 4
#define ELFCLASSNONE 0
#define ELFCLASS32   1
#define ELFCLASS64   2

#define EI_DATA 5
#define ELFDATANONE 0
#define ELFDATA2LSB 1
#define ELFDATA2MSB 2

#define EI_VERSION 6
#define EV_NONE 0
#define EV_CURRENT 1

#define EI_OSABI 7
#define ELFOSABI_NONE 0
#define ELFOSABI_SYSV 0
#define ELFOSABI_LINUX 3
#define ELFOSABI_STANDALONE 255

#define EI_ABIVERSION 8

#define EI_PAD 9

#define EI_BRAND 10

#define SHN_UNDEF     0
#define SHN_LORESERVE 1
#define SHN_LOPROC    2
#define SHN_HIPROC    3

/** Elf32 program segment header. */
typedef struct
{
  uint32_t   p_type;
  Elf32_Off  p_offset;
  Elf32_Addr p_vaddr;
  Elf32_Addr p_paddr;
  uint32_t   p_filesz;
  uint32_t   p_memsz;
  uint32_t   p_flags;
  uint32_t   p_align;
} Elf32_Phdr;

/** Elf64 program segment header. */
typedef struct
{
  uint32_t   p_type;
  uint32_t   p_flags;
  Elf64_Off  p_offset;
  Elf64_Addr p_vaddr;
  Elf64_Addr p_paddr;
  uint64_t   p_filesz;
  uint64_t   p_memsz;
  uint64_t   p_align;
} Elf64_Phdr;

#define PT_NULL    0
#define PT_LOAD    1
#define PT_DYNAMIC 2
#define PT_INTERP  3
#define PT_NOTE    4
#define PT_SHLIB   5
#define PT_PHDR    6


/** Elf32 section header. */
typedef struct
{
  uint32_t   sh_name;
  uint32_t   sh_type;
  uint32_t   sh_flags;
  Elf32_Addr sh_addr;
  Elf32_Off  sh_offset;
  uint32_t   sh_size;
  uint32_t   sh_link;
  uint32_t   sh_info;
  uint32_t   sh_addralign;
  uint32_t   sh_entsize;
} Elf32_Shdr;

/** Elf64 section header. */
typedef struct
{
  uint32_t   sh_name;
  uint32_t   sh_type;
  uint64_t   sh_flags;
  Elf64_Addr sh_addr;
  Elf64_Off  sh_offset;
  uint64_t   sh_size;
  uint32_t   sh_link;
  uint32_t   sh_info;
  uint64_t   sh_addralign;
  uint64_t   sh_entsize;
} Elf64_Shdr;

#define SHT_NULL     0
#define SHT_PROGBITS 1
#define SHT_SYMTAB   2
#define SHT_STRTAB   3
#define SHT_RELA     4
#define SHT_NOBITS   8


typedef struct
{
  uint32_t      st_name;
  Elf32_Addr    st_value;
  uint32_t      st_size;
  unsigned char st_info;
  unsigned char st_other;
  uint16_t      st_shndx;
} Elf32_Sym;

typedef struct
{
  uint32_t      st_name;
  unsigned char st_info;
  unsigned char st_other;
  uint16_t      st_shndx;
  Elf64_Addr    st_value;
  uint64_t      st_size;
} Elf64_Sym;


// Architecture dependant config

#ifdef CONFIG_CPU_IA32

#define VAL_32(ptr) (ptr)
typedef Elf32_Ehdr Elf_Ehdr;
typedef Elf32_Shdr Elf_Shdr;
typedef Elf32_Phdr Elf_Phdr;
typedef Elf32_Sym  Elf_Sym;

#elif defined CONFIG_CPU_AMD64

/** Get the 32 bits values stored at a 64 bits place. */
#define VAL_32(ptr) (*((uint32_t *) &(ptr)))
typedef Elf64_Ehdr Elf_Ehdr;
typedef Elf64_Shdr Elf_Shdr;
typedef Elf64_Phdr Elf_Phdr;
typedef Elf64_Sym  Elf_Sym;

#endif


#endif
