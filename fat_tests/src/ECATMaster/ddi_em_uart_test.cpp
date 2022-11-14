/**************************************************************************
(c) Copyright 2022 Digital Dynamics Inc. Scotts Valley CA USA.
Unpublished copyright. All rights reserved. Contains proprietary and
confidential trade secrets belonging to DDI. Disclosure or release without
prior written authorization of DDI is prohibited.
**************************************************************************/
#include <fcntl.h>
#include <termios.h>
#include <inttypes.h>
#include <stdio.h>
#include <iostream>
#include <inttypes.h>
#include "EnvironmentRegistry.h"
#include "DDIEMUtility.h"
#include "EnvironmentRegistry.h"
#include "DDIEMEnvironment.h"
#include "ddi_em_fusion_uart_api.h"
#include "ddi_em_fusion.h"
#include "ddi_fusion_uart_test_fixture.h"

TEST(UART_Init_Suite, DDIEM_UART_Init_Test)
{
  DDIEMEnvironment* environment = (DDIEMEnvironment *)g_environment_registry.GetEnvironment(DDIEM_ENVIRONMENT_NAME);
  EXPECT_NE(nullptr, environment);

  ddi_em_handle em_handle;
  ddi_es_handle es_handle;
  uint16_t index = 0x5005;
  uart_channel channel = UART_CH0;
  uint32_t flags = 0;
  ddi_fusion_uart_handle uart_handle = 0;
  ddi_em_result result;
  ddi_em_init_params init_params;

  memset(&init_params, 0, sizeof(ddi_em_init_params));

  // EtherCAT network adapter to use
  init_params.network_adapter         = DDI_EM_NIC_4;
  // Enable direct call-in from an external thread
  init_params.enable_cyclic_thread    = DDI_EM_TRUE;
  init_params.scan_rate_us            = DDI_EM_DEFAULT_CYCLIC_RATE;
  // Use the default thread priority for the EtherCAT Cyclic Thread
  init_params.polling_thread_priority = DDI_EM_CYCLIC_THREAD_PRI_DEFAULT;

  result = ddi_em_sdk_init();
  EXPECT_EQ(DDI_EM_STATUS_OK, result) << "ddi_em_sdk_init failed: " << ddi_em_get_error_string(result) << "\n";

  //initialize master instance
  result = ddi_em_init(&init_params, &em_handle);
  EXPECT_EQ(DDI_EM_STATUS_OK, result) << "ddi_em_init failed \n";

  // configure master
  result = ddi_em_configure_master(em_handle, environment->GetEniFile().c_str());
  EXPECT_EQ(DDI_EM_STATUS_OK, result);

  // open the fusion with vendor and product ID
  result = ddi_em_open_slave_by_id(em_handle, DDI_ETHERCAT_VENDOR_ID, DDI_FUSION_PRODUCT_CODE, 0, 0, &es_handle);
  EXPECT_EQ(DDI_EM_STATUS_OK, result);

  printf("\nUART handle value before: %d\n", uart_handle);

  // call ddi_fusion_uart_open with correct input
  result = ddi_fusion_uart_open(em_handle, es_handle, index, channel, flags, &uart_handle);
  EXPECT_EQ(DDI_EM_STATUS_OK, result);

  printf("\nUART handle value after: %d\n", uart_handle);

  // close fusion instance
  result = ddi_fusion_uart_close(uart_handle);
  EXPECT_EQ(DDI_EM_STATUS_OK, result);

  // de-init the master instance
  result = ddi_em_deinit(em_handle);
  EXPECT_EQ(DDI_EM_STATUS_OK, result) << "ddi_em_deinit failed \n";

  //de-init the SDK
  result = ddi_em_sdk_deinit();
  EXPECT_EQ(DDI_EM_STATUS_OK, result) << "ddi_em_sdk_deinit failed: " << ddi_em_get_error_string(result) << "\n";
}

TEST(UART_Init_Suite, DDIEM_UART_Init_Test_Negative)
{
  DDIEMEnvironment* environment = (DDIEMEnvironment *)g_environment_registry.GetEnvironment(DDIEM_ENVIRONMENT_NAME);
  EXPECT_NE(nullptr, environment);

  ddi_em_handle em_handle;
  ddi_es_handle es_handle;
  uint16_t index = 0x5005;
  uart_channel channel = UART_CH0;
  uint32_t flags = 0;
  ddi_fusion_uart_handle uart_handle = 0;
  ddi_em_result result;
  ddi_em_init_params init_params;

  memset(&init_params, 0, sizeof(ddi_em_init_params));

  // EtherCAT network adapter to use
  init_params.network_adapter         = DDI_EM_NIC_4;
  // Enable direct call-in from an external thread
  init_params.enable_cyclic_thread    = DDI_EM_TRUE;
  init_params.scan_rate_us            = DDI_EM_DEFAULT_CYCLIC_RATE;
  // Use the default thread priority for the EtherCAT Cyclic Thread
  init_params.polling_thread_priority = DDI_EM_CYCLIC_THREAD_PRI_DEFAULT;

  result = ddi_em_sdk_init();
  EXPECT_EQ(DDI_EM_STATUS_OK, result) << "ddi_em_sdk_init failed: " << ddi_em_get_error_string(result) << "\n";

  //initialize master instance
  result = ddi_em_init(&init_params, &em_handle);
  EXPECT_EQ(DDI_EM_STATUS_OK, result) << "ddi_em_init failed \n";

  // configure master
  result = ddi_em_configure_master(em_handle, environment->GetEniFile().c_str());
  EXPECT_EQ(DDI_EM_STATUS_OK, result);

  // open the fusion with vendor and product ID
  result = ddi_em_open_slave_by_id(em_handle, DDI_ETHERCAT_VENDOR_ID, DDI_FUSION_PRODUCT_CODE, 0, 0, &es_handle);
  EXPECT_EQ(DDI_EM_STATUS_OK, result);

  // call ddi_fusion_uart_open with faulty em_handle input
  result = ddi_fusion_uart_open(255, es_handle, index, channel, flags, &uart_handle);
  EXPECT_NE(DDI_EM_STATUS_OK, result);

  // call ddi_fusion_uart_open with faulty uart_handle pointer input
  result = ddi_fusion_uart_open(255, es_handle, index, channel, flags, nullptr);
  EXPECT_NE(DDI_EM_STATUS_OK, result);

  // de-init the master instance
  result = ddi_em_deinit(em_handle);
  EXPECT_EQ(DDI_EM_STATUS_OK, result) << "ddi_em_deinit failed \n";

  //de-init the SDK
  result = ddi_em_sdk_deinit();
  EXPECT_EQ(DDI_EM_STATUS_OK, result) << "ddi_em_sdk_deinit failed: " << ddi_em_get_error_string(result) << "\n";
}

