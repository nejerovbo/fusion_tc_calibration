/*****************************************************************************
 * (c) Copyright 2019 Digital Dynamics Inc. Scotts Valley CA USA.
 * Unpublished copyright. All rights reserved. Contains proprietary and
 * confidential trade secrets belonging to DDI. Disclosure or release without
 * prior written authorization of DDI is prohibited.
 *****************************************************************************/
/*  ddi_hexdump.c
 *  Created by Johana Lehrer on 2019-07-10
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>

int hexdump(uint32_t addr, const void *data, size_t len, char *buf, int width);

void simple_hexdump(const void *data, size_t len)
{
  hexdump(0, data, len, 0, 1);
}

//int hexdump(uint32_t addr, const void *data, size_t len, char *buf)

#define PRINTF(buf,...) do{ if (buf){buf += sprintf(buf, __VA_ARGS__);} else {len += printf(__VA_ARGS__); fflush(0);} }while(0)

int hexdump(uint32_t addr, const void *data, size_t len, char *buf, int width)
{
  char *start = buf;
  uint8_t *p = (uint8_t *)data;
  uint16_t *s = (uint16_t *)data;
  uint32_t *q = (uint32_t *)data;
  char str[32];
  uint32_t i, n, a, addr_width, col, cols;
  uint32_t end = addr + len;
  col = 0;
  n = 0;

  memset(str, 0, sizeof(str));

  if (end < 0x100) {
    addr_width = 1;
  } else if (end < 0x10000) {
    addr_width = 2;
  } else {
    addr_width = 4;
  }

  cols = 16 / width;

  i = (addr & 15);
  a = i / width;
  if (a > 0)
  {
    if (addr_width == 4) {
      PRINTF(buf, "%08x:  ", addr & ~15);
    } else if (addr_width == 2) {
      PRINTF(buf, "%04x:  ", addr & ~15);
    } else {
      PRINTF(buf, "%02x:  ", addr & ~15);
    }

    for (i = 0; i < a; i++)
    {
      col++;
      if (width == 4) {
        PRINTF(buf, "         ");
      } else if (width == 2) {
        PRINTF(buf, "     ");
      } else {
        PRINTF(buf, "   ");
      }

      // pad ascii str[]
      for (int x = 0; x < width; x++) {
        str[n++] = ' ';
      }
    }
  }

  while (addr < end)
  {
    if (col == 0) {
      if (addr_width == 4) {
        PRINTF(buf, "%08x:  ", addr);
      } else if (addr_width == 2) {
        PRINTF(buf, "%04x:  ", addr);
      } else {
        PRINTF(buf, "%02x:  ", addr);
      }
    }

    if (width == 4) {
      PRINTF(buf, "%08x ", *q++);
    } else if (width == 2) {
      PRINTF(buf, "%04x ", *s++);
    } else {
      PRINTF(buf, "%02x ", *p);
    }

    for (i = 0; i < width; i++) {
      str[n++] = isprint(*p) ? *p : '.';
      p++; // always increment char pointer for ascii str[]
    }

    if (++col == cols) {
      PRINTF(buf, "  %s\n", str);
      memset(str, 0, sizeof(str));
      n = 0;
      col = 0;
    }
    addr += width;
  }

  if ((col > 0) && (col < cols))
  {
    while (col < cols)
    {
      if (width == 4) {
        PRINTF(buf, "         ");
      } else if (width == 2) {
        PRINTF(buf, "     ");
      } else {
        PRINTF(buf, "   ");
      }

      // pad ascii str[]
      for (int x = 0; x < width; x++) {
        str[n++] = ' ';
      }

      col++;
    }
    PRINTF(buf, "  %s\n", str);
  }
  PRINTF(buf, "\n");

  if (buf) {
    len = (int)(buf - start);
  }
  return len;
}

