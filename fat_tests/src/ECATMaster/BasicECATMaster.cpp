/**************************************************************************
(c) Copyright 2021 Digital Dynamics Inc. Scotts Valley CA USA.
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

/*! @var DDI_EM_TEST_CYCLIC_RATE
  @brief Testing cyclic rate in microseconds
*/
#define DDI_EM_TEST_CYCLIC_RATE_FAST       1000

/*! @var DDI_EM_TEST_CYCLIC_RATE
  @brief Testing cyclic rate in microseconds
*/
#define DDI_EM_TEST_CYCLIC_RATE_SLOW       5000

TEST(BasicInitializationTestSuite, DDIEM_Basic_SDK_InitDeinit)
{
  DDIEMEnvironment* environment = (DDIEMEnvironment *)g_environment_registry.GetEnvironment(DDIEM_ENVIRONMENT_NAME);
  ASSERT_NE(nullptr, environment);

  // DDI EtherCAT result type
  ddi_em_result result;

  // Initialize the ddi_em_sdk, this should be done before other function calls
  result = ddi_em_sdk_init();
  ASSERT_EQ(DDI_EM_STATUS_OK, result) << "ddi_em_sdk_init failed: " << ddi_em_get_error_string(result) << "\n";

  result = ddi_em_sdk_deinit();
  ASSERT_EQ(DDI_EM_STATUS_OK, result) << "ddi_em_sdk_deinit failed: " << ddi_em_get_error_string(result) << "\n";
}

TEST(BasicInitializationTestSuite, DDIEM_Basic_SDK_InitDeinit_Multi)
{
  DDIEMEnvironment* environment = (DDIEMEnvironment *)g_environment_registry.GetEnvironment(DDIEM_ENVIRONMENT_NAME);
  ASSERT_NE(nullptr, environment);

  // DDI EtherCAT result type
  ddi_em_result result;

  // Initialize the ddi_em_sdk, this should be done before other function calls
  result = ddi_em_sdk_init();
  ASSERT_EQ(DDI_EM_STATUS_OK, result) << "ddi_em_sdk_init failed: " << ddi_em_get_error_string(result) << "\n";

  result = ddi_em_sdk_deinit();
  ASSERT_EQ(DDI_EM_STATUS_OK, result) << "ddi_em_sdk_deinit failed: " << ddi_em_get_error_string(result) << "\n";

   // Initialize the ddi_em_sdk, 2nd call
  result = ddi_em_sdk_init();
  ASSERT_EQ(DDI_EM_STATUS_OK, result) << "ddi_em_sdk_init failed: " << ddi_em_get_error_string(result) << "\n";

  result = ddi_em_sdk_deinit();
  ASSERT_EQ(DDI_EM_STATUS_OK, result) << "ddi_em_sdk_deinit failed: " << ddi_em_get_error_string(result) << "\n";
}

TEST(BasicInitializationTestSuite, DDIEM_Basic_InitDeinitMaster)
{
  DDIEMEnvironment* environment = (DDIEMEnvironment *)g_environment_registry.GetEnvironment(DDIEM_ENVIRONMENT_NAME);
  ASSERT_NE(nullptr, environment);

  //etherCAT master handle, unint_32
  ddi_em_handle em_handle;
  // DDI EtherCAT result type
  ddi_em_result result;
  // DDI EtherCAT Master Initialization Parameters
  ddi_em_init_params init_params;
  
  memset(&init_params, 0, sizeof(ddi_em_init_params));

  // EtherCAT network adapter to use
  init_params.network_adapter         = DDI_EM_NIC_4;
  // Enable direct call-in from an external thread
  init_params.enable_cyclic_thread    = DDI_EM_TRUE;
  init_params.scan_rate_us            = DDI_EM_DEFAULT_CYCLIC_RATE;
  // Use the default thread priority for the EtherCAT Cyclic Thread
  init_params.polling_thread_priority = DDI_EM_CYCLIC_THREAD_PRI_DEFAULT;

  // Initialize the ddi_em_sdk, this should be done before other function calls
  result = ddi_em_sdk_init();
  ASSERT_EQ(DDI_EM_STATUS_OK, result) << "ddi_em_sdk_init failed: " << ddi_em_get_error_string(result) << "\n";
  
  //initialize master instance
  result = ddi_em_init(&init_params, &em_handle);
  ASSERT_EQ(DDI_EM_STATUS_OK, result) << "ddi_em_init failed \n";

  //de-init the master instance
  result = ddi_em_deinit(em_handle);
  ASSERT_EQ(DDI_EM_STATUS_OK, result) << "ddi_em_deinit failed \n";
}

