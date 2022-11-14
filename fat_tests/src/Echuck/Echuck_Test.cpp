/**************************************************************************
(c) Copyright 2022 Digital Dynamics Inc. Scotts Valley CA USA.
Unpublished copyright. All rights reserved. Contains proprietary and
confidential trade secrets belonging to DDI. Disclosure or release without
prior written authorization of DDI is prohibited.
**************************************************************************/

// For console output and gflag testing
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <iostream>
#include <inttypes.h>
#include "DDIEMTestFixture.h"
#include "EnvironmentRegistry.h"
#include "DDIEMUtility.h"
#include "ddi_defines.h"
#include "Echuck_Test.h"

// This particular confifguration has an output offset at 39
#define OUTPUT_OFFSET 0 
// This particular configuration has an input offset at 39
#define INPUT_OFFSET  0

// Represent the DIN and DIN readback
// PACKED_BEGIN
// typedef struct PACKED {
//   uint8_t din;
//   uint8_t din_rb;
// } dio8x_input_pd;
// PACKED_END


// Store the input values
echuck_in_t g_input_pd_echuck;

// Store the output values
echuck_out_t g_output_pd_echuck;

// Called when the cyclic process data receive is complete
void echuck_process_data_callback (void *args)
{
  uint8_t dout_value;
  uint16_t aout_value;
  static int timer = 0;
  static bool is_toggled = 0;
  uint count;
  pd_callback_args *callback_args;
  ddi_em_state current_master_state;

  if ( args == NULL )
  {
    ELOG("Callback arguments are NULL \n");
    return;
  }
  if(timer < 1000)
  {
    timer++;
    return;
  }
  callback_args = (pd_callback_args *)args;

  ddi_em_get_master_state(callback_args->em_handle,&current_master_state);
  if ( current_master_state != DDI_EM_STATE_OP)
    return; // Don't do nuthin' 'less the master is in OP mode

  if(is_toggled)
  {
    dout_value = 0;
    aout_value = 0x8000;
    is_toggled = 0;
  }
  else
  {
    dout_value = 0xff;
    aout_value = 0x7fff;
    is_toggled = 1;
  }

  for(int i = 0; i < 8; i++)
  {
    g_output_pd_echuck.digital_out[i] = dout_value;
    if(i < 4){
      g_output_pd_echuck.analog_out[i] = aout_value;
    }
  }
  ddi_em_set_process_data(callback_args->em_handle, OUTPUT_OFFSET, (uint8_t*)&g_output_pd_echuck, sizeof(g_output_pd_echuck));

  // Receive input process data
  ddi_em_get_process_data(callback_args->em_handle, callback_args->es_cfg->pd_input_offset, (uint8_t*)&g_input_pd_echuck, sizeof(g_input_pd_echuck), DDI_EM_FALSE);
  clrscr();
  for (auto i = 0; i < (sizeof(g_input_pd_echuck.analog_in) / 2);  i++)
  {
    printf("AIN %d: %f\n", i+1, ddi_fusion_convert_hex_to_volts(g_input_pd_echuck.analog_in[i]));  
  }
  for (auto i = 0; i < (sizeof(g_input_pd_echuck.digital_in));  i++)
  {
    printf("DIN %d: 0x%02x\n", i+1, g_input_pd_echuck.digital_in[i]);  
  }
  for (auto i = 0; i < (sizeof(g_output_pd_echuck.analog_out) / 4);  i++)
  {
    printf("AOUT %d: %f\n", i+1, ddi_fusion_convert_hex_to_volts(g_output_pd_echuck.analog_out[i]));  
  }
  for (auto i = 0; i < (sizeof(g_output_pd_echuck.digital_out));  i++)
  {
    printf("DOUT %d: 0x%02x\n", i+1, g_output_pd_echuck.digital_out[i]);  
  }
  timer = 0;
}

// Convert from signed 16-bit hex value to floating point voltage
float ddi_fusion_convert_hex_to_volts (uint16_t input_in_hex)
{
  return (float)((int16_t)input_in_hex/3276.8f);
}

// Convert from floating point voltage to unsigned 16-bit hex value
uint16_t ddi_fusion_convert_volts_to_hex (float input_in_volts)
{
  if ( input_in_volts == 10.0 )
  {
    return 0x7FFF;
  }

return (uint16_t)(input_in_volts * 3276.8f);
}

// create a test to bring the whole thing to op mode, regular start.
TEST_F(DDIEMTestFixture, DDIEM_Echuck_OP_Test)
{
  DDIEMUtility m_ddi_em_utility;

  // Seed the random number generator
  // srand(time(0));

  // Process data callback arguments
  pd_callback_args pd_callback_args;

  // configure thread_manager + ddi_em_configure_master
  SetFixtureStatus(configure());
  ASSERT_EQ(GetFixtureStatus(), DDI_EM_STATUS_OK);
  
  // Open the Fusion with vendor and product ID
  SetFixtureStatus(ddi_em_open_slave_by_id(GetEtherCATMasterHandle(), DDI_ETHERCAT_VENDOR_ID, DDI_FUSION_PRODUCT_CODE, 0, 0, GetEtherCATSlaveHandlePointer()));

  ASSERT_EQ(GetFixtureStatus(), DDI_EM_STATUS_OK);

  // Retrieve the Fusion process data information
  SetFixtureStatus(ddi_em_get_slave_config(GetEtherCATMasterHandle(), GetEtherCATSlaveHandle(), GetEtherCATSlaveConfigPointer()));
  // ddi_em_get_slave_config returns DDI_EM_STATUS_OK if successful
  ASSERT_EQ(GetFixtureStatus(), DDI_EM_STATUS_OK);

  pd_callback_args.em_handle = GetEtherCATMasterHandle();
  pd_callback_args.es_cfg = GetEtherCATSlaveConfigPointer();

  // Register the cyclic callback
  SetFixtureStatus(ddi_em_register_cyclic_callback(GetEtherCATMasterHandle(), echuck_process_data_callback, &pd_callback_args));
  // ddi_em_register_cyclic_callback returns 0 if successful
  ASSERT_EQ(GetFixtureStatus(), DDI_EM_STATUS_OK);

  // Register the notification event handler
  SetFixtureStatus(ddi_em_set_event_handler(GetEtherCATMasterHandle(), event_handler));
  // ddi_em_set_event_handler returns 0 if successful
  ASSERT_EQ(GetFixtureStatus(), DDI_EM_STATUS_OK);

  // Set the EtherCAT Master State to OP mode
  // This will also set the EtherCAT slave(s) mode to the desired mode
  SetFixtureStatus(ddi_em_set_master_state(GetEtherCATMasterHandle(), DDI_EM_STATE_OP, TEST_DEFAULT_TIMEOUT));
  // ddi_em_set_master_state returns 0 if successful
  ASSERT_EQ(GetFixtureStatus(), DDI_EM_STATUS_OK);

  // Sleep for 50 minutes
  sleep(SEC_PER_MIN*5);

  // Return test failure if any iterations did not pass
  // ASSERT_EQ(g_test_iterations_failed_echuck, 0);

  // Stop the cylcic thread started by ddi_em_cyclic_task_start() 
  SetFixtureStatus(ddi_em_cyclic_task_stop(GetEtherCATMasterHandle()));
  // ddi_em_cyclic_task_stop returns 0 if successful
  ASSERT_EQ(GetFixtureStatus(), DDI_EM_STATUS_OK);
}