// ****************                ddi_fusion_uart_test_fixture              **************************

TEST_F(ddi_fusion_uart_test_fixture, DDIEM_UART_param_set_get_baud_rate_test)
{
  SetFixtureStatus(ddi_fusion_uart_set_baud(GetFusionUARTHandle(), baud_list[0]));
  ASSERT_EQ(DDI_EM_STATUS_OK, GetFixtureStatus());

  uart_baud read_back;

  SetFixtureStatus(ddi_fusion_uart_get_baud(GetFusionUARTHandle(), &read_back));
  ASSERT_EQ(DDI_EM_STATUS_OK, GetFixtureStatus());
}

TEST_F(ddi_fusion_uart_test_fixture, DDIEM_UART_param_set_get_baud_rate_negative_test)
{
  // hopefully this works
  uart_baud bad_baud = (uart_baud) -1;
  uart_baud good_baud = UART_BAUD_2400;

  // call set_baud with invalid baud rate, expect error
  SetFixtureStatus(ddi_fusion_uart_set_baud(GetFusionUARTHandle(), bad_baud));
  EXPECT_NE(DDI_EM_STATUS_OK, GetFixtureStatus());

  // call set_baud with invalid handle, expect error
  SetFixtureStatus(ddi_fusion_uart_set_baud(-1, good_baud));
  EXPECT_NE(DDI_EM_STATUS_OK, GetFixtureStatus());
  
  // correct set_baud call
  SetFixtureStatus(ddi_fusion_uart_set_baud(GetFusionUARTHandle(), good_baud));
  EXPECT_EQ(DDI_EM_STATUS_OK, GetFixtureStatus());

  // call get_baud with faulty handle, expect error
  SetFixtureStatus(ddi_fusion_uart_get_baud(-1, &good_baud));
  ASSERT_NE(DDI_EM_STATUS_OK, GetFixtureStatus());

  // call get_baud with nullptr, expect error
  SetFixtureStatus(ddi_fusion_uart_get_baud(GetFusionUARTHandle(), nullptr));
  ASSERT_NE(DDI_EM_STATUS_OK, GetFixtureStatus());
}

TEST_F(ddi_fusion_uart_test_fixture, DDIEM_UART_param_baud_rate_cycle_test)
{
  // used to store baud rate read back, this ensures that our read back value is correct
  uart_baud read_back = UART_BAUD_2400;

  // cycle through the array and exercise set/get functions 
  for ( uart_baud input : baud_list )
  {
    printf("\nSetting baud rate to: %d\n", input);
    SetFixtureStatus(ddi_fusion_uart_set_baud(GetFusionUARTHandle(), input));
    EXPECT_EQ(DDI_EM_STATUS_OK, GetFixtureStatus()) << "\nFaulty baud_rate input: " << input << "\n"; 

    SetFixtureStatus(ddi_fusion_uart_get_baud(GetFusionUARTHandle(), &read_back));
    EXPECT_EQ(DDI_EM_STATUS_OK, GetFixtureStatus()) << "\nFaulty baud_rate read-back: " << read_back << "\n";
    // ensure the read_back and input are the same
    ASSERT_EQ(input, read_back) << "\ninput: " << input << "\nread_back: " << read_back << "\n";
    printf("\nReading baud rate back: %d\n", read_back);
  }

}

TEST_F(ddi_fusion_uart_test_fixture, DDIEM_UART_param_set_get_parity_test)
{
  // set to be different so the compare at the end will be a valid test
  uart_parity input = UART_PARITY_NONE;
  uart_parity read_back = UART_PARITY_ODD;

  SetFixtureStatus(ddi_fusion_uart_set_parity_mode(GetFusionUARTHandle(), input));
  ASSERT_EQ(DDI_EM_STATUS_OK, GetFixtureStatus());

  SetFixtureStatus(ddi_fusion_uart_get_parity_mode(GetFusionUARTHandle(), &read_back));
  ASSERT_EQ(DDI_EM_STATUS_OK, GetFixtureStatus());  

  ASSERT_EQ(input, read_back);
}

