/**************************************************************************
(c) Copyright 2022 Digital Dynamics Inc. Scotts Valley CA USA.
Unpublished copyright. All rights reserved. Contains proprietary and
confidential trade secrets belonging to DDI. Disclosure or release without
prior written authorization of DDI is prohibited.
**************************************************************************/
#include "CyclicData.h"
#include <string>
#include <iostream>
#include "ddi_fusion_uart_test_fixture.h"
#include "DDIEMUtility.h"
#include <sstream>
#include <inttypes.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "sys/socket.h"
#include "arpa/inet.h"
#include "ddi_em_api.h"
#include "ddi_em_fusion.h"
#include "DDIEMTestFixture.h"
#include "DDIEMEnvironment.h"
#include "EnvironmentRegistry.h"

ddi_fusion_uart_test_fixture::ddi_fusion_uart_test_fixture(void)
{  
  m_DDIEMEnvironment = (DDIEMEnvironment *)g_environment_registry.GetEnvironment(DDIEM_ENVIRONMENT_NAME);
  em_handle =       0;
  es_handle =       0;
  tx_index_base =   0;
  rx_index_base =   0;
  config_index =    m_DDIEMEnvironment->GetUARTIndex();
  info_index =      0;
  channel =         (uart_channel) m_DDIEMEnvironment->GetUARTChannel();
  is_allocated =    0;

  //sets all fields in init_params to 0
  memset(GetInitParamsPointer(), 0, sizeof(ddi_em_init_params));
  SetInitParamsDefault();
}

void ddi_fusion_uart_test_fixture::SetUp()
{
  // Initialize the ddi_em_sdk, this should be done before other function calls
  SetFixtureStatus(ddi_em_sdk_init());
  ASSERT_EQ(DDI_EM_STATUS_OK, GetFixtureStatus()) << "ddi_em_sdk_init failed: " << ddi_em_get_error_string(GetFixtureStatus()) << "\n";

  // next step is to set init_params with some default values, this step has been moved to the constructor  

  // Initialize the EtherCAT master instance                   
  SetFixtureStatus(ddi_em_init(GetInitParamsPointer(), GetEtherCATMasterHandlePointer()));  
  ASSERT_EQ(DDI_EM_STATUS_OK, GetFixtureStatus()) << "ddi_em_init failed: " << ddi_em_get_error_string(GetFixtureStatus()) << "\n";

  printf("\nddi_em_init\n");

  // Simulate a thread manager creating the thread used by the DDI ECAT Master SDK
  //int success = m_ddi_em_utility.start_thread_manager(GetEtherCATMasterHandle(), m_ddi_em_utility.GetThreadHandlePointer());
  //ASSERT_EQ(0, success) << "m_ddi_em_utility.start_thread_manager failed. \n";
  
  printf("\nstart_thread_manager\n");

  // configure master
  SetFixtureStatus(ddi_em_configure_master(GetEtherCATMasterHandle(), GetEnvironmentPointer()->GetEniFile().c_str()));
  ASSERT_EQ(DDI_EM_STATUS_OK, GetFixtureStatus());

  printf("\nddi_em_configure_master\n");

  // initialize slave handle
  SetFixtureStatus(ddi_em_open_slave_by_id(GetEtherCATMasterHandle(), DDI_ETHERCAT_VENDOR_ID, DDI_FUSION_PRODUCT_CODE, 0, 0, GetEtherCATSlaveHandlePointer()));
  ASSERT_EQ(DDI_EM_STATUS_OK, GetFixtureStatus());

  printf("\nddi_em_open_slave_by_id\n");

  // retrieve the information about the slave
  SetFixtureStatus(ddi_em_get_slave_config(em_handle, es_handle, &es_cfg));
  ASSERT_EQ(DDI_EM_STATUS_OK, GetFixtureStatus());

  // open a uart channel
  SetFixtureStatus(ddi_fusion_uart_open(GetEtherCATMasterHandle(), GetEtherCATSlaveHandle(), config_index, channel, 0, GetFusionUARTHandlePointer()));
  EXPECT_EQ(DDI_EM_STATUS_OK, GetFixtureStatus());
 
  // Set the master state to PREOP
  SetFixtureStatus(ddi_em_set_master_state(GetEtherCATMasterHandle(), DDI_EM_STATE_PREOP, TEST_DEFAULT_TIMEOUT));
  // ddi_em_set_master_state returns 0 if successful
  ASSERT_EQ(GetFixtureStatus(), DDI_EM_STATUS_OK);
}

