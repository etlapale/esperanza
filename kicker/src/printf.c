#include <io.h>
#include <printf.h>
#include <stdarg.h>
#include <stdint.h>


#define COM1_PORT 0x3f8
#define COM1_SPEED 115200


#ifdef CONFIG_SCREEN_DEBUG

#define CRT_REG_INDEX 0x3d4
#define CRT_REG_DATA 0x3d5

/* Screen size */
#define COLUMNS 80
#define LINES 17

/* Default character attribute (gray) */
#define ATTRIBUTE 7

/* Tabulation length in spaces */
#define TAB_LENGTH 8


/* Current position */
int console_xpos;
int console_ypos;

static volatile unsigned char *video;
#endif



static void itoa (char *buf, int base, int d);
int putchar (int c);
void vprintf (const char *format, va_list ap);


/**
 * Initialize the printf output
 */
void
init_printf (void)
{
#ifdef CONFIG_COM1_DEBUG
  uint16_t divisor;
#endif
  
#ifdef CONFIG_SCREEN_DEBUG
  int i;

  /* Hide the cursor */
  outb (CRT_REG_INDEX, 0x0a);
  outb (CRT_REG_DATA, 1 << 5);
  
  video = (unsigned char *) VIDEO_ADDRESS;

  for (i = 0; i < COLUMNS * LINES * 2; i++)
    *(video + i) = 0;

  console_xpos = 0;
  console_ypos = 0;
#endif

#ifdef CONFIG_COM1_DEBUG
  divisor = 115200 / COM1_SPEED;
  outb (COM1_PORT + 3, 0x80);
  outb (COM1_PORT, divisor & 0xf);
  outb (COM1_PORT + 1, divisor >> 8);
  outb (COM1_PORT + 3, 3);
#endif
}

/* Convert the integer D to a string and save the string in BUF. If
   BASE is equal to 'd', interpret that D is decimal, and if BASE is
   equal to 'x', interpret that D is hexadecimal.  */
static void
itoa (char *buf, int base, int d)
{
  char *p = buf;
  char *p1, *p2;
  unsigned long ud = d;
  int divisor = 10;

  /* If %d is specified and D is minus, put `-' in the head.  */
  if (base == 'd' && d < 0)
    {
      *p++ = '-';
      buf++;
      ud = -d;
    }
  else if (base == 'x')
    divisor = 16;

  /* Divide UD by DIVISOR until UD == 0.  */
  do
    {
      int remainder = ud % divisor;

      *p++ = (remainder < 10) ? remainder + '0' : remainder + 'a' - 10;
    }
  while (ud /= divisor);

  /* Terminate BUF.  */
  *p = 0;

  /* Reverse BUF.  */
  p1 = buf;
  p2 = p - 1;
  while (p1 < p2)
    {
      char tmp = *p1;
      *p1 = *p2;
      *p2 = tmp;
      p1++;
      p2--;
    }
}

#ifdef CONFIG_SCREEN_DEBUG
/**
 * Scroll the screen one line down.
 */
void
scroll_down ()
{
  uint32_t x, y;
  
  for (x = 0; x < COLUMNS; x++)
    {
      for (y = 1; y < LINES; y++)
        {
          *(video + (x + (y - 1) * COLUMNS) * 2) = *(video + (x + y * COLUMNS) * 2);
          *(video + (x + (y - 1) * COLUMNS) * 2 + 1) = *(video + (x + y * COLUMNS) * 2 + 1);
        }
      
      *(video + (x  + (LINES - 1) * COLUMNS) * 2) = 0;
      *(video + (x  + (LINES - 1) * COLUMNS) * 2 + 1) = ATTRIBUTE;
    }
}
#endif

/* Display a char on the screen. */
void
print_char (int c)
{
#ifdef CONFIG_BOCHS_DEBUG
  __asm__ volatile ("outb %%al, $0xe9" :: "a" (c));
#endif

#ifdef CONFIG_COM1_DEBUG
  if (c == '\n')
    {
      while (!(inb (COM1_PORT + 5) & 0x20));
      outb (COM1_PORT, '\r');
    }

  while (!(inb (COM1_PORT + 5) & 0x20));
  outb (COM1_PORT, c);
#endif

#ifdef CONFIG_SCREEN_DEBUG
  if (c == '\n')
    {
    newline:
      console_xpos = 0;
      if (console_ypos < LINES - 1)
        console_ypos++;
      else
        scroll_down ();
      return;
    }

  /* Tabulation */
  if (c == '\t')
    {
      unsigned int i ;
      for (i = 0 ; i < TAB_LENGTH; i++)
        print_char (' ');
      return;
    }

  *(video + (console_xpos + console_ypos * COLUMNS) * 2) = c & 0xff;
  *(video + (console_xpos + console_ypos * COLUMNS) * 2 + 1) = ATTRIBUTE;

  console_xpos++;
  if (console_xpos >= COLUMNS)
    goto newline;
#endif
}

void
panic (const char *fstr, ...)
{
  va_list ap;
  
  printf ("Kicker panic: ");
  va_start (ap, fstr);
  vprintf (fstr, ap);
  va_end (ap);
  printf ("\n");
  
  for (;;);
}

/* Format a string and print it on the screen, just like the libc
   function printf.  */
void
printf (const char *format, ...)
{
  va_list ap;
  va_start (ap, format);
  vprintf (format, ap);
  va_end (ap);
}

void
vprintf (const char *format, va_list ap)
{
  /* Current char */
  int c;
  char buf[20];

  /* For each char on the format string */
  while ((c = *format++) != 0)
    {
      /* Simple char, just display it */
      if (c != '%')
        {
          print_char (c);
        }
      else
        {
          char *p;

          c = *format++;
          switch (c)
            {
            case 'd':
            case 'u':
            case 'x':
              itoa (buf, c, va_arg (ap, uint32_t));
              p = buf;
              goto string;
              break;

            case 's':
              p = va_arg (ap, char *);
              if (! p)
                p = "(null)";

            string:
              while (*p)
                print_char (*p++);
              break;

            default:
              print_char (va_arg (ap, uint32_t));
              break;
            }
        }
    }
}

void
vsprintf (char *outbuff, const char *format, va_list ap)
{
  int outpos = 0;
  /* Current char */
  int c;
  char buf[20];

  /* For each char on the format string */
  while ((c = *format++) != 0)
    {
      /* Simple char, just display it */
      if (c != '%')
        {
          outbuff[outpos++] = c;
        }
      else
        {
          char *p;

          c = *format++;
          switch (c)
            {
            case 'd':
            case 'u':
            case 'x':
              itoa (buf, c, va_arg (ap, uint32_t));
              p = buf;
              goto string;
              break;

            case 's':
              p = va_arg (ap, char *);
              if (! p)
                p = "(null)";

            string:
              while (*p)
		outbuff[outpos++] = *p++;
              break;

            default:
              outbuff[outpos++] = va_arg (ap, uint32_t);
              break;
            }
        }
    }

  outbuff[outpos] = '\0';
}

void
sprintf (char *buff, const char *format, ...)
{
  va_list ap;
  va_start (ap, format);
  vsprintf (buff, format, ap);
  va_end (ap);
}