TEST(BasicInitializationTestSuite, DDIEM_Basic_InitDeinitMaster_Multi)
{
  DDIEMEnvironment* environment = (DDIEMEnvironment *)g_environment_registry.GetEnvironment(DDIEM_ENVIRONMENT_NAME);
  ASSERT_NE(nullptr, environment);

  //etherCAT master handle, unint_32
  ddi_em_handle em_handle;
  // DDI EtherCAT result type
  ddi_em_result result;
  // DDI EtherCAT Master Initialization Parameters
  ddi_em_init_params init_params;
  
  memset(&init_params, 0, sizeof(ddi_em_init_params));

  // EtherCAT network adapter to use
  init_params.network_adapter         = DDI_EM_NIC_4;
  // Enable direct call-in from an external thread
  init_params.enable_cyclic_thread    = DDI_EM_TRUE;
  init_params.scan_rate_us            = DDI_EM_DEFAULT_CYCLIC_RATE;
  // Use the default thread priority for the EtherCAT Cyclic Thread
  init_params.polling_thread_priority = DDI_EM_CYCLIC_THREAD_PRI_DEFAULT;

  // init round 1
  result = ddi_em_sdk_init();
  ASSERT_EQ(DDI_EM_STATUS_OK, result) << "ddi_em_sdk_init failed: " << ddi_em_get_error_string(result) << "\n";
  
  //initialize master instance
  result = ddi_em_init(&init_params, &em_handle);
  ASSERT_EQ(DDI_EM_STATUS_OK, result) << "ddi_em_init failed \n";

  //de-init the master instance
  result = ddi_em_deinit(em_handle);
  ASSERT_EQ(DDI_EM_STATUS_OK, result) << "ddi_em_deinit failed \n";

  //de-init the SDK
  result = ddi_em_sdk_deinit();
  ASSERT_EQ(DDI_EM_STATUS_OK, result) << "ddi_em_sdk_deinit failed: " << ddi_em_get_error_string(result) << "\n";

  // init round 2
  result = ddi_em_sdk_init();
  ASSERT_EQ(DDI_EM_STATUS_OK, result) << "ddi_em_sdk_init failed: " << ddi_em_get_error_string(result) << "\n";
  
  //initialize master instance
  result = ddi_em_init(&init_params, &em_handle);
  ASSERT_EQ(DDI_EM_STATUS_OK, result) << "ddi_em_init failed \n";

  //de-init the master instance
  result = ddi_em_deinit(em_handle);
  ASSERT_EQ(DDI_EM_STATUS_OK, result) << "ddi_em_deinit failed \n";

  //de-init the SDK
  result = ddi_em_sdk_deinit();
  ASSERT_EQ(DDI_EM_STATUS_OK, result) << "ddi_em_sdk_deinit failed: " << ddi_em_get_error_string(result) << "\n";
}

TEST(BasicInitializationTestSuite, DDIEM_Basic_InitDeinitMasterNegative)
{
  DDIEMEnvironment* environment = (DDIEMEnvironment *)g_environment_registry.GetEnvironment(DDIEM_ENVIRONMENT_NAME);
  ASSERT_NE(nullptr, environment);

  //etherCAT master handle, unint_32
  ddi_em_handle em_handle;
  // DDI EtherCAT result type
  ddi_em_result result;
  // DDI EtherCAT Master Initialization Parameters
  ddi_em_init_params init_params;
  
  memset(&init_params, 0, sizeof(ddi_em_init_params));

  // EtherCAT network adapter to use
  init_params.network_adapter         = DDI_EM_NIC_4;
  // Enable direct call-in from an external thread
  init_params.enable_cyclic_thread    = DDI_EM_TRUE;
  init_params.scan_rate_us            = DDI_EM_DEFAULT_CYCLIC_RATE;
  // Use the default thread priority for the EtherCAT Cyclic Thread
  init_params.polling_thread_priority = DDI_EM_CYCLIC_THREAD_PRI_DEFAULT;

  // Initialize the ddi_em_sdk, this should be done before other function calls
  result = ddi_em_sdk_init();
  ASSERT_EQ(DDI_EM_STATUS_OK, result) << "ddi_em_sdk_init failed: " << ddi_em_get_error_string(result) << "\n";
  
  //initialize master instance
  result = ddi_em_init(&init_params, &em_handle);
  ASSERT_EQ(DDI_EM_STATUS_OK, result) << "ddi_em_init failed \n";

  // de-init the master instance
  result = ddi_em_deinit(-1);
  ASSERT_NE(DDI_EM_STATUS_OK, result) << "ddi_em_deinit did not fail \n";

  // de-init the master instance
  result = ddi_em_deinit(em_handle);
  ASSERT_EQ(DDI_EM_STATUS_OK, result) << "ddi_em_deinit failed \n";

  result = ddi_em_sdk_deinit();
  ASSERT_EQ(DDI_EM_STATUS_OK, result) << "ddi_em_sdk_deinit failed: " << ddi_em_get_error_string(result) << "\n";

}

