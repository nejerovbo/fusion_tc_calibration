/******************************************************************************
 * (c) Copyright 2020-2021 Digital Dynamics Inc. Scotts Valley CA USA.
 * Unpublished copyright. All rights reserved. Contains proprietary and
 * confidential trade secrets belonging to DDI. Disclosure or release without
 * prior written authorization of DDI is prohibited.
 *****************************************************************************/
#ifndef DDI_SDK_RIM_CONFIG_H
#define DDI_SDK_RIM_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

// This file provides a mapping of the default module mapping for certain DDI RIM types

/* change these if the number of rims changes, make clean; make all after */
#define NUMBER_OF_6_SLOT_RIM 0 
#define NUMBER_OF_12_SLOT_RIM 0

//support for ddi rim types

typedef struct __attribute__((__packed__))
{
  uint16_t standard_dout_J1;
} ddi_6_slot_dout_rim_t;

typedef struct __attribute__((__packed__))
{
  uint16_t opto_dout_J1;
  uint8_t  relay_dout_J2;
  uint8_t  relay_dout_J3;
} ddi_12_slot_dout_rim_t;

typedef struct __attribute__((__packed__))
{
  uint16_t standard_din_J1;
} ddi_6_slot_din_rim_t;

typedef struct __attribute__((__packed__))
{
  uint16_t unknown_din_J8;
  uint16_t relay_din_J2_J3;
  uint16_t opto_din_J1;
} ddi_12_slot_din_rim_t;

typedef struct __attribute__((__packed__))
{
  uint16_t ain_16x_J4_J5[16];
  uint16_t ain_16x_J2_J3[16];
} ddi_6_slot_ain_rim_t;

typedef struct __attribute__((__packed__))
{
  uint16_t ain_8x_J6[8];
  uint16_t ain_8x_J7[8];
  uint16_t ain_8x_J5[8];
  uint16_t ain_8x_J4[8];
} ddi_12_slot_ain_rim_t;

typedef struct __attribute__((__packed__))
{
  uint16_t aout_8x_J4_J5[8];
  uint16_t aout_8x_J2_J3[8];
} ddi_6_slot_aout_rim_t;

typedef struct __attribute__((__packed__))
{
  uint16_t aout_8x_J4_J6[8];
  uint16_t aout_8x_J5_J7[8];
} ddi_12_slot_aout_rim_t;

int compare_RIM_analog(ddi_12_slot_ain_rim_t *ddi_12_slot_ai,
                      ddi_6_slot_ain_rim_t *ddi_6_slot_ai,
                      ddi_12_slot_aout_rim_t *ddi_12_slot_aout,
                      ddi_6_slot_ain_rim_t *ddi_6_slot_aout
                      );

int update_RIM_IO(void);

#ifdef __cplusplus
}
#endif

#endif // DDI_SDK_RIM_CONFIG_H

