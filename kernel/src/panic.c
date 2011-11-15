#include <printf.h>


void
panic (const char *fstr, ...)
{
  va_list ap;
  
  printf ("Kernel panic: ");
  va_start (ap, fstr);
  vprintf (fstr, ap);
  va_end (ap);
  printf ("\n");
  
  for (;;);
}