TEST(BasicInitializationTestSuite, DDIEM_Basic_InitDeinitMaster_Remote)
{
  DDIEMEnvironment* environment = (DDIEMEnvironment *)g_environment_registry.GetEnvironment(DDIEM_ENVIRONMENT_NAME);
  ASSERT_NE(nullptr, environment);

  //etherCAT master handle, unint_32
  ddi_em_handle em_handle;
  // DDI EtherCAT result type
  ddi_em_result result;
  // DDI EtherCAT Master Initialization Parameters
  ddi_em_init_params init_params;
  
  memset(&init_params, 0, sizeof(ddi_em_init_params));

  // EtherCAT network adapter to use
  init_params.network_adapter         = DDI_EM_NIC_4;
  // Enable direct call-in from an external thread
  init_params.enable_cyclic_thread    = DDI_EM_TRUE;
  init_params.scan_rate_us            = DDI_EM_DEFAULT_CYCLIC_RATE;
  // Use the default thread priority for the EtherCAT Cyclic Thread
  init_params.polling_thread_priority = DDI_EM_CYCLIC_THREAD_PRI_DEFAULT;
  
   // Initialize the ddi_em_sdk, this should be done before other function calls
   result = ddi_em_sdk_init();
   ASSERT_EQ(DDI_EM_STATUS_OK, result) << "ddi_em_sdk_init failed: " << ddi_em_get_error_string(result) << "\n";

   //initialize master instance
   result = ddi_em_init(&init_params, &em_handle);
   ASSERT_EQ(DDI_EM_STATUS_OK, result) << "ddi_em_init failed: " << ddi_em_get_error_string(result) << "\n";

   ddi_em_remote_access_init_params remote_init_params;
   ddi_ra_handle remote_access_handle;
   remote_init_params.remote_port = 6000;

   //remote_init_params.remote_enable_set_pd = 0;

   result = ddi_em_remote_access_init(&remote_init_params, &remote_access_handle);
   ASSERT_EQ(DDI_EM_STATUS_OK, result) << "ddi_em_remote_access_init failed \n";
   sleep(5);
   result = ddi_em_remote_access_deinit(em_handle);
   ASSERT_EQ(DDI_EM_STATUS_OK, result) << "ddi_em_remote_access_deinit failed \n";

   //de-init the master instance
   result = ddi_em_deinit(em_handle);
   ASSERT_EQ(DDI_EM_STATUS_OK, result) << "ddi_em_deinit failed \n";
 }

TEST(BasicInitializationTestSuite, DDIEM_Basic_InitDeinitMaster_Remote_Negative)
{
  DDIEMEnvironment* environment = (DDIEMEnvironment *)g_environment_registry.GetEnvironment(DDIEM_ENVIRONMENT_NAME);
  ASSERT_NE(nullptr, environment);

  //etherCAT master handle, unint_32
  ddi_em_handle em_handle;
  // DDI EtherCAT result type
  ddi_em_result result;
  // DDI EtherCAT Master Initialization Parameters
  ddi_em_init_params init_params;
  
  memset(&init_params, 0, sizeof(ddi_em_init_params));

  // EtherCAT network adapter to use
  init_params.network_adapter         = DDI_EM_NIC_4;
  // Enable direct call-in from an external thread
  init_params.remote_client_enable    = DDI_EM_REMOTE_ENABLED;
  init_params.enable_cyclic_thread    = DDI_EM_TRUE;
  init_params.scan_rate_us            = DDI_EM_DEFAULT_CYCLIC_RATE;
  // Use the default thread priority for the EtherCAT Cyclic Thread
  init_params.polling_thread_priority = DDI_EM_CYCLIC_THREAD_PRI_DEFAULT;
  
  // Initialize the ddi_em_sdk, this should be done before other function calls
  result = ddi_em_sdk_init();
  ASSERT_EQ(DDI_EM_STATUS_OK, result) << "ddi_em_sdk_init failed: " << ddi_em_get_error_string(result) << "\n";

  //initialize master instance
  result = ddi_em_init(&init_params, &em_handle);
  ASSERT_EQ(DDI_EM_STATUS_OK, result) << "ddi_em_init failed \n";

  ddi_em_remote_access_init_params remote_init_params;
  ddi_ra_handle remote_access_handle;
  remote_init_params.remote_port = 6000;

  //remote_init_params.remote_enable_set_pd = 0;

  result = ddi_em_remote_access_init(&remote_init_params, &remote_access_handle);
  ASSERT_EQ(DDI_EM_STATUS_OK, result) << "ddi_em_remote_access_init failed \n";
  sleep(5);
  result = ddi_em_remote_access_deinit(-1);
  ASSERT_NE(DDI_EM_STATUS_OK, result) << "ddi_em_remote_access_deinit failed \n";

  //de-init the master instance
  result = ddi_em_deinit(em_handle);
  ASSERT_EQ(DDI_EM_STATUS_OK, result) << "ddi_em_deinit failed \n";
}

