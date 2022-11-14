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
#include <stdint.h>

#include "DDIEMTestFixture.h"
#include "EnvironmentRegistry.h"
#include "DDIEMUtility.h"
#include "ddi_defines.h"

#define CH8 8
#define AIN_AOUT_COMPARE_BAND 16

namespace ECHUCK{
  // Input process definition for the 22-200668-00 Field-Connect
  PACKED_BEGIN
  // UUT
  typedef struct PACKED{
    uint16_t ain_slot4[CH8];
    uint16_t ain_slot15[CH8];
    uint16_t din_rb_slot16;
  } FC_200688_pd_in_t;// UUT IN's

  // Output process definition for the 22-200668-00 Field-Connect
  typedef struct PACKED{
    uint16_t aout_slot14[CH8];
    uint16_t dout_slot16;// Set lower two bits, four frames later check on mirror
  } FC_200688_pd_out_t;// UUT OUT

  // Input process definition for the 22-200668-00 Field-Connect Mirror
  typedef struct PACKED{
    uint16_t ain_slot2[CH8];
    uint16_t din_slot4;
  } FC_200688_mirror_pd_in_t;// 

  // Output process definition for the 22-200668-00 Field-Connect Mirror
  typedef struct PACKED{
    uint8_t pad[16]; // Dummy Padding for offset
    uint16_t aout_slot3[CH8];
  } FC_200688_mirror_pd_out_t;

  // EtherCAT Input process data - get with ddi_em_get_process_data()
  typedef struct PACKED{
    FC_200688_pd_in_t FC_200688_UUT_pd_in;
    FC_200688_mirror_pd_in_t FC_200688_mirror_pd_in;
  } network_pd_in_t;

  // EtherCAT Output process data - set with ddi_em_set_process_data()
  typedef struct PACKED{
    FC_200688_pd_out_t FC_200688_UUT_pd_out;
    FC_200688_mirror_pd_out_t FC_200688_mirror_pd_out;
  } network_pd_out_t;
  PACKED_END


  int count = 0;
  const int SEND_CYCLE = 0;
  const int READ_CYCLE = 5;
  const int RESET_CYCLE = READ_CYCLE + 1;

  // Works for Both UUT and MIRROR
  const int PIN_ONE_BIT = 1;
  const int PIN_TWO_BIT = 2;

  const int MIRROR_PIN_FIVE_AIN_CHANNEL = 0;

  const int MIRROR_PIN_SEVEN_AOUT_CHANNEL = 0;
  const int MIRROR_PIN_EIGHT_AOUT_CHANNEL = 1;

  const int UUT_PIN_FIVE_AOUT_CHANNEL = 0;

  const int UUT_PIN_SEVEN_AIN_CHANNEL = 0;
  const int UUT_PIN_EIGHT_AIN_CHANNEL = 1;

  // Process Data Structures
  network_pd_out_t network_pd_out;
  network_pd_in_t network_pd_in;

  // Call back variables
  pd_callback_args *callback_args;
  ddi_em_state current_master_state;

  // Test Values
  // Analogs
  int analog_test_index = 0;
  const int ANALOG_TEST_SIZE = 6;
  uint16_t analog_test_values[ANALOG_TEST_SIZE] = {0x7FFF,0x4000,0x0000,0xc000,0x8000,0x0000}; // Default test values 10v,5v,0v,-5v,-10v,rand

  // Digital
  int digital_test_index = 0;
  const int DIGTAL_TEST_SIZE = 3;
  uint16_t digital_test_values[DIGTAL_TEST_SIZE] = {0x0000,0x0003,0x0000}; // Default tests bits on, bits off, random

  // Compares analog signals, if difference is greater then BAND returns 1 else 0
  uint16_t compare_ain_aout (int16_t ain_value, int16_t compare_value){
    ain_value = ain_value & 0xffff;
    compare_value = compare_value & 0xffff;
    //compare_value = 0.9*compare_value;

    if(abs(ain_value - compare_value ) > AIN_AOUT_COMPARE_BAND) {
     return 1;
    }
    return 0;
  }

  // Set UUT pins 1 & 2
  void set_uut_dout(uint16_t value){
    network_pd_out.FC_200688_UUT_pd_out.dout_slot16 = value;
  }

  // Set UUT pin 5
  void set_uut_aout_channel(uint16_t set_value){
    network_pd_out.FC_200688_UUT_pd_out.aout_slot14[UUT_PIN_FIVE_AOUT_CHANNEL] = set_value;
  }

  // Set MIRROR set pin 7 & 8
  void set_mirror_aout(int channel, uint16_t val){
    network_pd_out.FC_200688_mirror_pd_out.aout_slot3[channel] = val;
  }

  // Error/passed counters
  uint g_test_iterations_failed = 0;
  uint g_test_iterations_passed = 0;
  uint total_send_cycles = 0;
  uint total_read_cycles = 0;
  uint total_pin_one_errors = 0;
  uint total_pin_two_errors = 0;
  uint total_pin_five_errors = 0;
  uint total_pin_seven_errors = 0;
  uint total_pin_eight_errors = 0;

  // Digital compare of bit one or two
  void compare_pin_one_or_two(uint16_t pin){
    uint16_t bit_for_inspection = (1<<(pin-1));
    // Compares UUT and MIRROR by given pin. 
    if((network_pd_out.FC_200688_UUT_pd_out.dout_slot16 & bit_for_inspection) != (network_pd_in.FC_200688_mirror_pd_in.din_slot4 & bit_for_inspection)){
      g_test_iterations_failed++;
      if(pin == 1)
        total_pin_one_errors++;
      else
        total_pin_two_errors++;
    }
  }

