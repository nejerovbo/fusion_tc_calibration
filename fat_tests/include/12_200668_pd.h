/*****************************************************************************
 * (c) Copyright 2022 Digital Dynamics Inc. Scotts Valley CA USA.
 * Unpublished copyright. All rights reserved. Contains proprietary and
 * confidential trade secrets belonging to DDI. Disclosure or release without
 * prior written authorization of DDI is prohibited.
 *****************************************************************************/

#include <stdint.h>

#define CH8 8

// UCSC input PD definition
typedef struct
{
   uint16_t dout16_rb_slot1;
   uint16_t dout16_rb_slot4;
} UCSC_pd_in_t;

// UCSC output PD definition
typedef struct
{
   uint16_t dout16_slot1;
   uint16_t dout16_slot4;
} UCSC_pd_out_t;

// Input process definition for the 22-200668-00 Field-Connect
typedef struct
{
   uint16_t ain_slot4[CH8];
   uint16_t ain_slot15[CH8];
   uint16_t din_rb_slot16;
} FC_200688_pd_in_t;

// Output process definition for the 22-200668-00 Field-Connect
typedef struct 
{
   uint16_t aout_slot14[CH8];
   uint16_t dout_slot16;
} FC_200688_pd_out_t;

// Input process definition for the 22-200668-00 Field-Connect Mirror
typedef struct
{
   uint16_t ain_slot2[CH8];
   uint16_t din_slot4;
} FC_200688_mirror_pd_in_t;

// Output process definition for the 22-200668-00 Field-Connect Mirror
typedef struct
{
   uint16_t aout_slot3[CH8];
} FC_200688_mirror_pd_out_t;

// EtherCAT Input process data - get with ddi_em_get_process_data()
typedef struct {
  FC_200688_pd_in_t FC_200688_UUT_pd_in;
  FC_200688_mirror_pd_in_t FC_200688_mirror_pd_in;
} network_pd_in_t;

// EtherCAT Output process data - set with ddi_em_set_process_data()
typedef struct {
  FC_200688_pd_out_t FC_200688_UUT_pd_out;
  FC_200688_mirror_pd_out_t FC_200688_mirror_pd_out;
} network_pd_out_t;

/************************************ Sample Usage *******************/

// Declare input and output structure
network_pd_in_t in_pd;
network_pd_out_t out_pd;

void cyclic_frame ()
{
   // Set the DOUT value of the UUT
   out_pd.FC_200688_UUT_pd_out.dout_slot16 = 3;

   // Get EtherCAT Process data
   ddi_em_get_process_data(callback_args->em_handle, INPUT_OFFSET, (uint8_t *)&in_pd, sizeof(in_pd), DDI_EM_FALSE);

   // Set EtherCAT Process data
   ddi_em_set_process_data(callback_args->em_handle, OUTPUT_OFFSET, (uint8_t *)&out_pd, sizeof(out_pd), DDI_EM_FALSE);

   // Print out the 
   printf("AIN value 0x%x \n", in_pd.FC_200688_mirror_pd_out.ain_slot2[0]);
}

// printf("")
