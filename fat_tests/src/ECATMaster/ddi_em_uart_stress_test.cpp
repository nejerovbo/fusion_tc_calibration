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
#include <thread>
#include <sys/time.h>
#include "EnvironmentRegistry.h"
#include "DDIEMUtility.h"
#include "DDIEMEnvironment.h"
#include "ddi_em_fusion_uart_api.h"
#include "ddi_em_fusion.h"
#include "ddi_fusion_uart_test_fixture.h"

void Test_Thread(ddi_fusion_uart_handle uart_handle);
int accumulate_bytes_transferred(int tx_bytes, int rx_bytes);
#define NUM_UARTS_PER_CARD   4
#define NUM_UARTS           20
ddi_mutex_handle_t data_handle;

TEST(UART_Init_Stress_Suite, DDIEM_UART_Stress_Test)
{
  ddi_em_handle em_handle;
  ddi_es_handle es_handle;
  ddi_em_slave_config es_cfg;
  int16_t index;
  uart_channel channel;
  uint32_t flags = 0;
  ddi_fusion_uart_handle uart_handle[NUM_UARTS];
  ddi_em_result result;
  ddi_em_init_params init_params;
  thread t[NUM_UARTS];
  int count;
  struct timeval time1, time2;
  struct timezone tz;

  memset(&init_params, 0, sizeof(ddi_em_init_params));

  // EtherCAT network adapter to use
  init_params.network_adapter         = DDI_EM_NIC_2;
  // Enable direct call-in from an external thread
  init_params.enable_cyclic_thread    = DDI_EM_TRUE;
  init_params.scan_rate_us            = DDI_EM_DEFAULT_CYCLIC_RATE;
  // Use the default thread priority for the EtherCAT Cyclic Thread
  init_params.polling_thread_priority = DDI_EM_CYCLIC_THREAD_PRI_DEFAULT;

  result = ddi_em_sdk_init();
  ASSERT_EQ(DDI_EM_STATUS_OK, result) << "ddi_em_sdk_init failed: " << ddi_em_get_error_string(result) << "\n";

  //initialize master instance
  result = ddi_em_init(&init_params, &em_handle);
  ASSERT_EQ(DDI_EM_STATUS_OK, result) << "ddi_em_init failed \n";

  // configure master
  result = ddi_em_configure_master(em_handle, "config/8_uart_eni.xml");
  ASSERT_EQ(DDI_EM_STATUS_OK, result);

  // open the fusion with vendor and product ID
  result = ddi_em_open_slave_by_id(em_handle, DDI_ETHERCAT_VENDOR_ID, DDI_FUSION_PRODUCT_CODE, 0, 0, &es_handle);
  ASSERT_EQ(DDI_EM_STATUS_OK, result);

  // retrieve the information about the slave
  result = ddi_em_get_slave_config(em_handle, es_handle, &es_cfg);
  ASSERT_EQ(DDI_EM_STATUS_OK, result);

  for(int count = 0; count < NUM_UARTS; count++)
  {
    index = 0x5085 + ((count/NUM_UARTS_PER_CARD) << 4);     // 0x5085, 0x5095, 0x50a5, ...
    channel = (uart_channel)(count % NUM_UARTS_PER_CARD);   // 0, 1, 2, 3, 0, 1, ....
    result = ddi_fusion_uart_open(em_handle, es_handle, index, channel, flags, &uart_handle[count]);
    ASSERT_EQ(DDI_EM_STATUS_OK, result);
  }

  result = ddi_em_set_master_state(em_handle, DDI_EM_STATE_PREOP, TEST_DEFAULT_TIMEOUT);
  ASSERT_EQ(DDI_EM_STATUS_OK, result) << "ddi_em_set_master_state failed.\n";

//  result = ddi_em_set_master_state(em_handle, DDI_EM_STATE_OP, TEST_DEFAULT_TIMEOUT);
//  ASSERT_EQ(DDI_EM_STATUS_OK, result) << "ddi_em_set_master_state failed.\n";

  ddi_mutex_create(&data_handle);

  gettimeofday(&time1, &tz);

  for(int i = 0; i < NUM_UARTS; i++)
  {
    t[i] = std::thread(Test_Thread, uart_handle[i]);
  }

  for(int i = 0; i < NUM_UARTS; i++)
  {
    t[i].join();
  }

  gettimeofday(&time2, &tz);
  int time_diff = int(time2.tv_sec - time1.tv_sec);

  // close fusion instance
  for(count = 0; count < NUM_UARTS; count++);
  result = ddi_fusion_uart_close(uart_handle[count]);
  EXPECT_EQ(DDI_EM_STATUS_OK, result);

  // de-init the master instance
  result = ddi_em_deinit(em_handle);
  EXPECT_EQ(DDI_EM_STATUS_OK, result) << "ddi_em_deinit failed \n";

  //de-init the SDK
  result = ddi_em_sdk_deinit();
  EXPECT_EQ(DDI_EM_STATUS_OK, result) << "ddi_em_sdk_deinit failed: " << ddi_em_get_error_string(result) << "\n";

  int bytes_transferred = accumulate_bytes_transferred(0, 0);
  printf("  --Transfer Rate = %.0f bytes per second\n", (float)bytes_transferred/time_diff);
}


