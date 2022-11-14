/*****************************************************************************
 * (c) Copyright 2020 Digital Dynamics Inc. Scotts Valley CA USA.
 * Unpublished copyright. All rights reserved. Contains proprietary and
 * confidential trade secrets belonging to DDI. Disclosure or release without
 * prior written authorization of DDI is prohibited.
 *****************************************************************************/
/*  ddi_str_utils.c
 *  Created by Johana Lehrer on 2020-01-18
 */

#include "ddi_str_utils.h"
#include <string.h>

char *scan_to(const char *p, char c)
{
  while (*p && (*p != c))
    p++;
  return (char *)p;
}

char *skip_spaces(const char *p)
{
  char c = *p;
  while ((c == ' ') || (c == '\t') || (c == '\r') || (c == '\n'))
  {
    c = *(++p);
  }
  return (char *)p;
}

char *next_line(const char *p)
{
  while (*p && (*p != '\r') && (*p != '\n'))
    p++;
  while (*p && ((*p == '\r') || (*p == '\n')))
    p++;
  return (char *)p;
}

void append_char(char *str, char c)
{
  int len = strlen(str);
  if (len && str[len - 1] != c) {
    str[len++] = c;
    str[len] = '\0';
  }
}

char *scan_to_eol(const char *p)
{
  while (*p && (*p != '\n') && (*p != '\r'))
    p++;
  return (char *)p;
}

uint8_t atox(char a)
{
  if ((a >= '0') && (a <= '9'))
    return (a - '0');
  if ((a >= 'A') && (a <= 'F'))
    return 10 + (a - 'A');
  return 10 + (a - 'a');
}

/** basename
 @brief returns a pointer to the filename string without the leading path
 */
char *basename(const char *path)
{
  char *p = (char *)path + strlen(path);
  while (p > path)
  {
    if (*p == '/')
      return p + 1;
    p--;
  }
  return p;
}

/** suffix
 @brief returns a pointer to the dot suffix of a filename
 */
char *suffix(const char *filename)
{
  char *p = (char *)filename + strlen(filename);
  while (p > filename)
  {
    if (*p == '.')
      break;
    p--;
  }
  return p;
}

/** vmemcpy
 @brief implements a volatile memcpy
 */
void vmemcpy (volatile void *dest, volatile void *src, int len )
{
  volatile uint8_t *s, *d;
  s = src;
  d = dest;
  while ( 0 < len--)
  {
    *d++=*s++;
  }
}

/** vstrncpy
 @brief implements a volatile strncpy
 */
volatile char* vstrncpy(volatile char* dest, volatile const char* src, size_t num)
{
  volatile char *d;
  volatile char *s;

  // Check for NULL
  if (dest == NULL)
    return NULL;

  d = dest;
  s = (volatile char *)src;

  // copy characters while they are not NULL and there are more characters to copy
  while (*s && num--)
  {
    *d++ = *s++;
  }

  // null terminate the copied string
  *d = '\0';

  // return destination pointer
  return d;
}

