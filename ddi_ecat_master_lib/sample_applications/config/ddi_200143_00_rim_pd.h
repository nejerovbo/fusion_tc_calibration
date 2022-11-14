/**************************************************************************
(c) Copyright 2021 Digital Dynamics Inc. Scotts Valley CA USA.
Unpublished copyright. All rights reserved. Contains proprietary and
confidential trade secrets belonging to DDI. Disclosure or release without
prior written authorization of DDI is prohibited.
**************************************************************************/

#ifndef DDI_200143_00_RIM_PD
#define DDI_200143_00_RIM_PD

// This file provides support for the 10-018064-00-0008 platform provided to Mattson
// For evaluation purposes

#define CH16 16
#define CH8  8

#include <stdint.h>
#include "ddi_em_api.h"

// Input process data
typedef struct
{
  uint16_t ain_slot1[CH16];      // 16-Channel AIN
  uint16_t ain_slot4[CH16];      // 16-Channel AIN
  uint16_t din_slot5;            // 16-Channel DIN
  uint16_t dout_readback_slot_6; // 16-Channel DOUT readback
} six_slot_pd_in_struct;

// Output process data
typedef struct
{
  uint16_t aout_slot2[CH8];      // 8-Channel AOUT
  uint16_t aout_slot3[CH8];      // 8-Channel AOUT
  uint16_t dout_slot_6;          // 16-Channel DOUT
} six_slot_pd_out_struct;

#endif //DDI_200143_00_RIM_PD
