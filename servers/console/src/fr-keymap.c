#include <stddef.h>
#include <stdarg.h>

void __attribute__ ((format (printf, 1, 2)))
printf (const char *format, ...);

void
vprintf (const char *format, va_list ap);


typedef unsigned char scancode_t;

#define KBD_BREAKCODE_LIMIT         0x80
#define KBD_EXTENDED_SCANCODE       0xe0

#define KBD_IS_MAKECODE(c)          ((c) < KBD_BREAKCODE_LIMIT)
#define KBD_IS_BREAKCODE(c)         ((c) >= KBD_BREAKCODE_LIMIT)
#define KBD_BREAKCODE_2_MAKECODE(c) ((c) ^ KBD_BREAKCODE_LIMIT)

#define KBD_LEFTSHIFT_SCANCODE      0x2a
#define KBD_RIGHTSHIFT_SCANCODE     0x36


int kbd_ext = FALSE;

char *kbd_normal_table[128] = {
  0,
  /* ESC */ 0,
  "1",
  "2",
  "3",
  "4",
  "5",
  "6",
  "7",
  "8",
  "9",
  "0",
  ")",
  "=",
  /* BCKSP */ 0,
  "\t",
  "a",
  "z",
  "e",
  "r",
  "t",
  "y",
  "u",
  "i",
  "o",
  "p",
  /* ^ */ 0,
  "$",
  "\n",
  /* CTRL-L */ 0,
  "q",
  "s",
  "d",
  "f",
  "g",
  "h",
  "j",
  "k",
  "l",
  "m",
  "ù",
  0,
  0,
  "*",
  "w",
  "x",
  "c",
  "v",
  "b",
  "n",
  ",",
  ";",
  ":",
  "!",
  /* SHF-R */ 0,
  0,
  /* ALT-L */ 0,
  " ",
  /* CAPS-LOCK */ 0,
  0,
};

char *kbd_shift_table[128] =  {
  0,
  /* ESC */ 0,
  "1",
  "2",
  "3",
  "4",
  "5",
  "6",
  "7",
  "8",
  "9",
  "0",
  ")",
  "=",
  /* BCKSP */ 0,
  "\t",
  "A",
  "Z",
  "E",
  "R",
  "T",
  "Y",
  "U",
  "I",
  "O",
  "P",
  /* ^ */ 0,
  "£",
  "\n",
  /* CTRL-L */ 0,
  "Q",
  "S",
  "D",
  "F",
  "G",
  "H",
  "J",
  "K",
  "L",
  "M",
  "%",
  0,
  0,
  "µ",
  "W",
  "X",
  "C",
  "V",
  "B",
  "N",
  "?",
  ".",
  "/",
  "§",
  /* SHF-R */ 0,
  0,
  /* ALT-L */ 0,
  " ",
  /* CAPS-LOCK */ 0,
  0,
};

char **kbd_current_translation_table = kbd_normal_table;

extern void print_char (int);

void
keyboard_listener (scancode_t code)
{
  /* Extended */
  if (code == KBD_EXTENDED_SCANCODE)
    {
      kbd_ext = TRUE;
    }
  /* Last was extended */
  else if (kbd_ext)
    {
      printf ("\n[EXT] 0x%x\n", (unsigned int) code);
      kbd_ext = FALSE;
    }
  /* Key pressed */
  else if (KBD_IS_MAKECODE (code))
    {
      /* SHIFT translation table */
      if (code == KBD_LEFTSHIFT_SCANCODE || code == KBD_RIGHTSHIFT_SCANCODE)
	kbd_current_translation_table = kbd_shift_table;

      /* ALT-GR translation table */
      

      /* Normal key press */
      else if (kbd_current_translation_table[code])
	printf (kbd_current_translation_table[code]);
      
      else
	printf ("\n[???] 0x%x\n", (unsigned int) code);
    }
  /* Key released */
  else
    {
      scancode_t makecode = KBD_BREAKCODE_2_MAKECODE (code);
      
      /* Remove the SHIFT translation table */
      if (makecode == KBD_LEFTSHIFT_SCANCODE
	  || makecode == KBD_RIGHTSHIFT_SCANCODE)
	kbd_current_translation_table = kbd_normal_table;
    }
}
