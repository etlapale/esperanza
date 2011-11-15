#ifndef __PRINTF_H
#define __PRINTF_H

#include <stdint.h>


/* Video address */
#define VIDEO_ADDRESS 0xb8000


/**
 * Clears the screen.
 */
void init_printf ();

/*
 * printf for the kernel. Display the given paramaters according to the format
 * string.
 */
void __attribute__ ((format (printf, 1, 2)))
printf (const char *format, ...);

void __attribute__ ((format (printf, 2, 3)))
sprintf (char *buff, const char *format, ...);

void panic (const char *format, ...);

extern uint32_t console_debug;
extern int console_xpos;
extern int console_ypos;

#endif