TEST_F(ddi_fusion_uart_test_fixture, DDIEM_UART_param_set_get_parity_test_negative)
{
  uart_parity input = UART_PARITY_NONE;
  uart_parity faulty_mcFaultFace = (uart_parity) -1;

  // set with faulty uart handle, expect error
  SetFixtureStatus(ddi_fusion_uart_set_parity_mode(-1, input));
  ASSERT_NE(DDI_EM_STATUS_OK, GetFixtureStatus());

  // set with faulty input, expect error
  SetFixtureStatus(ddi_fusion_uart_set_parity_mode(GetFusionUARTHandle(), faulty_mcFaultFace));
  ASSERT_NE(DDI_EM_STATUS_OK, GetFixtureStatus());

  // correct set mode
  SetFixtureStatus(ddi_fusion_uart_set_parity_mode(GetFusionUARTHandle(), input));
  ASSERT_EQ(DDI_EM_STATUS_OK, GetFixtureStatus());

  // get with faulty uart handle, expect error
  SetFixtureStatus(ddi_fusion_uart_get_parity_mode(-1, &input));
  ASSERT_NE(DDI_EM_STATUS_OK, GetFixtureStatus());  

  // get with nullptr, expect error
  SetFixtureStatus(ddi_fusion_uart_get_parity_mode(GetFusionUARTHandle(), nullptr));
  ASSERT_NE(DDI_EM_STATUS_OK, GetFixtureStatus());  
}

TEST_F(ddi_fusion_uart_test_fixture, DDIEM_UART_param_parity_mode_cycle_test)
{
  // used to store parity_mode read back, this ensures a correct read back value
  uart_parity read_back = UART_PARITY_ODD;

  // cycle through the array and exercise set/get functions
  for ( uart_parity input: parity )
  {
    printf("\nSetting parity mode: %d\n", input);
    SetFixtureStatus(ddi_fusion_uart_set_parity_mode(GetFusionUARTHandle(), input));
    EXPECT_EQ(DDI_EM_STATUS_OK, GetFixtureStatus());

    SetFixtureStatus(ddi_fusion_uart_get_parity_mode(GetFusionUARTHandle(), &read_back));
    EXPECT_EQ(DDI_EM_STATUS_OK, GetFixtureStatus());

    printf("\nParity mode read_back: %d\n", read_back);

    ASSERT_EQ(input, read_back);   
  }
}

TEST_F(ddi_fusion_uart_test_fixture, DDIEM_UART_param_set_get_stop_bits_test)
{
  uart_stop_bits input = UART_STOP_BITS_1;
  
  uart_stop_bits read_back = UART_STOP_BITS_2;

  SetFixtureStatus(ddi_fusion_uart_set_stop_bits(GetFusionUARTHandle(), input));
  ASSERT_EQ(DDI_EM_STATUS_OK, GetFixtureStatus());

  SetFixtureStatus(ddi_fusion_uart_get_stop_bits(GetFusionUARTHandle(), &read_back));
  ASSERT_EQ(DDI_EM_STATUS_OK, GetFixtureStatus());  

  ASSERT_EQ(input, read_back);  
}

TEST_F(ddi_fusion_uart_test_fixture, DDIEM_UART_param_set_get_stop_bits_test_negative)
{
  uart_stop_bits input = UART_STOP_BITS_1;
  uart_stop_bits faulty_mcFaultFace = (uart_stop_bits) -1;

  // set with faulty uart handle, expect error
  SetFixtureStatus(ddi_fusion_uart_set_stop_bits(-1, input));
  ASSERT_NE(DDI_EM_STATUS_OK, GetFixtureStatus());

  // set with faulty input, expect error
  SetFixtureStatus(ddi_fusion_uart_set_stop_bits(GetFusionUARTHandle(), faulty_mcFaultFace));
  ASSERT_NE(DDI_EM_STATUS_OK, GetFixtureStatus());

  // correct set mode
  SetFixtureStatus(ddi_fusion_uart_set_stop_bits(GetFusionUARTHandle(), input));
  ASSERT_EQ(DDI_EM_STATUS_OK, GetFixtureStatus());

  // get with faulty uart handle, expect error
  SetFixtureStatus(ddi_fusion_uart_get_stop_bits(-1, &input));
  ASSERT_NE(DDI_EM_STATUS_OK, GetFixtureStatus());  

  // get with nullptr, expect error
  SetFixtureStatus(ddi_fusion_uart_get_stop_bits(GetFusionUARTHandle(), nullptr));
  ASSERT_NE(DDI_EM_STATUS_OK, GetFixtureStatus());    
}

TEST_F(ddi_fusion_uart_test_fixture, DDIEM_UART_param_stop_bits_cycle_test)
{
  uart_stop_bits read_back = UART_STOP_BITS_2;

  for ( uart_stop_bits input : m_stop_bits )
  {
    printf("\nSetting stop_bits: %d\n", input);
    SetFixtureStatus(ddi_fusion_uart_set_stop_bits(GetFusionUARTHandle(), input));
    EXPECT_EQ(DDI_EM_STATUS_OK, GetFixtureStatus());

    SetFixtureStatus(ddi_fusion_uart_get_stop_bits(GetFusionUARTHandle(), &read_back));
    EXPECT_EQ(DDI_EM_STATUS_OK, GetFixtureStatus());

    printf("\nStop_bits read_back: %d\n", read_back);

    ASSERT_EQ(input, read_back);  
  }
}

TEST_F(ddi_fusion_uart_test_fixture, DDIEM_UART_param_set_get_data_bits_test)
{
  uart_data_bits input = UART_DATA_BITS_5;
  
  uart_data_bits read_back = UART_DATA_BITS_7;

  SetFixtureStatus(ddi_fusion_uart_set_data_bits(GetFusionUARTHandle(), input));
  ASSERT_EQ(DDI_EM_STATUS_OK, GetFixtureStatus());

  SetFixtureStatus(ddi_fusion_uart_get_data_bits(GetFusionUARTHandle(), &read_back));
  ASSERT_EQ(DDI_EM_STATUS_OK, GetFixtureStatus());  

  ASSERT_EQ(input, read_back);
}

