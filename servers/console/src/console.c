#include <ipc.h>
#include <stddef.h>

extern int console_xpos;
extern int console_ypos;
extern volatile unsigned char *console_video;

void
init_printf (void);

void __attribute__ ((format (printf, 1, 2)))
printf (const char *format, ...);

typedef unsigned char scancode_t;

void
keyboard_listener (scancode_t code);

int
main (void)
{
  init_printf ();
  printf ("Esperanza console starting!\n");
  
  for (;;)
    keyboard_listener (receive (KEYBOARD_PORT));
  
  return 0;
}
