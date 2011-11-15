/**
 * \file stddef.h
 * Defines generic definitions.
 */

#ifndef __STDDEF_H
#define __STDDEF_H

#ifndef NULL
#ifdef __cplusplus
#define NULL 0
#else
#define NULL ((void *) 0)
#endif /* __cplusplus */
#endif /* ! NULL */

/** Returns the offset of a member in a structure. */
#define offsetof(type,member) ((unsigned long) &(((type*)0)->member))


#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#endif /* stddef.h */