TEST(BasicInitializationTestSuite, DDIEM_Basic_InitDeinit_Enable_Cyclic_Thread)
{
  DDIEMEnvironment* environment = (DDIEMEnvironment *)g_environment_registry.GetEnvironment(DDIEM_ENVIRONMENT_NAME);
  ASSERT_NE(nullptr, environment);

  //etherCAT master handle, unint_32
  ddi_em_handle em_handle;
  // DDI EtherCAT result type
  ddi_em_result result;
  // DDI EtherCAT Master Initialization Parameters
  ddi_em_init_params init_params;
  
  memset(&init_params, 0, sizeof(ddi_em_init_params));

  // EtherCAT network adapter to use
  init_params.network_adapter         = DDI_EM_NIC_4;
  // Enable direct call-in from an external thread
  init_params.enable_cyclic_thread    = DDI_EM_TRUE;
  init_params.scan_rate_us            = DDI_EM_DEFAULT_CYCLIC_RATE;
  // Use the default thread priority for the EtherCAT Cyclic Thread
  init_params.polling_thread_priority = DDI_EM_CYCLIC_THREAD_PRI_DEFAULT;

  // Initialize the ddi_em_sdk, this should be done before other function calls
  result = ddi_em_sdk_init();
  ASSERT_EQ(DDI_EM_STATUS_OK, result) << "ddi_em_sdk_init failed: " << ddi_em_get_error_string(result) << "\n";
  
  //initialize master instance
  result = ddi_em_init(&init_params, &em_handle);
  ASSERT_EQ(DDI_EM_STATUS_OK, result) << "ddi_em_init failed \n";

  //de-init the master instance
  result = ddi_em_deinit(em_handle);
  ASSERT_EQ(DDI_EM_STATUS_OK, result) << "ddi_em_deinit failed \n";

  //deinit the SDK
  result = ddi_em_sdk_deinit();
  ASSERT_EQ(DDI_EM_STATUS_OK, result) << "ddi_em_sdk_deinit failed: " << ddi_em_get_error_string(result) << "\n";
}

//This requires Shaun's attention
TEST(BasicInitializationTestSuite, Disabled_DDIEM_Basic_Cyclic_Task_Start_Test)
{
  DDIEMEnvironment* environment = (DDIEMEnvironment *)g_environment_registry.GetEnvironment(DDIEM_ENVIRONMENT_NAME);
  ASSERT_NE(nullptr, environment);

  //etherCAT master handle, unint_32
  ddi_em_handle em_handle;
  // DDI EtherCAT result type
  ddi_em_result result;
  // DDI EtherCAT Master Initialization Parameters
  ddi_em_init_params init_params;
  
  memset(&init_params, 0, sizeof(ddi_em_init_params));

  // EtherCAT network adapter to use
  init_params.network_adapter         = DDI_EM_NIC_4;
  // Enable direct call-in from an external thread
  init_params.enable_cyclic_thread    = DDI_EM_TRUE;
  init_params.scan_rate_us            = DDI_EM_DEFAULT_CYCLIC_RATE;
  // Use the default thread priority for the EtherCAT Cyclic Thread
  init_params.polling_thread_priority = DDI_EM_CYCLIC_THREAD_PRI_DEFAULT;

  // Initialize the ddi_em_sdk, this should be done before other function calls
  result = ddi_em_sdk_init();
  ASSERT_EQ(DDI_EM_STATUS_OK, result) << "ddi_em_sdk_init failed: " << ddi_em_get_error_string(result) << "\n";
  
  //initialize master instance
  result = ddi_em_init(&init_params, &em_handle);
  ASSERT_EQ(DDI_EM_STATUS_OK, result) << "ddi_em_init failed \n";

  // ddi_em_cyclic_task_start()
  result = ddi_em_cyclic_task_start(em_handle);
  ASSERT_EQ(DDI_EM_STATUS_OK, result) << "ddi_em_cyclic_task_start failed \n";

  //ddi_em_cyclic_task_stop()
  result = ddi_em_cyclic_task_stop(em_handle);
  ASSERT_EQ(DDI_EM_STATUS_OK, result) << "ddi_em_cyclic_task_stop failed \n";

  //de-init the master instance
  result = ddi_em_deinit(em_handle);
  ASSERT_EQ(DDI_EM_STATUS_OK, result) << "ddi_em_deinit failed \n";
}

