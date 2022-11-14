/******************************************************************************
 * (c) Copyright 2019-2021 Digital Dynamics Inc. Scotts Valley CA USA.
 * Unpublished copyright. All rights reserved. Contains proprietary and
 * confidential trade secrets belonging to DDI. Disclosure or release without
 * prior written authorization of DDI is prohibited.
 *****************************************************************************/

#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include "ddi_sdk_common.h"

//ain aout compare band
#define AIN_AOUT_COMPARE_BAND 60

uint16_t compute_scaled_sin(float angle, float phase)
{
    float sinvalue,sv1,sv2;

    sinvalue = sinf(angle+phase);
    sv1 = sinvalue+ 1.0;  // shift range from -1 to 1 to 0 to 2
    sv2 = sv1/2.0; // shift range to 0 to 1

    // scale to integer -  0 to 0xFFFF
    // out d/a in 0 to 10 v
    uint16_t sinvalueint = (uint16_t)(sv2 * 62000.);
    sinvalueint += 500;
  //  printf ("angle  %3.3f sinvalue %3.3f  sv2 %2.3f  sinvalueint %04X \n",angle,sinvalue,sv2,sinvalueint);
    return sinvalueint;
}

uint16_t compare_ain_aout (int16_t ain_value, int16_t compare_value)
{
  ain_value = ain_value & 0xffff;
  compare_value = compare_value & 0xffff;

  if( compare_value & 0x8000)
   compare_value = 0;

  if(abs(ain_value - compare_value ) > AIN_AOUT_COMPARE_BAND) {
      ddi_sdk_log_error(" ain value = 0x%x \n", ain_value);
      ddi_sdk_log_error(" compare_value = 0x%x \n", compare_value);
      ddi_sdk_log_error(" abs(int((int)compare_value - (int)ain_value)) = %d \n", abs(int((int)compare_value - (int)ain_value)));
      return 1;
  }

  return 0;
}

