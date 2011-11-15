#ifndef __PRINTF_H
#define __PRINTF_H

#undef CONFIG_SCREEN_DEBUG

#include <stdint.h>

/*
 * printf for the kernel. Display the given paramaters according to the format
 * string.
 */
void __attribute__ ((format (printf, 1, 2)))
printf (const char *format, ...);

#endif