TEST_F(ddi_fusion_uart_test_fixture, DDIEM_UART_param_set_get_data_bits_test_negative)
{
  uart_data_bits input = UART_DATA_BITS_5;
  uart_data_bits faulty_mcFaultFace = (uart_data_bits) -1;

  // set with faulty uart handle, expect error
  SetFixtureStatus(ddi_fusion_uart_set_data_bits(-1, input));
  ASSERT_NE(DDI_EM_STATUS_OK, GetFixtureStatus());

  // set with faulty input, expect error
  SetFixtureStatus(ddi_fusion_uart_set_data_bits(GetFusionUARTHandle(), faulty_mcFaultFace));
  ASSERT_NE(DDI_EM_STATUS_OK, GetFixtureStatus());

  // correct set mode
  SetFixtureStatus(ddi_fusion_uart_set_data_bits(GetFusionUARTHandle(), input));
  ASSERT_EQ(DDI_EM_STATUS_OK, GetFixtureStatus());

  // get with faulty uart handle, expect error
  SetFixtureStatus(ddi_fusion_uart_get_data_bits(-1, &input));
  ASSERT_NE(DDI_EM_STATUS_OK, GetFixtureStatus());  

  // get with nullptr, expect error
  SetFixtureStatus(ddi_fusion_uart_get_data_bits(GetFusionUARTHandle(), nullptr));
  ASSERT_NE(DDI_EM_STATUS_OK, GetFixtureStatus());    
}

TEST_F(ddi_fusion_uart_test_fixture, DDIEM_UART_param_data_bits_cycle_test)
{
  uart_data_bits read_back = UART_DATA_BITS_6;

  for ( uart_data_bits input : m_data_bits )
  {
    printf("\ndata_bits mode: %d\n", input);
    SetFixtureStatus(ddi_fusion_uart_set_data_bits(GetFusionUARTHandle(), input));
    EXPECT_EQ(DDI_EM_STATUS_OK, GetFixtureStatus());

    SetFixtureStatus(ddi_fusion_uart_get_data_bits(GetFusionUARTHandle(), &read_back));
    EXPECT_EQ(DDI_EM_STATUS_OK, GetFixtureStatus());

    printf("\ndata_bits read_back: %d\n", read_back);

    ASSERT_EQ(input, read_back);
  }
    
}

TEST_F(ddi_fusion_uart_test_fixture, DDIEM_UART_param_set_get_flow_control_test)
{
  uart_flow_control input = UART_FLOW_CONTROL_OFF;

  uart_flow_control read_back = UART_FLOW_CONTROL_RTS_CTS;

  // set test
  SetFixtureStatus(ddi_fusion_uart_set_flow_control(GetFusionUARTHandle(), input));
  ASSERT_EQ(DDI_EM_STATUS_OK, GetFixtureStatus());

  // get test
  SetFixtureStatus(ddi_fusion_uart_get_flow_control(GetFusionUARTHandle(), &read_back));
  ASSERT_EQ(DDI_EM_STATUS_OK, GetFixtureStatus());

  // check to ensure that input == read_back
  ASSERT_EQ(input, read_back);
}

TEST_F(ddi_fusion_uart_test_fixture, DDIEM_UART_param_set_get_flow_control_test_negative)
{
  uart_flow_control input = UART_FLOW_CONTROL_OFF;
  uart_flow_control faulty_mcFaultFace = (uart_flow_control) -1;

  // set with faulty uart handle, expect error
  SetFixtureStatus(ddi_fusion_uart_set_flow_control(-1, input));
  ASSERT_NE(DDI_EM_STATUS_OK, GetFixtureStatus());

  // set with faulty input, expect error
  SetFixtureStatus(ddi_fusion_uart_set_flow_control(GetFusionUARTHandle(), faulty_mcFaultFace));
  ASSERT_NE(DDI_EM_STATUS_OK, GetFixtureStatus());

  // correct set mode
  SetFixtureStatus(ddi_fusion_uart_set_flow_control(GetFusionUARTHandle(), input));
  ASSERT_EQ(DDI_EM_STATUS_OK, GetFixtureStatus());

  // get with faulty uart handle, expect error
  SetFixtureStatus(ddi_fusion_uart_get_flow_control(-1, &input));
  ASSERT_NE(DDI_EM_STATUS_OK, GetFixtureStatus());  

  // get with nullptr, expect error
  SetFixtureStatus(ddi_fusion_uart_get_flow_control(GetFusionUARTHandle(), nullptr));
  ASSERT_NE(DDI_EM_STATUS_OK, GetFixtureStatus()); 
}

TEST_F(ddi_fusion_uart_test_fixture, DDIEM_UART_flow_control_cycle_test)
{  
  uart_flow_control read_back = UART_FLOW_CONTROL_OFF;

  for( uart_flow_control input : flow_control )
  {
    SetFixtureStatus(ddi_fusion_uart_set_flow_control(GetFusionUARTHandle(), input));
    ASSERT_EQ(DDI_EM_STATUS_OK, GetFixtureStatus());

    SetFixtureStatus(ddi_fusion_uart_get_flow_control(GetFusionUARTHandle(), &read_back));
    ASSERT_EQ(DDI_EM_STATUS_OK, GetFixtureStatus());

    ASSERT_EQ(input, read_back);
  }
}

// Callback mechanism for the UART event tests
void uart_event_callback (uart_event *event, void *user_data)
{
  uint *callback_count;
  if ( user_data == NULL )
  {
    return;
  }
  // Increment the callback count
  callback_count = (uint *)user_data;
  *callback_count = *callback_count + 1;
}