//********************          DDIEMTestFixture          ***************************************

TEST_F(DDIEMTestFixture, DDIEM_Basic_Master_Handle_Validity_Test)
{
  ASSERT_EQ(DDI_EM_STATUS_OK, GetFixtureStatus());
  ASSERT_TRUE((GetEtherCATMasterHandle() >= 0 && GetEtherCATMasterHandle() <= 3));
}

TEST_F(DDIEMTestFixture, DDIEM_Basic_Slave_Handle_Validity_Test)
{
  ASSERT_EQ(DDI_EM_STATUS_OK, GetFixtureStatus());
  ASSERT_TRUE((GetEtherCATSlaveHandle() >= 0 && GetEtherCATSlaveHandle() <= 3));
}

// create a test to bring the whole thing to op mode, regular start.
TEST_F(DDIEMTestFixture, DDIEM_Basic_BasicOPModeTest)
{
  DDIEMUtility m_ddi_em_utility;

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
  SetFixtureStatus(ddi_em_register_cyclic_callback(GetEtherCATMasterHandle(), process_data_callback, &pd_callback_args));
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

  // Stop the cylcic thread started by ddi_em_cyclic_task_start() 
  SetFixtureStatus(ddi_em_cyclic_task_stop(GetEtherCATMasterHandle()));
  // ddi_em_cyclic_task_stop returns 0 if successful
  ASSERT_EQ(GetFixtureStatus(), DDI_EM_STATUS_OK);
}

// tests ddi_em_get_slave_config() with invalid es_handle
TEST_F(DDIEMTestFixture, DDIEM_Basic_Get_Slave_Config_Test_Negative)
{
  // configure ddi_em_configure_master
  SetFixtureStatus(configure());
  ASSERT_EQ(GetFixtureStatus(), DDI_EM_STATUS_OK);
  
  // Open the Fusion with vendor and product ID
  SetFixtureStatus(ddi_em_open_slave_by_id(GetEtherCATMasterHandle(), DDI_ETHERCAT_VENDOR_ID, DDI_FUSION_PRODUCT_CODE, 0, 0, GetEtherCATSlaveHandlePointer()));

  ASSERT_EQ(GetFixtureStatus(), DDI_EM_STATUS_OK);

  // Retrieve the Fusion process data information
  SetFixtureStatus(ddi_em_get_slave_config(GetEtherCATMasterHandle(), -1, GetEtherCATSlaveConfigPointer()));
  // ddi_em_get_slave_config returns DDI_EM_STATUS_OK if successful
  ASSERT_NE(GetFixtureStatus(), DDI_EM_STATUS_OK);
}

TEST_F(DDIEMTestFixture, DDIEM_Basic_Open_Slave_By_Id_Negative_Test)
{
  // configure thread_manager + ddi_em_configure_master
  SetFixtureStatus(configure());
  ASSERT_EQ(GetFixtureStatus(), DDI_EM_STATUS_OK);
  
  // Open with invalid em_handle
  SetFixtureStatus(ddi_em_open_slave_by_id(GetEtherCATMasterHandle() + 5, DDI_ETHERCAT_VENDOR_ID, DDI_FUSION_PRODUCT_CODE, 0, 0, GetEtherCATSlaveHandlePointer()));
  ASSERT_EQ(DDI_EM_SCAN_NO_SLV_FOUND, GetFixtureStatus());

  // Open with invalid vendor ID
  SetFixtureStatus(ddi_em_open_slave_by_id(GetEtherCATMasterHandle(), DDI_ETHERCAT_VENDOR_ID + 1, DDI_FUSION_PRODUCT_CODE, 0, 0, GetEtherCATSlaveHandlePointer()));
  ASSERT_EQ(DDI_EM_SCAN_NO_SLV_FOUND, GetFixtureStatus());
}

