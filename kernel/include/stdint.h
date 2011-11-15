/**
 * \file stdint.h ISO99 integer types.
 */

#ifndef __STDINT_H
#define __STDINT_H

/* Unigned types */

/** Unsigned 8-bits type. */
typedef unsigned char uint8_t;

/** Unsigned 16-bits type. */
typedef unsigned short int uint16_t;

/** Unsigned 32-bits type. */
typedef unsigned int uint32_t;


/* Signed types */

/** Signed 8-bits type. */
typedef signed char int8_t;

/** Signed 16-bits type. */
typedef signed short int int16_t;

/** Signed 32-bits type. */
typedef signed int int32_t;

#ifdef CONFIG_CPU_IA32

typedef unsigned int uintptr_t;
typedef unsigned int uintmax_t;

/** Unsigned 64-bits type. */
typedef unsigned long long uint64_t;

/** Signed 64-bits type. */
typedef signed long long int64_t;

typedef unsigned int size_t;

#elif defined CONFIG_CPU_AMD64

typedef unsigned long uintptr_t;
typedef unsigned long uintmax_t;

/** Unsigned 64-bits type. */
typedef unsigned long uint64_t;

/** Signed 64-bits type. */
typedef signed long int64_t;

typedef unsigned long size_t;

#endif

#endif
