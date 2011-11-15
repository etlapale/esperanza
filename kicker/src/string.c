#include <stdint.h>
#include <string.h>


size_t
strlen (const char* s)
{
  int i;
  for (i = 0; s[i] != '\0'; i++);
  return i;
}

void *
memcpy (void *dest, const void *src, size_t n)
{
  int i;

  for (i = 0; (unsigned) i < n; i++)
    ((char *) dest)[i] = ((const char *) src)[i];

  return dest;
}

int
strcmp (const char *s1, const char *s2)
{
  int i;

  for (i = 0;; i++)
    {
      if (s1[i] == s2[i])
        {
          if (s1[i] == '\0')
            return 0;
        }
      else if (s1[i] < s2[i])
        {
          return -1;
        }
      else
        {
          return 1;
        }
    }
}

int
memcmp (const void *s1, const void *s2, size_t n)
{
  uint32_t pos;
  const uint8_t *p1 = s1;
  const uint8_t *p2 = s2;
  
  for (pos = 0; pos < n; pos++)
    {
      if (p1[pos] < p2[pos])
	return -1;
      else if (p1[pos] > p2[pos])
	return 1;
    }

  return 0;
}

void *
memset (void *s, int c, size_t n)
{
  size_t i;
  for (i = 0; i < n; i++)
    ((char *) s)[i] = (unsigned char) c;
  return s;
}
