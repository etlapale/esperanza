#ifndef __TSS_H
#define __TSS_H

#ifdef CONFIG_CPU_IA32

/**
 * The Task state segment used to switch threads.
 */
typedef struct TSS
{
  uint32_t back_link;

  /* (+4) */
  uint32_t esp0;
  /** Kernel stack segment. (+8) */
  uint32_t ss0;

  /** (+12) */
  uint32_t esp1;

  /** (+16) */
  uint32_t ss1;
  uint32_t esp2;
  uint32_t ss2;

  /** (+28) */
  uint32_t cr3;

  uint32_t eip;
  uint32_t eflags;
  uint32_t eax;
  uint32_t ecx;
  uint32_t edx;
  uint32_t ebx;
  uint32_t esp;
  uint32_t ebp;
  uint32_t esi;
  uint32_t edi;

  uint32_t es;
  uint32_t cs;
  uint32_t ss;
  uint32_t ds;
  uint32_t fs;
  uint32_t gs;

  uint32_t ldt;
  uint32_t trace_bitmap;
} __attribute__((packed)) TSS;


#elif defined CONFIG_CPU_AMD64

typedef struct TSS
{
  uint32_t reserved;

  uint64_t rsp0;
  uint64_t rsp1;
  uint64_t rsp2;
  
  uint64_t reserved2;

  uint64_t ist1;
  uint64_t ist2;
  uint64_t ist3;
  uint64_t ist4;
  uint64_t ist5;
  uint64_t ist6;
  uint64_t ist7;

  uint64_t reserved3;

  uint16_t reserved4;
  uint16_t io_map_base_address;
} __attribute__((packed)) TSS;

#endif /* CONFIG_CPU_AMD64 */

extern TSS tss;

#endif /* __TSS_H */