void ddi_fusion_uart_test_fixture::TearDown()
{
  SetFixtureStatus(ddi_em_close_slave(GetEtherCATMasterHandle(), GetEtherCATSlaveHandle()));
  ASSERT_EQ(DDI_EM_STATUS_OK, GetFixtureStatus());

  // Close a uart channel
  SetFixtureStatus(ddi_fusion_uart_close(GetFusionUARTHandle()));
  EXPECT_EQ(DDI_EM_STATUS_OK, GetFixtureStatus());

  //first, deinit the master instance
  SetFixtureStatus(ddi_em_deinit(GetEtherCATMasterHandle()));
  ASSERT_EQ(DDI_EM_STATUS_OK, GetFixtureStatus());

  //second, deinit the SDK
  SetFixtureStatus(ddi_em_sdk_deinit());
  ASSERT_EQ(DDI_EM_STATUS_OK, GetFixtureStatus());
}

void ddi_fusion_uart_test_fixture::SetInitParamsDefault()
{
  // EtherCAT network adapter to use
  GetInitParamsPointer() -> network_adapter         = FindNic();
  // Enable direct call-in from an external thread
  GetInitParamsPointer() -> remote_client_enable    = DDI_EM_REMOTE_ENABLED;
  GetInitParamsPointer() -> enable_cyclic_thread    = DDI_EM_TRUE;
  GetInitParamsPointer() -> scan_rate_us            = DDI_EM_DEFAULT_CYCLIC_RATE;
  // Use the default thread priority for the EtherCAT Cyclic Thread
  GetInitParamsPointer() -> polling_thread_priority = DDI_EM_CYCLIC_THREAD_PRI_DEFAULT;
}

// Perform a loopback test
// Generate random data, send it through ECAT
// Verify the data matches
void ddi_fusion_uart_test_fixture::perform_loopback(bool data_7)
{
  uint8_t rx_data[DDI_FUSION_UART_SDO_DATA_SIZE_MAX];
  uint8_t tx_data[DDI_FUSION_UART_SDO_DATA_SIZE_MAX];
  uint32_t rx_length = sizeof(rx_data);

  memset(rx_data, 0, sizeof(rx_data));

  srand(time(NULL)); // Seed the RNG
  int count = 0;
  for (count =0; count < sizeof(tx_data); count++)
  {
    tx_data[count] = rand();
    tx_data[count] &= ~(data_7 << 7);
  }
// initiate data transfer
  SetFixtureStatus(ddi_fusion_uart_tx_data(GetFusionUARTHandle(), tx_data, sizeof(tx_data)));
  EXPECT_EQ(DDI_EM_STATUS_OK, GetFixtureStatus()) << "ddi_fusion_uart_tx_data failed\n";

  // get the amount of receive bytes for the given UART channel, do this before read
  uint16_t bytes_avail = 0;
  while ( bytes_avail < sizeof(tx_data))
  {
    SetFixtureStatus(ddi_fusion_uart_get_rx_bytes_avail(GetFusionUARTHandle(), &bytes_avail));
    EXPECT_EQ(DDI_EM_STATUS_OK, GetFixtureStatus()) << "ddi_fusion_uart_get_rx_bytes_avail failed\n";
    //printf("bytes_avail %d \n", bytes_avail);
  }

  // bytes_avail should match the tx amount before read
  EXPECT_TRUE( bytes_avail >= sizeof(tx_data) );

  SetFixtureStatus(ddi_fusion_uart_rx_data(GetFusionUARTHandle(), rx_data, &rx_length));
  EXPECT_EQ(DDI_EM_STATUS_OK, GetFixtureStatus()) << "ddi_fusion_uart_rx_data failed \n";

  ASSERT_EQ( memcmp(rx_data, tx_data, sizeof(tx_data)), 0 ) << "UART compare failed\n";
  for (int count = 0; count < sizeof(tx_data); count++ )
  {
    if ( (tx_data[count] & 0x7F) != (rx_data[count] & 0x7F))
    {
      printf("Rx_data[%03d] = 0x%04x, expected 0x%04x \n\n", count, rx_data[count], tx_data[count]);
      while(1); // We want to know about any mismatches, so stop the test
    }
  }
}

// Sets NIC according to input argument
ddi_em_interface_select ddi_fusion_uart_test_fixture::FindNic()
{
  // obtain the last character of the NIC string, it dictates which NIC we use. 1-4
  char temp = m_DDIEMEnvironment->GetNIC().back();
  switch (temp)
  {
  case '1':
    return DDI_EM_NIC_1;
    break;
  case '2':
    return DDI_EM_NIC_2;
    break;
  case '3':
    return DDI_EM_NIC_3;
    break;
  case '4':
    return DDI_EM_NIC_4;
    break;
  default:
    return DDI_EM_NIC_2;
    break;
  } 
}