// Test RS-232 loopback test with UART events enabled
TEST_F(ddi_fusion_uart_test_fixture, DDIEM_UART_232_loopback_test)
{
  uint callback_count;
  int test_count = 0;
  DDIEMUtility m_ddi_em_utility;

  uart_pd_callback_args       pd_callback_args;

  pd_callback_args.em_handle = GetEtherCATMasterHandle();
  pd_callback_args.es_cfg = GetEtherCATSlaveConfigPointer();

  // Register the cyclic callback
  SetFixtureStatus(ddi_em_register_cyclic_callback(GetEtherCATMasterHandle(), m_ddi_em_utility.UART_cyclic_function, &pd_callback_args));
  EXPECT_EQ(DDI_EM_STATUS_OK, GetFixtureStatus()) << "ddi_em_register_cyclic_callback has failed\n";

  printf("setting uart event threshold\n");
  // set the EtherCAT Master State to OP mode
  SetFixtureStatus(ddi_em_set_master_state(GetEtherCATMasterHandle(), DDI_EM_STATE_OP, TEST_DEFAULT_TIMEOUT));
  EXPECT_EQ(DDI_EM_STATUS_OK, GetFixtureStatus()) << "ddi_em_set_master_state failed.\n";

  SetFixtureStatus(ddi_fusion_uart_channel_flush(GetFusionUARTHandle()));
  EXPECT_EQ(DDI_EM_STATUS_OK, GetFixtureStatus()) << "channel flush failed\n";

  while ( test_count++ < 10 )
  {
    ASSERT_NO_FATAL_FAILURE(perform_loopback(false));
  }

  // Stop the cylcic thread started by ddi_em_cyclic_task_start()
  SetFixtureStatus(ddi_em_cyclic_task_stop(GetEtherCATMasterHandle()));
  // ddi_em_cyclic_task_stop returns 0 if successful
  ASSERT_EQ(GetFixtureStatus(), DDI_EM_STATUS_OK);
}

// Test RS-232 loopback test with UART events enabled
TEST_F(ddi_fusion_uart_test_fixture, DDIEM_UART_232_loopback_event_test)
{
  uint callback_count;
  int test_count = 0;
  DDIEMUtility m_ddi_em_utility;

  SetFixtureStatus(ddi_fusion_uart_set_baud(GetFusionUARTHandle(), UART_BAUD_115200));

  uart_pd_callback_args       pd_callback_args;

  pd_callback_args.em_handle = GetEtherCATMasterHandle();
  pd_callback_args.es_cfg = GetEtherCATSlaveConfigPointer();

  // Register the cyclic callback
  SetFixtureStatus(ddi_em_register_cyclic_callback(GetEtherCATMasterHandle(), m_ddi_em_utility.UART_cyclic_function, &pd_callback_args));
  EXPECT_EQ(DDI_EM_STATUS_OK, GetFixtureStatus()) << "ddi_em_register_cyclic_callback has failed\n";

  SetFixtureStatus(ddi_fusion_uart_register_event(GetFusionUARTHandle(),uart_event_callback,&callback_count));
  EXPECT_EQ(DDI_EM_STATUS_OK, GetFixtureStatus()) << "uart event register failed\n";

  SetFixtureStatus(ddi_fusion_uart_enable_threshold_event(GetFusionUARTHandle(), UART_EVENT_THRESHOLD_RISING_EDGE, 9));
  EXPECT_EQ(DDI_EM_STATUS_OK, GetFixtureStatus()) << "uart enable threshold failed\n";

  // set the EtherCAT Master State to OP mode
  SetFixtureStatus(ddi_em_set_master_state(GetEtherCATMasterHandle(), DDI_EM_STATE_OP, TEST_DEFAULT_TIMEOUT));
  EXPECT_EQ(DDI_EM_STATUS_OK, GetFixtureStatus()) << "ddi_em_set_master_state failed.\n";

  SetFixtureStatus(ddi_fusion_uart_channel_flush(GetFusionUARTHandle()));
  EXPECT_EQ(DDI_EM_STATUS_OK, GetFixtureStatus()) << "channel flush failed\n";

  while ( test_count++ < 20 )
  {
    ASSERT_NO_FATAL_FAILURE(perform_loopback(false));
    if ( test_count == 5 )
    {
      printf("Disabling uart events...\n");
      ddi_fusion_uart_disable_threshold_event(GetFusionUARTHandle(), UART_EVENT_THRESHOLD_RISING_EDGE | UART_EVENT_THRESHOLD_FALLING_EDGE);
    }
    if ( test_count == 12 )
    {
      printf("Re-enbabling uart events...\n");
      ddi_fusion_uart_enable_threshold_event(GetFusionUARTHandle(), UART_EVENT_THRESHOLD_RISING_EDGE, 25);
    }
  }

  // Compare the number of callbacks versus the expected value
  ASSERT_EQ(13, callback_count);
  printf("%d UART callbacks received \n", callback_count);

  // Stop the cylcic thread started by ddi_em_cyclic_task_start()
  SetFixtureStatus(ddi_em_cyclic_task_stop(GetEtherCATMasterHandle()));
  // ddi_em_cyclic_task_stop returns 0 if successful
  ASSERT_EQ(GetFixtureStatus(), DDI_EM_STATUS_OK);
}