  // UUT AOUT Check
  void compare_pin_five(){
    if(compare_ain_aout(network_pd_out.FC_200688_UUT_pd_out.aout_slot14[UUT_PIN_FIVE_AOUT_CHANNEL],
        network_pd_in.FC_200688_mirror_pd_in.ain_slot2[MIRROR_PIN_FIVE_AIN_CHANNEL])){
      g_test_iterations_failed++;
      total_pin_five_errors++;
    }
  }

  // UUT AIN Check
  void compare_pin_seven(){
    if(compare_ain_aout(network_pd_in.FC_200688_UUT_pd_in.ain_slot15[UUT_PIN_SEVEN_AIN_CHANNEL],
        network_pd_out.FC_200688_mirror_pd_out.aout_slot3[MIRROR_PIN_SEVEN_AOUT_CHANNEL])){
      g_test_iterations_failed++;
      total_pin_seven_errors++;
    }
  }

  // UUT AIN Check
  void compare_pin_eight(){
    if(compare_ain_aout(network_pd_in.FC_200688_UUT_pd_in.ain_slot15[UUT_PIN_EIGHT_AIN_CHANNEL],
        network_pd_out.FC_200688_mirror_pd_out.aout_slot3[MIRROR_PIN_EIGHT_AOUT_CHANNEL])){      
      g_test_iterations_failed++;
      total_pin_eight_errors++;
    }
  }


  // ### CYCLE START
  void cyclic_callback(void *args){
    if ( args == NULL ){
      ELOG("Callback arguments are NULL \n");
      return;
    }
    callback_args = (pd_callback_args *)args;

    ddi_em_get_master_state(callback_args->em_handle,&current_master_state);
    if ( current_master_state != DDI_EM_STATE_OP)
      return; // Don't perform compares unless the master is in OP mode

    if(count == SEND_CYCLE){
      // SET DATA
      // UUT: DOUT,AOUT
      set_uut_dout(digital_test_values[digital_test_index]);// two pins
      set_uut_aout_channel(analog_test_values[analog_test_index]);// one pin
      // MIRROR: DOUT
      set_mirror_aout(MIRROR_PIN_SEVEN_AOUT_CHANNEL,analog_test_values[analog_test_index]);// one pin
      set_mirror_aout(MIRROR_PIN_EIGHT_AOUT_CHANNEL,analog_test_values[analog_test_index]);// one pin

      // SEND DATA      
      ddi_em_set_process_data(callback_args->em_handle, callback_args->es_cfg->pd_output_offset,(uint8_t*)&network_pd_out,sizeof(network_pd_out));

      // Increment and reset Iterators and random test case
      analog_test_index++;
      if(analog_test_index ==  ANALOG_TEST_SIZE){
        analog_test_index = 0;
        analog_test_values[ANALOG_TEST_SIZE -1] = rand() & 0xFFFF;// Create New Rand Test, 16 bits
      }

      digital_test_index++;
      if(digital_test_index == DIGTAL_TEST_SIZE){
        digital_test_index = 0;
        digital_test_values[DIGTAL_TEST_SIZE -1] = rand() & 0x0003;// Create new Rand Test first two bits
      }

      total_send_cycles++;

    }else if(count == READ_CYCLE){
      // READ VALUES
      ddi_em_get_process_data(callback_args->em_handle, callback_args->es_cfg->pd_input_offset, (uint8_t *)&network_pd_in, sizeof(network_pd_in), DDI_EM_FALSE);

      // Compares with pins: [1,2,5,7,8]
      compare_pin_one_or_two(PIN_ONE_BIT);
      compare_pin_one_or_two(PIN_TWO_BIT);
      compare_pin_five();
      compare_pin_seven();
      compare_pin_eight();
      total_read_cycles++;
    }


    count++;
    if(count == RESET_CYCLE){
      count = 0;
    }
  }
}// End NAMESPACE ECHUCK



// create a test to bring the whole thing to op mode, regular start.
TEST_F(DDIEMTestFixture, EChuck_IO_TEST)
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

  pd_callback_args.em_handle = GetEtherCATMasterHandle();
  pd_callback_args.es_cfg = GetEtherCATSlaveConfigPointer();

  // Register the cyclic callback
  SetFixtureStatus(ddi_em_register_cyclic_callback(GetEtherCATMasterHandle(), ECHUCK::cyclic_callback, &pd_callback_args)); 
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



  // Infinite Loop: prints every second errors coming from which pin
  for(;;){
    printf("EChuck Errors: %d Sends: %d Reads: %d Pin_1: %d Pin_2: %d Pin_5: %d Pin_7: %d Pin_8: %d\n", 
      ECHUCK::g_test_iterations_failed,
      ECHUCK::total_send_cycles,
      ECHUCK::total_read_cycles,
      ECHUCK::total_pin_one_errors,ECHUCK::total_pin_two_errors,ECHUCK::total_pin_five_errors,ECHUCK::total_pin_seven_errors,ECHUCK::total_pin_eight_errors);
      // printf("UUT_DOUT: %d MIRROR_DIN: %d\n",ECHUCK::UUT_DOUT,ECHUCK::MIRROR_DIN);
    sleep(1);
  }
  
  // Sleep for 50 minutes
  // sleep(SEC_PER_MIN*5);

  // Return test failure if any iterations did not pass
  ASSERT_EQ(ECHUCK::g_test_iterations_failed, 0);

  // Stop the cylcic thread started by ddi_em_cyclic_task_start() 
  SetFixtureStatus(ddi_em_cyclic_task_stop(GetEtherCATMasterHandle()));
  // ddi_em_cyclic_task_stop returns 0 if successful
  ASSERT_EQ(GetFixtureStatus(), DDI_EM_STATUS_OK);
}
