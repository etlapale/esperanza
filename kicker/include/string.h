#ifndef __STRING_H
#define __STRING_H

#include <stddef.h>
#include <stdint.h>


/**
 * Calculate the length of a string.
 * The length does not include the terminating
 * '\0' character.
 */
size_t
strlen (const char *s);

/**
 * Compare two strings.
 * The  strcmp()  function compares the two strings s1 and s2.
 * It returns an integer less than, equal to, or greater than zero
 * if s1 is found, respectively, to be less than, to match, or be
 * greater than s2.
 */
int
strcmp (const char *s1, const char *s2);

int
memcmp (const void *p1, const void *p2, size_t n);

/**
 * Copy memory area.
 * The  memcpy()  function  copies  n bytes from memory area src
 * to memory area dest.
 */
void *
memcpy (void *dest, const void *src, size_t n);

void *
memset (void *s, int c, size_t n);

#endif