TEST_F(ddi_fusion_uart_test_fixture, DDIEM_UART_232_loopback_event_test2)
{
  uint callback_count;
  int test_count = 0;
  DDIEMUtility m_ddi_em_utility;

  uart_pd_callback_args       pd_callback_args;

  pd_callback_args.em_handle = GetEtherCATMasterHandle();
  pd_callback_args.es_cfg = GetEtherCATSlaveConfigPointer();

  // Register the cyclic callback
  SetFixtureStatus(ddi_em_register_cyclic_callback(GetEtherCATMasterHandle(), m_ddi_em_utility.UART_cyclic_function, &pd_callback_args));
  EXPECT_EQ(DDI_EM_STATUS_OK, GetFixtureStatus()) << "ddi_em_register_cyclic_callback has failed\n";

  SetFixtureStatus(ddi_fusion_uart_register_event(GetFusionUARTHandle(),uart_event_callback,&callback_count));
  EXPECT_EQ(DDI_EM_STATUS_OK, GetFixtureStatus()) << "uart event register failed\n";

  SetFixtureStatus(ddi_fusion_uart_enable_threshold_event(GetFusionUARTHandle(), UART_EVENT_THRESHOLD_RISING_EDGE, 50));
  EXPECT_EQ(DDI_EM_STATUS_OK, GetFixtureStatus()) << "uart enable threshold failed\n";

  // set the EtherCAT Master State to OP mode
  SetFixtureStatus(ddi_em_set_master_state(GetEtherCATMasterHandle(), DDI_EM_STATE_OP, TEST_DEFAULT_TIMEOUT));
  EXPECT_EQ(DDI_EM_STATUS_OK, GetFixtureStatus()) << "ddi_em_set_master_state failed.\n";

  SetFixtureStatus(ddi_fusion_uart_channel_flush(GetFusionUARTHandle()));
  EXPECT_EQ(DDI_EM_STATUS_OK, GetFixtureStatus()) << "channel flush failed\n";

  while ( test_count++ < 10 )
  {
    ASSERT_NO_FATAL_FAILURE(perform_loopback(false));
  }

  // Verify the callback count
  ASSERT_EQ(callback_count, 10);
  printf("%d callbacks received \n", callback_count);

  // Stop the cylcic thread started by ddi_em_cyclic_task_start()
  SetFixtureStatus(ddi_em_cyclic_task_stop(GetEtherCATMasterHandle()));
  // ddi_em_cyclic_task_stop returns 0 if successful
  ASSERT_EQ(GetFixtureStatus(), DDI_EM_STATUS_OK);
}

TEST_F(ddi_fusion_uart_test_fixture, DDIEM_UART_232_loopback_event_no_trigger)
{
  uint callback_count;
  int test_count = 0;
  DDIEMUtility m_ddi_em_utility;

  uart_pd_callback_args       pd_callback_args;

  pd_callback_args.em_handle = GetEtherCATMasterHandle();
  pd_callback_args.es_cfg = GetEtherCATSlaveConfigPointer();

  // Register the cyclic callback
  SetFixtureStatus(ddi_em_register_cyclic_callback(GetEtherCATMasterHandle(), m_ddi_em_utility.UART_cyclic_function, &pd_callback_args));
  EXPECT_EQ(DDI_EM_STATUS_OK, GetFixtureStatus()) << "ddi_em_register_cyclic_callback has failed\n";

  SetFixtureStatus(ddi_fusion_uart_register_event(GetFusionUARTHandle(),uart_event_callback,&callback_count));
  EXPECT_EQ(DDI_EM_STATUS_OK, GetFixtureStatus()) << "uart event register failed\n";

  // Set the threshold to a level which cannot be triggered
  SetFixtureStatus(ddi_fusion_uart_enable_threshold_event(GetFusionUARTHandle(), UART_EVENT_THRESHOLD_RISING_EDGE, 300));
  EXPECT_EQ(DDI_EM_STATUS_OK, GetFixtureStatus()) << "uart enable threshold failed\n";

  // set the EtherCAT Master State to OP mode
  SetFixtureStatus(ddi_em_set_master_state(GetEtherCATMasterHandle(), DDI_EM_STATE_OP, TEST_DEFAULT_TIMEOUT));
  EXPECT_EQ(DDI_EM_STATUS_OK, GetFixtureStatus()) << "ddi_em_set_master_state failed.\n";

  SetFixtureStatus(ddi_fusion_uart_channel_flush(GetFusionUARTHandle()));
  EXPECT_EQ(DDI_EM_STATUS_OK, GetFixtureStatus()) << "channel flush failed\n";

  printf("performing loopback\n");
  while ( test_count++ < 10 )
  {
    ASSERT_NO_FATAL_FAILURE(perform_loopback(false)); // Test loopback functionality
  }

  // Verify the callback count is set to 0
  ASSERT_EQ(0, callback_count);
  printf("%d callbacks received \n", callback_count);

  // Stop the cylcic thread started by ddi_em_cyclic_task_start()
  SetFixtureStatus(ddi_em_cyclic_task_stop(GetEtherCATMasterHandle()));
  // ddi_em_cyclic_task_stop returns 0 if successful
  ASSERT_EQ(GetFixtureStatus(), DDI_EM_STATUS_OK);
}

TEST_F(ddi_fusion_uart_test_fixture, DDIEM_UART_232_loopback_event_falling)
{
  uint callback_count;
  int test_count = 0;
  DDIEMUtility m_ddi_em_utility;

  uart_pd_callback_args       pd_callback_args;

  pd_callback_args.em_handle = GetEtherCATMasterHandle();
  pd_callback_args.es_cfg = GetEtherCATSlaveConfigPointer();

  // Register the cyclic callback
  SetFixtureStatus(ddi_em_register_cyclic_callback(GetEtherCATMasterHandle(), m_ddi_em_utility.UART_cyclic_function, &pd_callback_args));
  EXPECT_EQ(DDI_EM_STATUS_OK, GetFixtureStatus()) << "ddi_em_register_cyclic_callback has failed\n";

  SetFixtureStatus(ddi_fusion_uart_register_event(GetFusionUARTHandle(),uart_event_callback,&callback_count));
  EXPECT_EQ(DDI_EM_STATUS_OK, GetFixtureStatus()) << "uart event register failed\n";

  SetFixtureStatus(ddi_fusion_uart_enable_threshold_event(GetFusionUARTHandle(), UART_EVENT_THRESHOLD_FALLING_EDGE, 10));
  EXPECT_EQ(DDI_EM_STATUS_OK, GetFixtureStatus()) << "uart enable threshold failed\n";

  // set the EtherCAT Master State to OP mode
  SetFixtureStatus(ddi_em_set_master_state(GetEtherCATMasterHandle(), DDI_EM_STATE_OP, TEST_DEFAULT_TIMEOUT));
  EXPECT_EQ(DDI_EM_STATUS_OK, GetFixtureStatus()) << "ddi_em_set_master_state failed.\n";

  SetFixtureStatus(ddi_fusion_uart_channel_flush(GetFusionUARTHandle()));
  EXPECT_EQ(DDI_EM_STATUS_OK, GetFixtureStatus()) << "channel flush failed\n";

  while ( test_count++ < 10 )
  {
    ASSERT_NO_FATAL_FAILURE(perform_loopback(false)); // Test loopback functionality
  }

  printf("performing loopback\n");

  // Verify the callback count
  ASSERT_EQ(10, callback_count);
  printf("%d callbacks received \n", callback_count);

  // Stop the cylcic thread started by ddi_em_cyclic_task_start()
  SetFixtureStatus(ddi_em_cyclic_task_stop(GetEtherCATMasterHandle()));
  // ddi_em_cyclic_task_stop returns 0 if successful
  ASSERT_EQ(GetFixtureStatus(), DDI_EM_STATUS_OK);
}