TEST_F(DDIEMTestFixture, DDIEM_Basic_Close_Slave_Test)
{
  // configure thread_manager + ddi_em_configure_master
  SetFixtureStatus(configure());
  ASSERT_EQ(GetFixtureStatus(), DDI_EM_STATUS_OK);
  
  // Open first
  SetFixtureStatus(ddi_em_open_slave_by_id(GetEtherCATMasterHandle(), DDI_ETHERCAT_VENDOR_ID, DDI_FUSION_PRODUCT_CODE, 0, 0, GetEtherCATSlaveHandlePointer()));
  ASSERT_EQ(DDI_EM_STATUS_OK, GetFixtureStatus());

  // Close slave
  SetFixtureStatus(ddi_em_close_slave(GetEtherCATMasterHandle(), GetEtherCATSlaveHandle()));
  ASSERT_EQ(DDI_EM_STATUS_OK, GetFixtureStatus());
}

// Tests ddi_em_close_slave with an invalid es_handle
TEST_F(DDIEMTestFixture, DDIEM_Basic_Close_Slave_Test_Negative_esHandle)
{
  // configure thread_manager + ddi_em_configure_master
  SetFixtureStatus(configure());
  ASSERT_EQ(GetFixtureStatus(), DDI_EM_STATUS_OK);
  
  // Open first
  SetFixtureStatus(ddi_em_open_slave_by_id(GetEtherCATMasterHandle(), DDI_ETHERCAT_VENDOR_ID, DDI_FUSION_PRODUCT_CODE, 0, 0, GetEtherCATSlaveHandlePointer()));
  ASSERT_EQ(DDI_EM_STATUS_OK, GetFixtureStatus());

  // Open with invalid vendor ID
  SetFixtureStatus(ddi_em_close_slave(GetEtherCATMasterHandle(), -1));
  ASSERT_NE(DDI_EM_STATUS_OK, GetFixtureStatus());
}

// Tests ddi_em_close_slave with an invalid es_handle
TEST_F(DDIEMTestFixture, DDIEM_Basic_Close_Slave_Test_Negative_emHandle)
{
  // configure thread_manager + ddi_em_configure_master
  SetFixtureStatus(configure());
  ASSERT_EQ(GetFixtureStatus(), DDI_EM_STATUS_OK);
  
  // Open first
  SetFixtureStatus(ddi_em_open_slave_by_id(GetEtherCATMasterHandle(), DDI_ETHERCAT_VENDOR_ID, DDI_FUSION_PRODUCT_CODE, 0, 0, GetEtherCATSlaveHandlePointer()));
  ASSERT_EQ(DDI_EM_STATUS_OK, GetFixtureStatus());

  // Open with invalid vendor ID
  SetFixtureStatus(ddi_em_close_slave(-1, GetEtherCATSlaveHandle()));
  ASSERT_NE(DDI_EM_STATUS_OK, GetFixtureStatus());
}

// Tests ddi_em_configure_master with invalid master handle
TEST_F(DDIEMTestFixture, DDIEM_Basic_Configure_Master_Test_Negative)
{
  SetFixtureStatus(ddi_em_configure_master(GetEtherCATMasterHandle() + 4, GetEnvironmentPointer()->GetEniFile().c_str()));
  ASSERT_NE(GetFixtureStatus(), DDI_EM_STATUS_OK);
}

// tests ddi_em_open_slave_by_position function
TEST_F(DDIEMTestFixture, DDIEM_Basic_Open_Slave_Position)
{
  DDIEMUtility m_ddi_em_utility;

  // Process data callback arguments
  pd_callback_args pd_callback_args;

  // configure thread_manager + ddi_em_configure_master
  SetFixtureStatus(configure());
  ASSERT_EQ(GetFixtureStatus(), DDI_EM_STATUS_OK);
  
  SetFixtureStatus(ddi_em_open_slave_by_position(GetEtherCATMasterHandle(), 0, GetEtherCATSlaveHandlePointer()));

  // ddi_em_open_slave_by_position returns 0 if successful
  ASSERT_EQ(GetFixtureStatus(), DDI_EM_STATUS_OK);
}

