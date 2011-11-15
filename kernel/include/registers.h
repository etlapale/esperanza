#ifndef __REGISTERS_H
#define __REGISTERS_H

/* RFLAGS flags */
#define RFLAG_CF  (1UL << 0)
#define RFLAG_PF  (1UL << 2)
#define RFLAG_AF  (1UL << 4)
#define RFLAG_IF  (1UL << 9)
#define RFLAG_DF  (1UL << 10)
#define RFLAG_RF  (1UL << 16)
#define RFLAG_VM  (1UL << 17)
#define RFLAG_VIF (1UL << 19)
#define RFLAG_VIP (1UL << 20)
#define RFLAG_ID  (1UL << 21)

/* MSR registers */
#define MSR_TSC            0x00000010
#define MSR_MTRRCAP        0x000000fe
#define MSR_SYSENTER_CS    0x00000174
#define MSR_SYSENTER_ESP   0x00000175
#define MSR_SYSENTER_EIP   0x00000176
#define MSR_MGC_CAP        0x00000179
#define MSR_MGC_STATUS     0x0000017a
#define MSR_MGC_CTL        0x0000017b
#define MSR_EFER           0xc0000080
#define MSR_STAR           0xc0000081
#define MSR_LSTAR          0xc0000082
#define MSR_CSTAR          0xc0000083
#define MSR_SFMASK         0xc0000084
#define MSR_FS_BASE        0xc0000100
#define MSR_GS_BASE        0xc0000101
#define MSR_KERNEL_GS_BASE 0xc0000102
#define MSR_TS_AUX         0xc0000103
#define MSR_SYS_CFG        0xc0010010


#ifdef CONFIG_CPU_AMD64

static inline uint64_t __attribute__((always_inline))
get_cr2 (void)
{
  uint64_t __cr2;
  asm volatile ("movq %%cr2, %%rax"
                : "=a" (__cr2)); 
  return __cr2;
}

#elif defined CONFIG_CPU_IA32

static inline uint32_t __attribute__((always_inline))
get_cr2 (void)
{
  uint32_t __cr2;
  asm volatile ("movl %%cr2, %%eax"
                : "=a" (__cr2)); 
  return __cr2;
}
#else
#error Bad architecture
#endif



/** Read a MSR. */
static inline uint64_t __attribute__((always_inline))
rdmsr (uint32_t reg)
{
  uint64_t __val;
  __asm__ __volatile__ ("rdmsr"
			: "=A" (__val)
			: "c" (reg));
  return __val;
}

#define wrmsr(msr,val1,val2) \
     __asm__ __volatile__("wrmsr" \
                          : /* no outputs */ \
                          : "c" (msr), "a" (val1), "d" (val2))

#define wrmsrl(msr,val) wrmsr(msr,(uint32_t)((uint64_t)(val)),((uint64_t)(val))>>32)



#endif