TEST_F(ddi_fusion_uart_test_fixture, DDIEM_UART_232_loopback_all_baud_rate_test)
{
  DDIEMUtility m_ddi_em_utility;
  int test_count = 0;

  uart_pd_callback_args       pd_callback_args;

  pd_callback_args.em_handle = GetEtherCATMasterHandle();
  pd_callback_args.es_cfg = GetEtherCATSlaveConfigPointer();

  // Register the cyclic callback
  SetFixtureStatus(ddi_em_register_cyclic_callback(GetEtherCATMasterHandle(), m_ddi_em_utility.UART_cyclic_function, &pd_callback_args));
  EXPECT_EQ(DDI_EM_STATUS_OK, GetFixtureStatus()) << "ddi_em_register_cyclic_callback has failed\n";

  // set the EtherCAT Master State to OP mode
  SetFixtureStatus(ddi_em_set_master_state(GetEtherCATMasterHandle(), DDI_EM_STATE_OP, TEST_DEFAULT_TIMEOUT));
  EXPECT_EQ(DDI_EM_STATUS_OK, GetFixtureStatus()) << "ddi_em_set_master_state failed.\n";

  // cycle through the array and exercise set/get functions 
  for ( uart_baud input : baud_list )
  {
    test_count = 0;
    // set the EtherCAT Master State to PREOP mode so the baud rate can be changed
    SetFixtureStatus(ddi_em_set_master_state(GetEtherCATMasterHandle(), DDI_EM_STATE_PREOP, TEST_DEFAULT_TIMEOUT));
    EXPECT_EQ(DDI_EM_STATUS_OK, GetFixtureStatus()) << "ddi_em_set_master_state failed.\n";

    printf("\nSetting baud rate to: %d\n", input);
    SetFixtureStatus(ddi_fusion_uart_set_baud(GetFusionUARTHandle(), input));
    EXPECT_EQ(DDI_EM_STATUS_OK, GetFixtureStatus()) << "\nFaulty baud_rate input: " << input << "\n";

    // set the EtherCAT Master State to OP mode
    SetFixtureStatus(ddi_em_set_master_state(GetEtherCATMasterHandle(), DDI_EM_STATE_OP, TEST_DEFAULT_TIMEOUT));
    EXPECT_EQ(DDI_EM_STATUS_OK, GetFixtureStatus()) << "ddi_em_set_master_state failed.\n";

    SetFixtureStatus(ddi_fusion_uart_channel_flush(GetFusionUARTHandle()));
    EXPECT_EQ(DDI_EM_STATUS_OK, GetFixtureStatus()) << "channel flush failed\n";

    while ( test_count++ < 10 )
    {
      ASSERT_NO_FATAL_FAILURE(perform_loopback(false)); // Test loopback functionality
    }
  }

  // Stop the cylcic thread started by ddi_em_cyclic_task_start()
  SetFixtureStatus(ddi_em_cyclic_task_stop(GetEtherCATMasterHandle()));
  // ddi_em_cyclic_task_stop returns 0 if successful
  ASSERT_EQ(GetFixtureStatus(), DDI_EM_STATUS_OK);
}

TEST_F(ddi_fusion_uart_test_fixture, DDIEM_UART_485_loopback_single_channel)
{
  DDIEMUtility m_ddi_em_utility;
  int test_count = 0;

  uart_pd_callback_args       pd_callback_args;

  pd_callback_args.em_handle = GetEtherCATMasterHandle();
  pd_callback_args.es_cfg = GetEtherCATSlaveConfigPointer();

  // set the EtherCAT Master State to PREOP mode
  SetFixtureStatus(ddi_em_set_master_state(GetEtherCATMasterHandle(), DDI_EM_STATE_PREOP, TEST_DEFAULT_TIMEOUT));
  EXPECT_EQ(DDI_EM_STATUS_OK, GetFixtureStatus()) << "ddi_em_set_master_state failed.\n";

  printf("Setting interface to RS485\n");

  // set interface to RS-485
  SetFixtureStatus(ddi_fusion_uart_set_interface(GetFusionUARTHandle(), UART_INTERFACE_RS485));
  EXPECT_EQ(DDI_EM_STATUS_OK, GetFixtureStatus()) << "ddi_fusion_uart_set_interface has failed\n";

  // Register the cyclic callback
  SetFixtureStatus(ddi_em_register_cyclic_callback(GetEtherCATMasterHandle(), m_ddi_em_utility.UART_cyclic_function, &pd_callback_args));
  EXPECT_EQ(DDI_EM_STATUS_OK, GetFixtureStatus()) << "ddi_em_register_cyclic_callback has failed\n";

  // set the EtherCAT Master State to OP mode
  SetFixtureStatus(ddi_em_set_master_state(GetEtherCATMasterHandle(), DDI_EM_STATE_OP, TEST_DEFAULT_TIMEOUT));
  EXPECT_EQ(DDI_EM_STATUS_OK, GetFixtureStatus()) << "ddi_em_set_master_state failed.\n";

  printf("performing loopback\n");

  while ( test_count++ < 10 )
  {
    ASSERT_NO_FATAL_FAILURE(perform_loopback(false)); // Test loopback functionality
  }

  // Stop the cylcic thread started by ddi_em_cyclic_task_start()
  SetFixtureStatus(ddi_em_cyclic_task_stop(GetEtherCATMasterHandle()));
  // ddi_em_cyclic_task_stop returns 0 if successful
  ASSERT_EQ(GetFixtureStatus(), DDI_EM_STATUS_OK);
}

