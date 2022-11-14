/**************************************************************************
(c) Copyright 2021 Digital Dynamics Inc. Scotts Valley CA USA.
Unpublished copyright. All rights reserved. Contains proprietary and
confidential trade secrets belonging to DDI. Disclosure or release without
prior written authorization of DDI is prohibited.
**************************************************************************/

// FOE test program


#include <stdio.h>
#include <stdlib.h>
#include "ddi_em_fusion.h"
#include "ddi_debug.h"
#include "string.h"
#include "cram_process_data.h"
#include "ddi_em_uart.h"
#include "ddi_em_common.h"
#include "ddi_em_api.h"

int ddi_log_level = DDI_EM_LOG_LEVEL_ERRORS;

uint8_t foe_buf[20 * 1024 * 1024];

uint32_t send_foe_file (const char *filename, ddi_em_handle em_handle, ddi_es_handle es_handle)
{
  ddi_em_result result;
  FILE *fp = fopen("fusion-1.09.6.efw", "r");
  fseek(fp, 0, SEEK_END);
  int file_size = ftell(fp);
  rewind(fp);
  fread(foe_buf, 1, file_size, fp);
  printf("file_size %d \n", file_size);
  result = ddi_em_foe_write(em_handle, es_handle, (const char *)"fusion-1.09.6.efw", strlen("fusion-1.09.6.efw"),foe_buf, file_size, 0,TEST_DEFAULT_TIMEOUT*100 );
  printf("result %d \n", result);
  return result;
}

int main (int argc, char **argv)
{
  // DDI EtherCAT Master Handle
  ddi_em_handle em_handle;
  // DDI 
  ddi_es_handle es_handle;
  // DDI EtherCAT result type
  ddi_em_result result;
  // DDI EtherCAT Master Initialization Parameters
  ddi_em_init_params init_params;

  memset(&init_params, 0, sizeof(ddi_em_init_params));

  // Initialize the global DDI SDK Instance
  result = ddi_em_sdk_init();
  if ( result != DDI_EM_STATUS_OK )
  {
    printf("ddi_em_sdk_init failed: 0x%04x (%s) \n", result, ddi_em_get_error_string(result));
    return -1;
  }
  // EtherCAT network adapter to use
  init_params.network_adapter         = DDI_EM_NIC_2;
  // Disable the remote client capability
  init_params.remote_client_enable    = DDI_EM_REMOTE_DISABLED;
  // Enable direct call-in from an external thread
  init_params.enable_cyclic_thread    = DDI_EM_TRUE;
  init_params.scan_rate_us            = DDI_EM_DEFAULT_CYCLIC_RATE;
  // Use the default thread priority for the EtherCAT Cyclic Thread
  init_params.polling_thread_priority = DDI_EM_CYCLIC_THREAD_PRI_DEFAULT;
  // Initialize a master instance
  result = ddi_em_init(&init_params, &em_handle);
  if ( result != DDI_EM_STATUS_OK )
  {
    printf("ddi_em_init failed: 0x%04x (%s) \n", result, ddi_em_get_error_string(result));
    return 1;
  }
  printf("init success \n");
  // Configure the master with an ENI file
  result = ddi_em_configure_master(em_handle, "tests/config/cram_eni.xml");
  if ( result != DDI_EM_STATUS_OK )
  {
    ELOG("ddi_em_configure_master failed: 0x%04x (%s) \n", result, ddi_em_get_error_string(result));
    return 1;
  }

  // Open the Fusion with vendor and product ID
  result = ddi_em_open_slave_by_id(em_handle, DDI_ETHERCAT_VENDOR_ID, DDI_FUSION_PRODUCT_CODE, 0, 0, &es_handle);
  if ( result != DDI_EM_STATUS_OK )
  {
    ELOG("ddi_em_open_slave_by_id failed: 0x%04x (%s) \n", result, ddi_em_get_error_string(result));
    return 1;
  }

  ddi_em_state set_em_state = DDI_EM_STATE_PREOP;
  // Set the EtherCAT Master State to OP mode
  // This will also set the EtherCAT slave(s) mode to the desired mode
  result = ddi_em_set_master_state(em_handle, DDI_EM_STATE_PREOP, TEST_DEFAULT_TIMEOUT);  
  if ( result != DDI_EM_STATUS_OK )
  {
    ELOG("ddi_em_set_master_state failed: 0x%04x (%s) \n", result, ddi_em_get_error_string(result));
    // Return an error
    return 1;
  }

  ddi_em_state verify_em_state;
  // Set the EtherCAT Master State to OP mode
  // This will also set the EtherCAT slave(s) mode to the desired mode
  result = ddi_em_get_master_state(em_handle, &verify_em_state);  
  if ( result != DDI_EM_STATUS_OK )
  {
    ELOG("ddi_em_get_master_state failed: 0x%04x (%s) \n", result, ddi_em_get_error_string(result));
    // Return an error
    return 1;
  }

  if ( verify_em_state != set_em_state)
  {
    ELOG("Verified master state 0%d did not set state %d \n", verify_em_state, set_em_state);
    return 1;
  }

  ddi_es_state set_es_state = DDI_ES_STATE_BOOT;
  // Set the EtherCAT Master State to OP mode
  // This will also set the EtherCAT slave(s) mode to the desired mode
  result = ddi_em_set_slave_state(em_handle, es_handle, DDI_ES_STATE_BOOT, TEST_DEFAULT_TIMEOUT);  
  if ( result != DDI_EM_STATUS_OK )
  {
    printf("Set slave state state: %s \n", ddi_em_get_error_string(result));
    // Return an error
    return 1;
  }

  ddi_es_state verify_es_state;
  // Set the EtherCAT Master State to OP mode
  // This will also set the EtherCAT slave(s) mode to the desired mode
  result = ddi_em_get_slave_state(em_handle, es_handle, &verify_es_state);  
  if ( result != DDI_EM_STATUS_OK )
  {
    ELOG("Get state failed: %s \n", ddi_em_get_error_string(result));
    // Return an error
    return 1;
  }  

  if ( verify_es_state != set_es_state)
  {
    ELOG("Verified slave state 0%d did not set state %d \n", verify_em_state, set_em_state);
    return 1;
  }

  result = ddi_em_close_slave(em_handle, es_handle);
  if ( result != DDI_EM_STATUS_OK )
  {
    ELOG("Close slave: %s \n", ddi_em_get_error_string(result));
    // Return an error
    return 1;
  }  

  return send_foe_file("fusion-1.09.6.efw", em_handle, es_handle);
}