// tests ddi_em_open_slave_by_position function, this is a negative test
TEST_F(DDIEMTestFixture, DDIEM_Basic_Open_Slave_Position_Negative)
{
  DDIEMUtility m_ddi_em_utility;

  // Process data callback arguments
  pd_callback_args pd_callback_args;

  // configure thread_manager + ddi_em_configure_master
  SetFixtureStatus(configure());
  ASSERT_EQ(GetFixtureStatus(), DDI_EM_STATUS_OK);
  
  SetFixtureStatus(ddi_em_open_slave_by_position(GetEtherCATMasterHandle(), -1, GetEtherCATSlaveHandlePointer()));

  ASSERT_NE(GetFixtureStatus(), DDI_EM_STATUS_OK);
}

// tests ddi_em_open_by_station_address function, station address is hard coded in(FYI)
TEST_F(DDIEMTestFixture, DDIEM_Basic_Open_Station_Address_Test)
{
  DDIEMUtility m_ddi_em_utility;

  // Process data callback arguments
  pd_callback_args pd_callback_args;

  // configure thread_manager + ddi_em_configure_master
  SetFixtureStatus(configure());
  ASSERT_EQ(GetFixtureStatus(), DDI_EM_STATUS_OK);
  
  //  open by station address test
  SetFixtureStatus(ddi_em_open_by_station_address(GetEtherCATMasterHandle(), 1001, GetEtherCATSlaveHandlePointer()));
  ASSERT_EQ(GetFixtureStatus(), DDI_EM_STATUS_OK);
}

// tests ddi_em_open_by_station_address function, this is a negative test
TEST_F(DDIEMTestFixture, DDIEM_Basic_Open_Station_Address_Test_Negative)
{
  DDIEMUtility m_ddi_em_utility;

  // Process data callback arguments
  pd_callback_args pd_callback_args;

  // configure thread_manager + ddi_em_configure_master
  SetFixtureStatus(configure());
  ASSERT_EQ(GetFixtureStatus(), DDI_EM_STATUS_OK);
  
  // open by station address test
  SetFixtureStatus(ddi_em_open_by_station_address(GetEtherCATMasterHandle(), -1, GetEtherCATSlaveHandlePointer()));
  ASSERT_NE(GetFixtureStatus(), DDI_EM_STATUS_OK);
}

// test to check if init_params is being accessed correctly
TEST_F(DDIEMTestFixture, DDIEM_Basic_InitParamsTest)
{
  //sets the values in init_params to 0 via a pointer
  memset(GetInitParamsPointer(), 0, sizeof(ddi_em_init_params));
  
  //set a limited amount of params to a designated default value
  SetInitParams();

  //ensure init_params' values reflect the changes
  ASSERT_NE(0, GetInitParamsPointer()->network_adapter);
  ASSERT_EQ(DDI_EM_REMOTE_ENABLED, GetInitParamsPointer()->remote_client_enable);
  ASSERT_EQ(DDI_EM_FALSE, GetInitParamsPointer()->enable_cyclic_thread);
  ASSERT_EQ(DDI_EM_DEFAULT_CYCLIC_RATE, GetInitParamsPointer()->scan_rate_us);
  ASSERT_EQ(DDI_EM_CYCLIC_THREAD_PRI_DEFAULT, GetInitParamsPointer()->polling_thread_priority);
}

TEST_F(DDIEMTestFixture, DDIEM_Basic_Get_Version_Test)
{
  ASSERT_STREQ(DDI_EM_VERSION, ddi_em_get_version());
}

// Tests ddi_em_set_cycle_rate function with a fast rate, this also contains a test for ddi_em_get_master_stats
TEST_F(DDIEMTestFixture, DDIEM_Basic_Set_Cycle_Rate_Test_Fast)
{
  SetFixtureStatus(ddi_em_set_cycle_rate(GetEtherCATMasterHandle(), DDI_EM_TEST_CYCLIC_RATE_FAST));
  ASSERT_EQ(DDI_EM_STATUS_OK, GetFixtureStatus());

  //print out the timing info
  SetFixtureStatus(ddi_em_get_master_stats(GetEtherCATMasterHandle(), GetMasterStatsPointer()));
  EXPECT_EQ(GetFixtureStatus(), DDI_EM_STATUS_OK ) << "DDI Master Stats Failed: Result = &d\n", GetFixtureStatus();

  printf("**** Stats for master %d \n", GetEtherCATMasterHandle());
  printf("master_stats.cur_consecutive_lost_frame_count 0x%04x\n", GetMasterStatsPointer()->cur_consecutive_err_frame_count);
  printf("master_stats.max_consecutive_lost_frame_count 0x%04x\n", GetMasterStatsPointer()->max_consecutive_err_frame_count);
  printf("master_stats.cyclic_error_frame_count 0x%04x\n", GetMasterStatsPointer()->cyclic_err_frame_count);
  printf("master_stats.cyclic_frames_with_no_errors %" PRIu64 "\n", GetMasterStatsPointer()->cyclic_frames_with_no_errors);
  printf("master_stats.max_cyclic_timestamp_diff_ns %d\n", GetMasterStatsPointer()->max_cyclic_timestamp_diff_ns);
  printf("master_stats.min_cyclic_timestamp_diff_ns %d\n", GetMasterStatsPointer()->min_cyclic_timestamp_diff_ns);
  printf("master_stats.average_cyclic_timestamp_diff_ns %d\n", GetMasterStatsPointer()->average_cyclic_timestamp_diff_ns);
}

