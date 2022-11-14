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

// Perform I/O loopback for E-84 cards.  This file can be made more generic at a later time

// Error/passed counters
uint g_test_iterations_failed = 0;
uint g_test_iterations_passed = 0;

// This particular configuration uses 3 DIOx8 modules
#define DIO8x_NUMBER_OF_MODULES 3
// This particular confifguration has an output offset at 68
#define OUTPUT_OFFSET 68
// This particular configuration has an input offset at 168
#define INPUT_OFFSET  168

/* 
 * This define is how many frames in the future the output will be read back
 * The output takes 1 cycle to be sent out.  The output will be at the SOUT worst-case 333 us later
 * The DIN has a ~333 us delay.  This DIN will be latched the following train cycle, and availble
 * for read the following cyclic frame:
 * 
 *          <-------------------------------------  3 Frame Delay --------------------------------->
 *   Cyclic Frame N         Cyclic Frame N+1                 Cyclic Frame N+2                   Cyclic Frame N+3
 *          |                   |                               |                                  |
 *          | ^                 |       ^             ^         | ^                          ^     |
 *          | |                 |       |             |         | |                          |     |
 *          | Data queued       |  SOUT asserted   DIN changed  |  DIN latched       DIN avilable  |
 *          | for next cycle    |    at the FC      in S7       |   by ECAT               in the   |
 *          | in Master         |                               |   App           following cycle  |
 *
 *  Keep the pipeline long enough to handle all four stages
 *  
 */
#define PIPELINE_CYCLE_DELAY 4

// Represent the DIN and DIN readback
PACKED_BEGIN
typedef struct PACKED {
  uint8_t din;
  uint8_t din_rb;
} dio8x_input_pd;
PACKED_END

// Store the input values
dio8x_input_pd g_input_pd[DIO8x_NUMBER_OF_MODULES];

// Store the output values in an array PIPELINE_CYCLE_DELAY + 1 long
uint8_t random_output_pipeline[DIO8x_NUMBER_OF_MODULES][PIPELINE_CYCLE_DELAY];
uint8_t random_output[DIO8x_NUMBER_OF_MODULES];

// Handle the pipelining of output data (allow for changing DO output every cycle instead of waiting PIPELINE_CYCLE_DELAY cycles to send new data)
uint8_t handle_pipeline (uint *pipeline_pos)
{
  static uint pipeline_full = 0;

  // Reset the pipeline position if this is at the end
  if ( *pipeline_pos == (PIPELINE_CYCLE_DELAY -1))
  {
    *pipeline_pos = 0;
    // Notify that the pipeline has been filled
    pipeline_full = 1;
    return 1;
  }
  else
  {
    // Pipeline doesn't need to wrap around
    *pipeline_pos = *pipeline_pos + 1;
    // Once pipeline full has been set, keep returning full
    if (pipeline_full)
    {
      return 1;
    }
    else
    {
      return 0;
    }
  }
}

// Called when the cyclic process data receive is complete
void dio_8x_process_data_callback (void *args)
{
  static uint pipeline_wr_pos = 0;
  static uint pipeline_rd_pos = 0;
  uint pipeline_filled, count;
  pd_callback_args *callback_args;
  ddi_em_state current_master_state;
  if ( args == NULL )
  {
    ELOG("Callback arguments are NULL \n");
    return;
  }
  callback_args = (pd_callback_args *)args;

  ddi_em_get_master_state(callback_args->em_handle,&current_master_state);
  if ( current_master_state != DDI_EM_STATE_OP)
    return; // Don't perform compares unless the master is in OP mode

  for ( count = 0; count < DIO8x_NUMBER_OF_MODULES; count++ )
  {
    random_output[count] = rand();
    random_output_pipeline[count][pipeline_wr_pos] = random_output[count];
  }

  pipeline_filled=handle_pipeline(&pipeline_wr_pos);

  ddi_em_set_process_data(callback_args->em_handle, OUTPUT_OFFSET, (uint8_t *)random_output, 3);

  if (pipeline_filled) // Once pipeline has been filled, get the new data
  {
    // Receive input process data
    ddi_em_get_process_data(callback_args->em_handle, INPUT_OFFSET, (uint8_t *)g_input_pd, sizeof(g_input_pd), DDI_EM_FALSE);
    for ( count = 0; count < DIO8x_NUMBER_OF_MODULES; count++ )
    {
      // Get the output value for comparison
      uint8_t output_value_for_compare = random_output_pipeline[count][pipeline_rd_pos];
      // Compare the output vs the input from PIPELINE_CYCLE_DELAY - 1 cycles ago
      if (g_input_pd[count].din != output_value_for_compare)
      {
        g_test_iterations_failed++;
        ELOG("Module %d g_input_pd[count].din 0x%x output_compare 0x%x \n", count, g_input_pd[count].din, output_value_for_compare);
      }
      else
      {
        g_test_iterations_passed++;
      }
    }
    // Adjust the read position in the pipeline
    handle_pipeline(&pipeline_rd_pos);
  }
}

// create a test to bring the whole thing to op mode, regular start.
TEST_F(DDIEMTestFixture, DDIEM_E84_loopback)
{
  DDIEMUtility m_ddi_em_utility;

  // Seed the random number generator
  srand(time(0));

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

  pd_callback_args.es_cfg = GetEtherCATSlaveConfigPointer();

  pd_callback_args.em_handle = GetEtherCATMasterHandle();
  // Register the cyclic callback
  SetFixtureStatus(ddi_em_register_cyclic_callback(GetEtherCATMasterHandle(), dio_8x_process_data_callback, &pd_callback_args));
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
  ASSERT_EQ(g_test_iterations_failed, 0);

  // Stop the cylcic thread started by ddi_em_cyclic_task_start() 
  SetFixtureStatus(ddi_em_cyclic_task_stop(GetEtherCATMasterHandle()));
  // ddi_em_cyclic_task_stop returns 0 if successful
  ASSERT_EQ(GetFixtureStatus(), DDI_EM_STATUS_OK);
}
