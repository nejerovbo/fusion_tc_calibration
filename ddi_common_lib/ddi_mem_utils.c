/*****************************************************************************
 * (c) Copyright 2020 Digital Dynamics Inc. Scotts Valley CA USA.
 * Unpublished copyright. All rights reserved. Contains proprietary and
 * confidential trade secrets belonging to DDI. Disclosure or release without
 * prior written authorization of DDI is prohibited.
 *****************************************************************************/
/*  ddi_mem_utils.c
 *  Created by Johana Lehrer on 2020-01-18
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include "ddi_debug.h"
#include "ddi_defines.h"

void *zalloc(size_t size)
{
  void *p = malloc(size);
  if (!p)
  {
    ELOG("zalloc failed\n");
    return 0;
  }
  memset(p, 0, size);
  return p;
}

void blockzero(void *mem, int len, int width)
{
  if (width == 1) {
    volatile uint8_t *p = (volatile uint8_t *)mem;
    while (len-- > 0) {
      *p++ = 0;
    }
  } else if (width == 2) {
    volatile uint16_t *s = (volatile uint16_t *)mem;
    while (len > 0) {
      *s++ = 0;
      len -= 2;
    }
  } else {
    volatile uint32_t *q = (volatile uint32_t *)mem;
    while (len > 0) {
      *q++ = 0;
      len -= 4;
    }
  }
}

void blockcopy(void *dst, void *src, int len, int width)
{
  if (width == 1) {
    volatile uint8_t *s = (volatile uint8_t *)src;
    volatile uint8_t *d = (volatile uint8_t *)dst;
    while (len-- > 0) {
      *d++ = *s++;
    }
  } else if (width == 2) {
    volatile uint16_t *s = (volatile uint16_t *)src;
    volatile uint16_t *d = (volatile uint16_t *)dst;
    while (len > 0) {
      *d++ = *s++;
      len -= 2;
    }
  } else {
    volatile uint32_t *s = (volatile uint32_t *)src;
    volatile uint32_t *d = (volatile uint32_t *)dst;
    while (len > 0) {
      *d++ = *s++;
      len -= 4;
    }
  }
}

void block_endian_swap(void *src, uint32_t offset, int len, int width)
{
  if (width == 2)
  {
    volatile uint16_t *s = (volatile uint16_t *)((uint8_t *)src + offset);
    while (len > 0)
    {
      uint16_t tmp = *s;
      tmp = SWAP16(tmp);
      *s++ = tmp;
      len -= width;
    }
  }
  else if ( width == 4 )
  {
    volatile uint32_t *s = (volatile uint32_t *)((uint8_t *)src + offset);
    while (len > 0)
    {
      uint32_t tmp = *s;
      tmp = SWAP32(tmp);
      *s++ = tmp;
      len -= 4;
    }
  }
}

size_t readfile(const char *filename, void **pdata)
{
  size_t actual;
  size_t len;
  FILE *fp = fopen(filename, "rb");
  if (!fp)
  {
    ELOG("open \'%s\' failed: %d\n", filename, errno);
    return 0;
  }

  fseek(fp, 0, SEEK_END);
  len = ftell(fp);
  fseek(fp, 0, SEEK_SET);
  *pdata = zalloc(len);
  actual = fread(*pdata, 1, len, fp);
  fclose(fp);

  if (len != actual)
  {
    ELOG("len: %d written: %d\n", (int)len, (int)actual);
  }

  return actual;
}

size_t readfile_offset(const char *filename, int offset, void **pdata)
{
  size_t actual;
  size_t len;
  FILE *fp = fopen(filename, "rb");
  if (!fp)
  {
    ELOG("open \'%s\' failed: %d\n", filename, errno);
    return 0;
  }

  fseek(fp, 0, SEEK_END);
  len = ftell(fp);
  fseek(fp, 0, SEEK_SET);      // Rewind
  fseek(fp, offset, SEEK_SET); // seek to offset
  len = len - offset;          // Adjust length for offset
  *pdata = zalloc(len);
  actual = fread(*pdata, 1, len, fp);
  fclose(fp);

  if (len != actual)
  {
    ELOG("len: %d written: %d\n", (int)len, (int)actual);
  }

  return actual;
}

size_t writefile(const char *filename, const void *data, size_t len)
{
  size_t actual;
  FILE *fp = fopen(filename, "wb");
  if (!fp)
  {
    ELOG("open \'%s\' failed: %d\n", filename, errno);
    return 0;
  }
  actual = fwrite(data, 1, len, fp);
  fclose(fp);

  if (len != actual)
  {
    ELOG("len: %d written: %d\n", (int)len, (int)actual);
  }

  return actual;
}

int write_mem_to_file(const char *filename, uint8_t *mem, uint8_t *data, uint32_t len, uint32_t width)
{
  FILE *fp = fopen(filename, "wb");
  if (fp == 0)
  {
    ELOG("Could not open '%s' for write\n", filename);
    return 1;
  }

  blockcopy(data, mem, len, width);

  int n = fwrite(data, 1, len, fp);
  fclose(fp);
  if (n != len)
  {
    ELOG("error writing %d bytes to '%s' file\n", len, filename);
    return 1;
  }
  return 0;
}

int read_file_to_mem(const char *filename, uint8_t *mem, uint8_t *data, uint32_t len, uint32_t width)
{
  FILE *fp = fopen(filename, "rb");
  if (fp == 0)
  {
    ELOG("Could not open '%s' for read\n", filename);
    return 1;
  }

  int n = fread(data, 1, len, fp);
  fclose(fp);

  if (n != len)
  {
    ELOG("error reading %d bytes from '%s' file\n", len, filename);
    return 1;
  }

  blockcopy(mem, data, len, width);
  return 0;
}

