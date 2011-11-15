#include <ipc.h>
#include <printf.h>

void
idle ()
{
  for (;;)
    {
      //send (KEYBOARD_PORT, 0x10);
      __asm__ __volatile__ ("hlt");
    }
}
