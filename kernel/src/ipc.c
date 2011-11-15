#include <ipc.h>
#include <printf.h>
#include <stddef.h>
#include <thread.h>

thread_t *listeners[MAX_PORT_COUNT] = {NULL,};

void
send (unsigned int dest, unsigned long msg)
{
  if (listeners[dest])
    {
      listeners[dest]->buffer = msg;
      listeners[dest]->state = THREAD_RUNNING;
    }
  /*
  else
    {
      printf ("No listener on port 0x%x [DATA: 0x%lx]\n", dest, msg);
    }
  */
}
