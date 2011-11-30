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

    {
      msg_tag_t mr0;
      thread_id_t from;
      mr0 = ipc (0xcafe, 0xbabe, 0x42, &from);
      printf("IPC returned: 0x%lx\n", mr0);
      printf("%d\n", 0x123);
    }
  
  for (;;)
    keyboard_listener (receive (KEYBOARD_PORT));
  
  return 0;
}

// vim: sw=2:
