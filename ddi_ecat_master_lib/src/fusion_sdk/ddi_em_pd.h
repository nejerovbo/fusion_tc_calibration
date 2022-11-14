/**************************************************************************
(c) Copyright 2022 Digital Dynamics Inc. Scotts Valley CA USA.
Unpublished copyright. All rights reserved. Contains proprietary and
confidential trade secrets belonging to DDI. Disclosure or release without
prior written authorization of DDI is prohibited.
**************************************************************************/

#ifndef DDI_EM_PD_H
#define DDI_EM_PD_H

// Defines Fusion process data structure

#include <stdint.h>

// Represents an Fusion EtherCAT process data descriptor
typedef struct
{
  uint16_t byte_offset;  // byte offset into process data
  uint8_t  lo;           // low byte
  uint8_t  hi;           // high byte
  uint32_t mask;         // bitmask
  uint32_t size;         // bitsize
  uint8_t  *pd_input;    // input process data memory
  uint8_t  *pd_output;   // output process data memory
} fusion_pd_desc_t;

#endif
