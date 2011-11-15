#ifndef __PRINTF_H
#define __PRINTF_H

#include <stdarg.h>
#include <stdint.h>

/*
 * printf for the kernel. Display the given paramaters according to the format
 * string.
 */
void __attribute__ ((format (printf, 1, 2)))
printf (const char *format, ...);

void
vprintf (const char *format, va_list ap);

void panic (const char *format, ...);

extern uint32_t console_debug;

#endif
