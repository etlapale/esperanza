/**
 * \file stdint.h ISO99 integer types.
 */

#ifndef __STDINT_H
#define __STDINT_H


/** Unsigned 8-bits type. */
typedef unsigned char uint8_t;

/** Unsigned 16-bits type. */
typedef unsigned short int uint16_t;

/** Unsigned 32-bits type. */
typedef unsigned int uint32_t;

/** Unsigned 64-bits type. */
typedef unsigned long long uint64_t;

/** Biggest signed cpu word length. */
typedef signed int intmax_t;

typedef signed int ssize_t;


/** Signed 8-bits type. */
typedef signed char int8_t;

/** Signed 16-bits type. */
typedef signed short int int16_t;

/** Signed 32-bits type. */
typedef signed int int32_t;

/** Signed 64-bits type. */
typedef signed long long int64_t;

/** Biggest unsigned cpu word length. */
typedef unsigned int uintmax_t;

typedef unsigned int size_t;

/** Address pointer. */
typedef unsigned int uintptr_t;

#endif
