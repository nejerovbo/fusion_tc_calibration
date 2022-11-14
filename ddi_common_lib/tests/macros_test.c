/******************************************************************************
 * (c) Copyright 2020-2021 Digital Dynamics Inc. Scotts Valley CA USA.
 * Unpublished copyright. All rights reserved. Contains proprietary and
 * confidential trade secrets belonging to DDI. Disclosure or release without
 * prior written authorization of DDI is prohibited.
 *****************************************************************************/
/*
 * macros_test.c
 * sqa-common-tests
 *
 *  Created on: 2020-02-08 by Johana Lehrer
 */

#include <stdio.h>
#include "ddi_macros.h"
#include "ddi_defines.h"


int array_test(void)
{
  #define BITSIZE       4
  #define FIELDCOUNT    64
  uint32_t array[ARRAY32_SIZE(BITSIZE, FIELDCOUNT)] = {0};
  uint32_t value = 42;
  uint32_t index = 54;

  ARRAY32_SET_ENTRY(array, BITSIZE, index, value);
  uint32_t the_value = ARRAY32_GET_ENTRY(array, BITSIZE, index);
  printf("array set index: %d bitsize: %d field count: %d value: %x result: %X\n", index, BITSIZE, FIELDCOUNT, value, the_value);
  return OK;
}

int bit32_test(void)
{
  int status = OK;
  uint32_t reg;

  #define MY_REGISTER_BITFIELD_A             31:20
  #define MY_REGISTER_BITFIELD_B             19:16
  #define MY_REGISTER_BITFIELD_B_VAL1        0x3
  #define MY_REGISTER_BITFIELD_B_VAL2        0x9
  #define MY_REGISTER_BITFIELD_B_VAL3        0xd
  #define MY_REGISTER_BITFIELD_C             15:10
  #define MY_REGISTER_BITFIELD_D             9:8
  #define MY_REGISTER_BITFIELD_E             7:1
  #define MY_REGISTER_BITFIELD_F             0:0

  #define TEST(desc,num,ret,exp) do {  printf("%s test[%d] (0x%x == 0x%x) %s\n", desc, num, ret, exp, ((ret) == (exp)) ? "PASS" : "FAIL"); if ((ret) != (exp)) status = NOT_OK; } while(0)

  reg = BIT32_SHIFT(MY_REGISTER_BITFIELD_A);                            TEST("BIT32_SHIFT", 1, reg, 20);
  reg = BIT32_SIZE(MY_REGISTER_BITFIELD_B);                             TEST("BIT32_SIZE", 1, reg, 4);
  reg = BIT32_MASK(MY_REGISTER_BITFIELD_A);                             TEST("BIT32_MASK", 1, reg, 0x00000FFF);
  reg = BIT32_MASK(MY_REGISTER_BITFIELD_B);                             TEST("BIT32_MASK", 2, reg, 0x0000000F);
  reg = BIT32_SHIFT_MASK(MY_REGISTER_BITFIELD_C);                       TEST("BIT32_SHIFT_MASK", 1, reg, 0x0000FC00);
  reg = BIT32_SHIFT_MASK(MY_REGISTER_BITFIELD_B);                       TEST("BIT32_SHIFT_MASK", 2, reg, 0x000F0000);
  reg = BIT32_VALUE(MY_REGISTER_BITFIELD_A, 0x12);                      TEST("BIT32_VALUE", 1, reg, 0x01200000);
  reg = BIT32_CONST(MY_REGISTER_BITFIELD_B,_VAL2);                      TEST("BIT32_CONST", 1, reg, 0x00090000);
  reg = BIT32_CLR_FIELD(MY_REGISTER_BITFIELD_B, 0xFFFFFFFF);            TEST("BIT32_CLR_FIELD", 1, reg, 0xFFF0FFFF);
  reg = BIT32_SET_VALUE(MY_REGISTER_BITFIELD_A, 0x123, 0xFFFFFFFF);     TEST("BIT32_SET_VALUE", 1, reg, 0x123FFFFF);
  reg = BIT32_SET_CONST(MY_REGISTER_BITFIELD_B,_VAL2, 0xFFFFFFFF);      TEST("BIT32_SET_CONST", 1, reg, 0xFFF9FFFF);
  reg = BIT32_GET_FIELD(MY_REGISTER_BITFIELD_A, 0x98765432);            TEST("BIT32_GET_FIELD", 1, reg, 0x987);
  reg = BIT32_MASK(MY_REGISTER_BITFIELD_A);                             TEST("BIT32_MASK", 1, reg, 0x00000fff);

  printf("--------------------------------\n");
  printf("test result: %s\n", (status == OK) ? "PASSED" : "FAILED");

  return status;
}

int main(int argc, const char *argv[])
{
  array_test();
  bit32_test();
  return 0;
}