// Tests ddi_em_set_cycle_rate function with a slow rate, this also contains a test for ddi_em_get_master_stats
TEST_F(DDIEMTestFixture, DDIEM_Basic_Set_Cycle_Rate_Test_Slow)
{
  SetFixtureStatus(ddi_em_set_cycle_rate(GetEtherCATMasterHandle(), DDI_EM_TEST_CYCLIC_RATE_SLOW));
  ASSERT_EQ(DDI_EM_STATUS_OK, GetFixtureStatus());

  //print out the timing info
  SetFixtureStatus(ddi_em_get_master_stats(GetEtherCATMasterHandle(), GetMasterStatsPointer()));
  EXPECT_EQ(GetFixtureStatus(), DDI_EM_STATUS_OK ) << "DDI Master Stats Failed: Result = &d\n", GetFixtureStatus();

  printf("**** Stats for master %d \n", GetEtherCATMasterHandle());
  printf("master_stats.cur_consecutive_lost_frame_count 0x%04x\n", GetMasterStatsPointer()->cur_consecutive_err_frame_count);
  printf("master_stats.max_consecutive_lost_frame_count 0x%04x\n", GetMasterStatsPointer()->max_consecutive_err_frame_count);
  printf("master_stats.cyclic_error_frame_count 0x%04x\n", GetMasterStatsPointer()->cyclic_err_frame_count);
  printf("master_stats.cyclic_frames_with_no_errors %" PRIu64 "\n", GetMasterStatsPointer()->cyclic_frames_with_no_errors);
  printf("master_stats.max_cyclic_timestamp_diff_ns %d\n", GetMasterStatsPointer()->max_cyclic_timestamp_diff_ns);
  printf("master_stats.min_cyclic_timestamp_diff_ns %d\n", GetMasterStatsPointer()->min_cyclic_timestamp_diff_ns);
  printf("master_stats.average_cyclic_timestamp_diff_ns %d\n", GetMasterStatsPointer()->average_cyclic_timestamp_diff_ns);
}

// Tests ddi_em_set_cycle_rate function with negative value for cycle rate
TEST_F(DDIEMTestFixture, DDIEM_Basic_Set_Cycle_Rate_Test_Negative)
{
  SetFixtureStatus(ddi_em_set_cycle_rate(GetEtherCATMasterHandle(), -1));
  ASSERT_NE(DDI_EM_STATUS_OK, GetFixtureStatus());
}

// Tests ddi_em_get_master_stats with an invalid master handle, -1
TEST_F(DDIEMTestFixture, DDIEM_Basic_Get_Master_Stats_Test_Negative)
{
  SetFixtureStatus(ddi_em_get_master_stats(-1, GetMasterStatsPointer()));
  ASSERT_NE(GetFixtureStatus(), DDI_EM_STATUS_OK );
}

// create a test to bring the whole thing to op mode, regular start.
TEST_F(DDIEMTestFixture, DDIEM_Partial_OPModeTest)
{
  DDIEMUtility m_ddi_em_utility;

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
  SetFixtureStatus(ddi_em_register_cyclic_callback(GetEtherCATMasterHandle(), process_data_callback, &pd_callback_args));
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

  // Wait for 10 minutes
  sleep (60);

  printf("Waking up\n");
  // Stop the cylcic thread started by ddi_em_cyclic_task_start() 
  // SetFixtureStatus(ddi_em_cyclic_task_stop(GetEtherCATMasterHandle()));

  // ddi_em_cyclic_task_stop returns 0 if successful
  // ASSERT_EQ(GetFixtureStatus(), DDI_EM_STATUS_OK);
}
