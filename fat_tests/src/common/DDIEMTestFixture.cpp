/**************************************************************************
(c) Copyright 2021 Digital Dynamics Inc. Scotts Valley CA USA.
Unpublished copyright. All rights reserved. Contains proprietary and
confidential trade secrets belonging to DDI. Disclosure or release without
prior written authorization of DDI is prohibited.
**************************************************************************/
#include "CyclicData.h"
#include <string>
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

void DDIEMTestFixture::SetUp(void)
{
  m_DDIEMEnvironment = (DDIEMEnvironment *)g_environment_registry.GetEnvironment(DDIEM_ENVIRONMENT_NAME);

  memset(GetInitParamsPointer(), 0, sizeof(ddi_em_init_params));

  // Initialize the ddi_em_sdk, this should be done before other function calls
  SetFixtureStatus(ddi_em_sdk_init());
  ASSERT_EQ(DDI_EM_STATUS_OK, GetFixtureStatus()) << "ddi_em_sdk_init failed: " << ddi_em_get_error_string(GetFixtureStatus()) << "\n";

  SetInitParams();
  
  // Initialize the EtherCAT master instance                   
  SetFixtureStatus(ddi_em_init(GetInitParamsPointer(), GetEtherCATMasterHandlePointer()));
  
  ASSERT_EQ(DDI_EM_STATUS_OK, GetFixtureStatus()) << "ddi_em_init failed: " << ddi_em_get_error_string(GetFixtureStatus()) << "\n";
  // Simulate a thread manager creating the thread used by the DDI ECAT Master SDK
  int success = m_ddi_em_utility.start_thread_manager(GetEtherCATMasterHandle(), m_ddi_em_utility.GetThreadHandlePointer());
  ASSERT_EQ(0, success) << "m_ddi_em_utility.start_thread_manager failed. \n";
}

void DDIEMTestFixture::TearDown()
{
  printf("Tearing down\n");

  SetFixtureStatus(ddi_em_get_master_stats(GetEtherCATMasterHandle(), GetMasterStatsPointer()));
  EXPECT_EQ(GetFixtureStatus(), DDI_EM_STATUS_OK ) << "DDI Master Stats Failed: Result = &d\n", GetFixtureStatus();

  if((master_stats.cur_consecutive_err_frame_count != 0 ) && 
    ( master_stats.max_consecutive_err_frame_count != 0 ) &&
    ( master_stats.cyclic_err_frame_count != 0))
  {
    printf("**** Stats for master %d \n", GetEtherCATMasterHandle());
    printf("master_stats.cur_consecutive_lost_frame_count 0x%04x\n", master_stats.cur_consecutive_err_frame_count);
    printf("master_stats.max_consecutive_lost_frame_count 0x%04x\n", master_stats.max_consecutive_err_frame_count);
    printf("master_stats.cyclic_error_frame_count 0x%04x\n", master_stats.cyclic_err_frame_count);
    printf("master_stats.cyclic_frames_with_no_errors %" PRIu64 "\n", master_stats.cyclic_frames_with_no_errors);
    printf("master_stats.max_cyclic_timestamp_diff_ns %d\n", master_stats.max_cyclic_timestamp_diff_ns);
    printf("master_stats.min_cyclic_timestamp_diff_ns %d\n", master_stats.min_cyclic_timestamp_diff_ns);
    printf("master_stats.average_cyclic_timestamp_diff_ns %d\n", master_stats.average_cyclic_timestamp_diff_ns);
  }

  ddi_em_cyclic_task_stop(GetEtherCATMasterHandle());

  //first, deinit the master instance
  SetFixtureStatus(ddi_em_deinit(GetEtherCATMasterHandle()));
  ASSERT_EQ(DDI_EM_STATUS_OK, GetFixtureStatus());

  //second, deinit the SDK
  SetFixtureStatus(ddi_em_sdk_deinit());
  ASSERT_EQ(DDI_EM_STATUS_OK, GetFixtureStatus());
}


// Sets NIC according to input argument
ddi_em_interface_select DDIEMTestFixture::FindNic()
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

/* 
  init_params default set up includes the following:
    network_adapter = FindNic();
    remote_client_enable = DDI_EM_REMOTE_ENABLED;
    enable_cyclic_thread = DDI_EM_FALSE;
    scan_rate_us = DDI_EM_DEFAULT_CYCLIC_RATE;
    polling_thread_priority = DDI_EM_CYCLIC_THREAD_PRI_DEFAULT; */
void DDIEMTestFixture::SetInitParams()
{
  // EtherCAT network adapter to use
  GetInitParamsPointer() -> network_adapter         = FindNic();
  // Enable direct call-in from an external thread
  GetInitParamsPointer() -> remote_client_enable    = DDI_EM_REMOTE_ENABLED;
  GetInitParamsPointer() -> enable_cyclic_thread    = DDI_EM_FALSE;
  GetInitParamsPointer() -> scan_rate_us            = DDI_EM_DEFAULT_CYCLIC_RATE;
  // Use the default thread priority for the EtherCAT Cyclic Thread
  GetInitParamsPointer() -> polling_thread_priority = DDI_EM_CYCLIC_THREAD_PRI_DEFAULT;  
  if(GetEnvironmentPointer()->IsPartialMode())
  {
    // This allows for some slaves on the network to be offline
    GetInitParamsPointer() -> network_control_flags   = DDI_EM_NETWORK_MASTER_STATE_CHECK_DISABLE;
  }
  else{
    // This means that all slaves on the network must be online 
    GetInitParamsPointer() -> network_control_flags   = DDI_EM_NETWORK_MASTER_STATE_CHECK_ENABLE;
  }
}

ddi_em_result DDIEMTestFixture::configure()
{
  // Configure the master with an ENI file
  SetFixtureStatus(ddi_em_configure_master(GetEtherCATMasterHandle(), GetEnvironmentPointer()->GetEniFile().c_str()));
  // ddi_em_configure_master returns 0 if successful
  // ASSERT_EQ(GetFixtureStatus(), DDI_EM_STATUS_OK)<< "ddi_em_configure_master failed: " << ddi_em_get_error_string(GetFixtureStatus()) << "\n";
  return GetFixtureStatus();
}