TEST_F(ddi_fusion_uart_test_fixture, DDIEM_UART_param_set_get_interface_test)
{
  uart_interface interface_list[]
  {
    UART_INTERFACE_RS232, 
    UART_INTERFACE_RS485, 
    UART_INTERFACE_RS485_WITH_TERMINATION_RESISTOR
  };

  // always different from the input, so we can test the validity of the set function
  uart_interface read_back = UART_INTERFACE_RS485_WITH_TERMINATION_RESISTOR;

  for ( uart_interface input : interface_list )
  {
    SetFixtureStatus(ddi_fusion_uart_set_interface(GetFusionUARTHandle(), input));
    ASSERT_EQ(GetFixtureStatus(), DDI_EM_STATUS_OK);

    SetFixtureStatus(ddi_fusion_uart_get_interface(GetFusionUARTHandle(), &read_back));
    ASSERT_EQ(GetFixtureStatus(), DDI_EM_STATUS_OK);

    ASSERT_EQ(input, read_back);
  }
}

TEST_F(ddi_fusion_uart_test_fixture, DDIEM_UART_232_loopback_all_params)
{
  bool data_7;
  // set master to PREOP mode to set all configs 
  SetFixtureStatus(ddi_em_set_master_state(GetEtherCATMasterHandle(), DDI_EM_STATE_PREOP, TEST_DEFAULT_TIMEOUT));
  ASSERT_EQ(DDI_EM_STATUS_OK, GetFixtureStatus()) << "ddi_em_set_master_state failed.\n";

  // series of nested for-loops to test every combination of configs
  for ( uart_stop_bits stop_bit : m_stop_bits )
  {
    // set stop_bits
    SetFixtureStatus(ddi_fusion_uart_set_stop_bits(GetFusionUARTHandle(), stop_bit));
    ASSERT_EQ(DDI_EM_STATUS_OK, GetFixtureStatus());

    for ( uart_parity parity : parity )
    {
      //set parity mode
      SetFixtureStatus(ddi_fusion_uart_set_parity_mode(GetFusionUARTHandle(), parity));
      ASSERT_EQ(DDI_EM_STATUS_OK, GetFixtureStatus());

      for ( uart_flow_control flow : flow_control )
      {
        //set flow control
        SetFixtureStatus(ddi_fusion_uart_set_flow_control(GetFusionUARTHandle(), flow));
        ASSERT_EQ(DDI_EM_STATUS_OK, GetFixtureStatus());

        for( uart_data_bits data_bit : m_data_bits )
        {
          //set data_bit
          SetFixtureStatus(ddi_fusion_uart_set_data_bits(GetFusionUARTHandle(), data_bit));
          ASSERT_EQ(DDI_EM_STATUS_OK, GetFixtureStatus());

          for( uart_baud baud : baud_list )
          {
            // set baud rate
            SetFixtureStatus(ddi_fusion_uart_set_baud(GetFusionUARTHandle(), baud));
            ASSERT_EQ(DDI_EM_STATUS_OK, GetFixtureStatus());

            // perform loopback
            // set the EtherCAT Master State to OP mode
            SetFixtureStatus(ddi_em_set_master_state(GetEtherCATMasterHandle(), DDI_EM_STATE_OP, TEST_DEFAULT_TIMEOUT));
            ASSERT_EQ(DDI_EM_STATUS_OK, GetFixtureStatus()) << "ddi_em_set_master_state failed.\n";

            // Do loopback for all cases EXCEPT 7-N-1, which we don't support
            if( !((UART_DATA_BITS_7 == data_bit) && (UART_PARITY_NONE == parity) && (UART_STOP_BITS_1 == stop_bit)) )
            {
              printf("testing stop_bit %d parity %d flow %d data_bit %d baud %d \n", stop_bit, parity, flow, data_bit, baud);
              for(int count = 0; count < 10; count++)
              {
                printf("    loop %d\n", count);
                data_7 = (UART_DATA_BITS_7 == data_bit);
                ASSERT_NO_FATAL_FAILURE(perform_loopback(data_7)); // Test loopback functionality
              }
            }
            else
            {
              printf("testing stop_bit %d parity %d flow %d data_bit %d baud %d -- skipped \n", stop_bit, parity, flow, data_bit, baud);
            }

            // set master back to PREOP mode to set all configs 
            SetFixtureStatus(ddi_em_set_master_state(GetEtherCATMasterHandle(), DDI_EM_STATE_PREOP, TEST_DEFAULT_TIMEOUT));
            ASSERT_EQ(DDI_EM_STATUS_OK, GetFixtureStatus()) << "ddi_em_set_master_state failed.\n";
          }
        }
      }
    }
  }
}
