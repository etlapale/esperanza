#ifndef __GDT_H
#define __GDT_H

#include <stdint.h>



#define CODE_SEGMENT  11
#define DATA_SEGMENT  3
#define STACK_SEGMENT 7

#define AVAILABLE_TSS 9
#define BUSY_TSS      11

#define SYSTEM_DESCRIPTOR  0
#define SEGMENT_DESCRIPTOR 1

#define KERNEL_LEVEL 0
#define HIGH_LEVEL 1
#define MEDIUM_LEVEL 2
#define USER_LEVEL 3

#define ABSENT 0
#define PRESENT 1

#define INSTRUCTIONS_16 0
#define INSTRUCTIONS_32 1

#define UNIT_BYTE 0
#define UNIT_4KB 1

#define SEGMENT_DESCRIPTOR_SIZE 0x08

#define KERNEL_CODE_SEGMENT 0x08
#define KERNEL_DATA_SEGMENT 0x10
#define USER_CODE_SEGMENT 0x18
#define USER_DATA_SEGMENT 0x20
#define USER_CODE_SEGMENT2 0x28
#define TSS_SEGMENT 0x30

/** The minimal size a TSS can have. */
#define MINIMAL_TSS_SIZE 0x67


/** Segment descriptor. */
struct Segment_Descriptor {
  /** First part of the segment size. */
  uint16_t size_first ;
  /** First part of the segment address. */
  uint16_t address_first ;
  /** Second part of the segment address. */
  uint8_t address_second ;
  /** Segment type. */
  int type : 4;
  /** Indicate if the descriptor is a normal segment descriptor. */
  int segment : 1;
  /** Segment privilegied level. */
  int level : 2;
  /** Indicate if the segment is present in memory. */
  int present : 1 ;
  /** Second part of the segment size. */
  int size_second : 4 ;
  
  /** Available for the OS. We don't use it. */
  int available : 1;
  
  /**
   * On AMD64, indicates if we are in Long Mode,
   * on IA32 set this bit to zero.
   */
  int long_mode : 1;
  
  /**
   * Default operand size.
   * On 32-bits segments, set this bit to have a default operand
   * size of 32 bits, otherwise a default operand size of 16 bits.
   * On 64-bits segments, the flag must be cleared and the default
   * operand size is fixed to 32 bits.
   */
  int instructions_length : 1;
  
  /** Segment size unit (1 for 4Ko). */
  int size_unit : 1;
  
  /** Third part of the segment address. */
  uint8_t address_third ;
} __attribute__ ((packed));

/** GDTR representation. */
struct GDTR {
  /** Size. */
  uint16_t size ;
  /** Address. */
  uint32_t address ;
} __attribute__ ((packed)) ;

#endif