//-----------------------------------------------------------------------------
// Set the UART to RS232, 115,200 baud, 8-N-1
// Write-Read-Compare 256 times
//-----------------------------------------------------------------------------
void Test_Thread(ddi_fusion_uart_handle uart_handle)
{
  uint8_t rx_data[DDI_FUSION_UART_SDO_DATA_SIZE_MAX];
  uint8_t tx_data[DDI_FUSION_UART_SDO_DATA_SIZE_MAX];
  uint32_t rx_length = sizeof(rx_data);  
  ddi_em_result result;

  srand(time(NULL)); // Seed the RNG

  result = ddi_fusion_uart_set_interface(uart_handle, UART_INTERFACE_RS232);
  ASSERT_EQ(DDI_EM_STATUS_OK, result);

  result = ddi_fusion_uart_set_baud(uart_handle, UART_BAUD_115200);
  ASSERT_EQ(DDI_EM_STATUS_OK, result);

  result = ddi_fusion_uart_set_data_bits(uart_handle, UART_DATA_BITS_8);
  ASSERT_EQ(DDI_EM_STATUS_OK, result);

  result = ddi_fusion_uart_set_parity_mode(uart_handle, UART_PARITY_NONE);
  ASSERT_EQ(DDI_EM_STATUS_OK, result);

  result = ddi_fusion_uart_set_stop_bits(uart_handle, UART_STOP_BITS_1);
  ASSERT_EQ(DDI_EM_STATUS_OK, result);

  result = ddi_fusion_uart_channel_flush (uart_handle);
  ASSERT_EQ(DDI_EM_STATUS_OK, result);

  for(int i = 0; i < 256; i++)
  {
    memset(rx_data, 0, sizeof(rx_data));
    for (int count = 0; count < sizeof(tx_data); count++)
    {
      tx_data[count] = rand();
    }

    // initiate data transfer
    result = ddi_fusion_uart_tx_data(uart_handle, tx_data, sizeof(tx_data));
    EXPECT_EQ(DDI_EM_STATUS_OK, result) << "ddi_fusion_uart_tx_data failed\n";
    accumulate_bytes_transferred(sizeof(tx_data), 0);

    // Print an "iterator" so we know the test is still running
    if( (i % 16 == 0) && (0 == uart_handle) )
    {
      printf("iteration = %d\n", i);
    }

    // get the amount of receive bytes for the given UART channel, do this before read
    uint16_t bytes_avail = 0;
    while ( bytes_avail < sizeof(tx_data))
    {
      result = ddi_fusion_uart_get_rx_bytes_avail(uart_handle, &bytes_avail);
      EXPECT_EQ(DDI_EM_STATUS_OK, result) << "ddi_fusion_uart_get_rx_bytes_avail failed\n";
    }

    // bytes_avail should match the tx amount before read
    EXPECT_TRUE( bytes_avail == sizeof(tx_data) ) << "bytes avail = " << bytes_avail << "\n";

    result = ddi_fusion_uart_rx_data(uart_handle, rx_data, &rx_length);
    EXPECT_EQ(DDI_EM_STATUS_OK, result) << "ddi_fusion_uart_rx_data failed \n";
    accumulate_bytes_transferred(0, rx_length);
    ASSERT_EQ( memcmp(rx_data, tx_data, sizeof(tx_data)), 0 ) << "UART compare failed, i = " << i << ", uart handle = " << uart_handle << "\n";
  }
}


//-----------------------------------------------------------------------------
// Accumulate bytes transmitted and bytes received.
// Return the combined total.
//-----------------------------------------------------------------------------
int accumulate_bytes_transferred(int tx_bytes, int rx_bytes)
{
  static int total_tx_bytes = 0, total_rx_bytes = 0;

  ddi_mutex_lock(data_handle, 0);   // timeout = forever
  total_tx_bytes += tx_bytes;
  total_rx_bytes += rx_bytes;
  ddi_mutex_unlock(data_handle);
  return (total_tx_bytes + total_rx_bytes);
}
