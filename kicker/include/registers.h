#ifndef __REGISTERS_H
#define __REGISTERS_H


#define CR0_PAGING (1UL << 31)

#define CR4_PSE (1UL << 4)
#define CR4_PAE (1UL << 5)

#define MSR_EFER 0xc0000080

#define MSR_EFER_SCE (1UL << 0)
#define MSR_EFER_LME (1UL << 8)
#define MSR_EFER_LMA (1UL << 10)


#define reg_set(reg, val) \
    asm volatile ("mov %%" #reg ", %%eax\n\t" \
                  "or  %0, %%eax\n\t" \
                  "mov %%eax, %%" #reg "\n\t" \
                  : \
                  : "i" (val))

#define reg_mask(reg, val) \
    asm volatile ("mov %%" #reg ", %%eax\n\t" \
                  "and %0, %%eax\n\t" \
                  "mov %%eax, %%" #reg "\n\t" \
                  : \
                  : "i" (~val))

#define reg_set_value(reg, val) \
    asm volatile ("mov %%eax, %%" #reg "\n\t" :: "a" (val))


inline uint32_t __attribute__((always_inline))
rdmsr (uint32_t reg)
{
  uint32_t __eax;
  
  asm volatile ("rdmsr"
                : "=a" (__eax)
                : "c" (reg));
  
  return __eax;
}

inline void __attribute__((always_inline))
wrmsr (uint32_t reg, uint32_t val)
{
  asm volatile ("wrmsr"
                :
                : "a" (val), "d" (0), "c" (reg));
}

inline void __attribute__((always_inline))
msr_set (uint32_t reg, uint32_t val)
{
  wrmsr (reg, rdmsr (reg) | val);
}

inline void __attribute__((always_inline))
msr_mask (uint32_t reg, uint32_t val)
{
  wrmsr (reg, rdmsr (reg) & (~val));
}

#endif
