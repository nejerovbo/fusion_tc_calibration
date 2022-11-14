// Provides the process data mapping for a Fusion.IO

#ifndef __CRAM_PD_H__
#define __CRAM_PD_H__

#include <stdint.h>

typedef struct __attribute__((__packed__))
{
  uint16_t aout_8x_slot4[8];
  uint16_t aout_8x_slot5[8];
  uint16_t aout_8x_slot6[8];
} ddi_6_slot_pd_out_rim_t;

typedef struct __attribute__((__packed__))
{
  uint16_t dout_opto_slot2;
  uint16_t dout_opto_slot4;
  uint16_t dout_opto_slot6;
  uint16_t aout_x16_slot10[16];
} ddi_12_slot_pd_out_rim_t;

typedef struct __attribute__((__packed__))
{
  uint16_t ain_8x_slot1[8];
  uint16_t ain_8x_slot2[8];
  uint16_t ain_8x_slot3[8];
} ddi_6_slot_pd_in_rim_t;

typedef struct __attribute__((__packed__))
{
  uint16_t din_5v_slot1;
  uint16_t dout_opto_slot2_readback;
  uint16_t din_5v_slot3;
  uint16_t dout_opto_slot4_readback;
  uint16_t din_5v_slot5;
  uint16_t dout_opto_slot6_readback;
  uint16_t ain_temp_slot7[4];
  uint16_t ain_tempTC_slot8[8];
  uint16_t ain_tempTC_slot9[8];
  uint16_t ain_x16_slot11[16];
  uint16_t ain_temp_slot12[4];

} ddi_12_slot_pd_in_rim_t;


typedef struct __attribute__((__packed__))
{
  ddi_6_slot_pd_out_rim_t six_slot_pd_out_rim[2];
  ddi_12_slot_pd_out_rim_t twelve_slot_pd_out_rim[2];
  uint16_t dout_source_slot4;
  uint16_t dout_source_slot5;
  uint16_t dout_source_slot6;
  uint16_t dout_source_slot7;
  uint16_t dout_source_slot8;
  uint16_t dout_source_slot9;
  uint16_t aout_x16_slot14[16];
  uint16_t dout_source_slot16;
  uint16_t aout_x16_slot18[16];
  uint8_t  dout_relay_slot19;
  uint8_t  dout_opto_slot21;
  uint8_t  dout_relay_slot22;
  uint8_t  dout_opto_slot24;
  uint32_t mode; // Digital, AC or DC
  uint32_t reset_compare;
  char* pattern_type;
} CRAM_pd_out_config_t;

typedef struct __attribute__((__packed__))
{
  uint64_t timestamp;
  uint16_t event_status[6];
  ddi_6_slot_pd_in_rim_t six_slot_pd_in_rim[2];
  ddi_12_slot_pd_in_rim_t twelve_slot_pd_in_rim[2];
  uint16_t din_24v_slot1;
  uint16_t din_24v_slot2;
  uint16_t din_24v_slot3;
  uint16_t dout_source_slot4_readback;
  uint16_t dout_source_slot5_readback;
  uint16_t dout_source_slot6_readback;
  uint16_t dout_source_slot7_readback;
  uint16_t dout_source_slot8_readback;
  uint16_t dout_source_slot9_readback;
  uint16_t din_24v_slot10;
  uint16_t din_24v_slot11;
  uint16_t din_24v_slot12;
  uint16_t ain_x16_slot13[16];
  uint16_t ain_x16_slot15[16];
  uint16_t dout_source_slot16_readback;
  uint16_t din_24v_slot17;
  uint8_t  dout_relay_slot19_readback;
  uint16_t din_24v_slot20;
  uint8_t  dout_opto_slot21_readback;
  uint8_t  dout_relay_slot22_readback;
  uint16_t din_24v_slot23;
  uint8_t  dout_opto_slot24_readback;

} CRAM_pd_in_config_t;

#define CRAM_6_SLOT_AIN_START   0  // RIM1, J1
#define CRAM_6_SLOT_AOUT_START  0  // RIM1, J2
#define CRAM_12_SLOT_AIN_START  68 // RIM3, J2/J4
#define CRAM_12_SLOT_AOUT_START 48 // RIM3, J2/J4
#define CRAM_RIM_5_DIN_START    12 // RIM5, J8
#define CRAM_RIM_5_DOUT_START   6  // RIM5, J8

#endif

