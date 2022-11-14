/**************************************************************************
(c) Copyright 2021 Digital Dynamics Inc. Scotts Valley CA USA.
Unpublished copyright. All rights reserved. Contains proprietary and
confidential trade secrets belonging to DDI. Disclosure or release without
prior written authorization of DDI is prohibited.
**************************************************************************/

// Performs tests on the ddi_em_master library

#include <iostream>
#include <inttypes.h>
#include "ddi_em_api.h"
#include "ddi_em_fusion.h"
#include "ddi_debug.h"
#include "string.h"
#include "cram_process_data.h"
#include "ddi_em_uart.h"
#include "ddi_em_common.h"

#define MAX_MASTER_TEST_INSTANCES 4

#define TEST_FAILED -1
#define TEST_PASSED 0

int ddi_log_level = 3;

ddi_em_result initialize_remote_server (ddi_ra_handle *remote_access_handle)
{
  ddi_em_result result;
  ddi_em_remote_access_init_params remote_init_params;

  remote_init_params.remote_port = 6000;
  // Initialize the remote access server to listen on port 6000
  result = ddi_em_remote_access_init(&remote_init_params, remote_access_handle);
  if ( result != DDI_EM_STATUS_OK )
  {
    ELOG("Error during remote access init \n");
  }
  return result;
}

// Test the DDI EtherCAT Master Initialization function
ddi_em_handle test_ddi_em_initialization (int instance)
{
  // DDI EtherCAT Master Handle
  ddi_em_handle em_handle;
  // DDI EtherCAT result type
  ddi_em_result result;
  // DDI EtherCAT Master Initialization Parameters
  ddi_em_init_params init_params;

  memset(&init_params, 0, sizeof(ddi_em_init_params));
  // This is a test configuration located at the 231 lab
  init_params.enable_cpu_affinity = 1;
  if ( instance == 0)
  {
	  init_params.cyclic_cpu_select = DDI_EM_CPU_1;
    init_params.network_adapter         = DDI_EM_NIC_2;
  }
  else if ( instance == 1 )
  {
    init_params.cyclic_cpu_select = DDI_EM_CPU_2;
    init_params.network_adapter         = DDI_EM_NIC_4;
  }
  else if ( instance == 2 )
  {
    init_params.cyclic_cpu_select = DDI_EM_CPU_3;
    init_params.network_adapter         = DDI_EM_NIC_1;
  }
  else if ( instance == 3 )
  {
    init_params.cyclic_cpu_select = DDI_EM_CPU_4;
    init_params.network_adapter         = DDI_EM_NIC_3;
  }
  // Use the default thread priority for the EtherCAT Cyclic Thread
  init_params.polling_thread_priority = DDI_EM_CYCLIC_THREAD_PRI_DEFAULT;
  // Disable the remote client capability
  init_params.remote_client_enable    = DDI_EM_REMOTE_DISABLED;
  // Use the default cyclic data rate of 1000 microseconds
  init_params.scan_rate_us            = DDI_EM_DEFAULT_CYCLIC_RATE;
  init_params.enable_cyclic_thread    = 1;
  // Initialize a master instance
  result = ddi_em_init(&init_params, &em_handle);
  if ( result != DDI_EM_STATUS_OK )
  {
    // Return an invalid handle
    return result;
  }
  // Return a valid handle
  return em_handle;
}

// Set the EtherCAT Master ENI Configuration
ddi_em_result test_eni_configuration (ddi_em_handle em_handle)
{
  const char *config;
  if (em_handle == 0)
    config = "tests/config/cram_eni.xml";
  else
    config = "tests/config/cram_eni.xml";
  return ddi_em_configure_master(em_handle, config);
}

// Test the EtherCAT master State Change mode
ddi_em_result test_ddi_em_coe_access (ddi_em_handle em_handle, ddi_em_handle slv_handle)
{
  int test_iterations = 0;
  char data[256];
  uint32_t out_len;
  ddi_em_result result;
  memset(data,0,sizeof(data));
  for (test_iterations = 0; test_iterations < 1000; test_iterations++)
  {
    result = ddi_em_coe_read(em_handle, slv_handle, 0x100A, 0, (uint8_t *)data, 256, &out_len, 10000, 0);
    if ( result != DDI_EM_STATUS_OK )
    {
      ELOG("Error during COE Read [0x%04x = %s ]\n", result, ddi_em_get_error_string(result));
      return result;
    }
    data[out_len] = 0; // Null terminate the result;
    // Display the result
    printf("SW version = %s \n", data);
    //if ( strncmp (data, (const char *)"1.09.6", out_len) )
    //{
    //  return DDI_EM_DIAG_INVALID_COMPARE;
    //}
  }
  return DDI_EM_STATUS_OK;
}

// Test the EtherCAT master State Change mode
ddi_em_result test_ddi_em_state_change (ddi_em_handle em_handle, ddi_em_state state)
{
  // Set the EtherCAT Master State to the desired mode
  // This will also set the EtherCAT slave(s) mode to the desired mode
  return ddi_em_set_master_state(em_handle, state, TEST_DEFAULT_TIMEOUT);
}

// Open EtherCAT slave by returning the slave handle of the first slave that matches the vendor_id
// and product code
ddi_es_handle test_ddi_em_slave_open (ddi_em_handle em_handle, uint32_t vendor_id, uint32_t product_code)
{
  ddi_es_handle slave_handle;
  ddi_em_open_slave_by_id(em_handle, vendor_id, product_code, 0, 0, &slave_handle);
  return slave_handle;
}

// Print out the callback information
// This will printout a Callback notification event message
uint32_t event_handler (ddi_em_event *event)
{
  static int frame_response_count = 0;
  printf("Instance [%d] Event flags = 0x%x (%s)\n", event->master_handle, event->event_code, event->event_str);
  switch ( event->event_code )
  {
    case DDI_EM_EVENT_STATE_CHANGED:
      printf(GREEN "Master state changed " CLEAR "\n");
      break;
    case DDI_ES_EVENT_STATE_CHANGED:
      printf(YELLOW "Slave state changed " CLEAR "\n");
      break;
    case DDI_EM_EVENT_FRAMELOSS:
      printf(RED "Frameloss detected \n");
      break;
    case DDI_ES_EVENT_PRESENCE:
      printf(RED "Slave precense changed \n");
      break;
    case DDI_ES_EVENT_ERR_FRAME_RESPONSE:
      printf("Frame response error \n");
      frame_response_count++;
      printf("Frame response error count %d \n", frame_response_count);
      break;
    default:
      ELOG("Unsupported notification code 0x%x \n", event->event_code);
      break;
  }
  return 0;
}
ddi_em_result test_ddi_em_event_registration(ddi_em_handle em_handle)
{
  ddi_em_result result;
  result = ddi_em_enable_event_handler(em_handle, DDI_ES_EVENT_STATE_CHANGED, DDI_EM_EVENT_DISABLE_EVENT);
  if ( result != DDI_EM_STATUS_OK )
  {
    return result;
  }
  result = ddi_em_enable_event_handler(em_handle, DDI_ES_EVENT_STATE_CHANGED, DDI_EM_EVENT_ENABLE_EVENT);
  if ( result != DDI_EM_STATUS_OK )
  {
    return result;
  }
  return DDI_EM_STATUS_OK;
}

// test DDI EtherCAT Master slave changed notification
ddi_em_result test_ddi_em_notification (ddi_em_handle em_handle)
{
  // Register the notification event handler
  return ddi_em_set_event_handler(em_handle, event_handler);
}

int g_pd_callbacks = 0;
static CRAM_pd_in_config_t  g_input_pd[MAX_MASTER_TEST_INSTANCES];
static CRAM_pd_out_config_t g_output_pd[MAX_MASTER_TEST_INSTANCES];

static ddi_em_handle g_em_master_handles[MAX_MASTER_TEST_INSTANCES];
static ddi_em_handle g_em_slave_handles[MAX_MASTER_TEST_INSTANCES];

// Test Process data callback with CRAM configuration
void process_data_callback (void *args)
{
  uint32_t threshold;
  ddi_em_handle instance;
  ddi_em_handle *em_instance = (ddi_em_handle *)args;
  instance = *em_instance;
  static uint32_t count[MAX_MASTER_TEST_INSTANCES];
  threshold = (instance) * 5000; // Trigger a process data frequency based on the instance
  if ( count[instance] == threshold ) // Update and display data every 1 millisecond (TODO: use ntime)
  {
    g_output_pd[instance].dout_relay_slot22 = ~g_output_pd[instance].dout_relay_slot22;
    g_output_pd[instance].dout_relay_slot19 = ~g_output_pd[instance].dout_relay_slot19;
    g_output_pd[instance].dout_opto_slot21 = ~g_output_pd[instance].dout_opto_slot21;
    g_output_pd[instance].dout_opto_slot24 = ~g_output_pd[instance].dout_opto_slot24;
    ddi_em_get_process_data(instance,39,(uint8_t*)&g_input_pd[instance], sizeof(CRAM_pd_in_config_t), false);
    //for ( int ain_count = 0; ain_count < 8; ain_count++)
    //  printf("Val = %x \n", g_input_pd[instance].ain_x16_slot13[ain_count]);
    ddi_em_set_process_data(instance,39,(uint8_t*)&g_output_pd[instance], sizeof(CRAM_pd_out_config_t));
    count[instance]=0;
  }
  count[instance]++;
  g_pd_callbacks++;
  return;
}

// test DDI EtherCAT Master Cyclic data callback
ddi_em_result test_ddi_em_cyclic_data_callback (ddi_em_handle em_handle)
{
  // Instance 0 has special handling for the UART
  if ( em_handle == 0)
    return ddi_em_register_cyclic_callback(em_handle, UART_cyclic_method, &g_em_master_handles[em_handle]);
  else
    return ddi_em_register_cyclic_callback(em_handle, process_data_callback, &g_em_master_handles[em_handle]);
}

// Make sure UARTs and process data accesses are working
int execute_basic_test (ddi_em_handle em_handle, ddi_em_handle *slave_handle)
{
  ddi_em_result result;
  ddi_es_handle es_handle;
  ddi_em_slave_config cfg_info;
  ddi_em_set_logging_level(em_handle, DDI_EM_LOG_LEVEL_ERRORS);
  // Use a test ENI configuration
  result = test_eni_configuration(em_handle);
  if ( result != DDI_EM_STATUS_OK)
  {
    ELOG("DDI Configuration Test Configuration Failed Result (0x%04x = %s)\n",\
      result, ddi_em_get_error_string(result));
    return TEST_FAILED;
  }
  // Open a slave instance using the DDI EtherCAT Vendor ID and Fusion Product Code
  es_handle = test_ddi_em_slave_open(em_handle, DDI_ETHERCAT_VENDOR_ID, DDI_FUSION_PRODUCT_CODE );
  if ( es_handle == DDI_EM_INVALID_HANDLE)
  {
    ELOG("DDI Slave Open Test Failed = Handle %d ",em_handle);
    return TEST_FAILED;
  }
  // Test the slave retrieval information function
  result = ddi_em_get_slave_config(em_handle, es_handle, &cfg_info);
  if ( result != DDI_EM_STATUS_OK)
  {
    ELOG("DDI Notification Configuration Failed Result (0x%04x = %s)\n",\
      result, ddi_em_get_error_string(result));
    exit(EXIT_FAILURE);
    return TEST_FAILED;
  }
  printf("pd input offset 0x%04x \n", cfg_info.pd_input_offset);
  printf("pd output offset 0x%04x \n", cfg_info.pd_output_offset);
  printf("pd input size 0x%04x \n", cfg_info.pd_input_size);
  printf("pd output size 0x%04x \n", cfg_info.pd_output_size);
  // Test notification handling
  result = test_ddi_em_notification(em_handle);
  if ( result != DDI_EM_STATUS_OK)
  {
    ELOG("DDI Notification Configuration Failed Result (0x%04x = %s)\n",\
      result, ddi_em_get_error_string(result));
    return TEST_FAILED;
  }
  // Test a state change to OP mode
  result = test_ddi_em_state_change(em_handle, DDI_EM_STATE_PREOP);
  if ( result != DDI_EM_STATUS_OK)
  {
    ELOG("DDI State Change Test Configuration Failed Result (0x%04x = %s)\n",\
      result, ddi_em_get_error_string(result));
    return TEST_FAILED;
  }
  result = test_ddi_em_event_registration(em_handle);
  if ( result != DDI_EM_STATUS_OK)
  {
    ELOG("DDI Event Registration Test Failed Result (0x%04x = %s)\n",\
      result, ddi_em_get_error_string(result));
    return TEST_FAILED;
  }
  if ( em_handle == 0)
  {
    UART_channel_select(em_handle, es_handle, 0);
    UART_select_baud(em_handle, es_handle, (const char *)"9600");
    UART_channel_flush(em_handle, es_handle);
  }
  // Test a state change to OP mode
  result = test_ddi_em_state_change(em_handle, DDI_EM_STATE_OP);
  if ( result != DDI_EM_STATUS_OK)
  {
    ELOG("DDI State Change Test Configuration Failed Result (0x%04x = %s)\n",\
      result, ddi_em_get_error_string(result));
    return TEST_FAILED;
  }
  // Test process data callback registering
  result = test_ddi_em_cyclic_data_callback(em_handle);
  if ( result != DDI_EM_STATUS_OK)
  {
    ELOG("DDI Cyclic Data Failed Result (0x%04x = %s)\n",\
      result, ddi_em_get_error_string(result));
    return TEST_FAILED;
  }
#if 0 // Direct cyclic task test
  // Test notification handling
  result = ddi_em_cyclic_task_start(em_handle);
  if ( result != DDI_EM_STATUS_OK)
  {
    ELOG("DDI Cyclic Data Failed Result (0x%04x = %s)\n",\
      result, ddi_em_get_error_string(result));
    return TEST_FAILED;
  }
#endif 
  // Test COE access
  result = test_ddi_em_coe_access(em_handle, es_handle);
  if ( result != DDI_EM_STATUS_OK)
  {
    ELOG("DDI COE Test Failed Result (0x%04x = %s)\n",\
      result, ddi_em_get_error_string(result));
    return TEST_FAILED;
  }
  // Return the slave handle
  *slave_handle = es_handle;
  return TEST_PASSED;
}

// Perform a basic EtherCAT Initialization Sequence
int main (int argc, char *argv[])
{
  ddi_em_result  result;
  ddi_em_master_stats master_stats;
  const char *version = ddi_em_get_version();
  uint16_t al_status;
  uint16_t al_status_code;
  std::cout << "DDI EM Version = " << version << "\n";
  memset(&g_input_pd, 0, sizeof(g_input_pd));
  memset(&g_output_pd, 0, sizeof(g_output_pd));
  if ( argc >= 2 )
  {
    if ( argv[1] && strncmp (argv[1], "github-action", 13) == 0 )
      // If this is a github action, don't run any additional tests
      return TEST_PASSED;
  }
  // Initialize the DDI SDK Instance
  result = ddi_em_sdk_init();
  if ( result != DDI_EM_STATUS_OK)
  {
    ELOG("DDI SDK Test Failed: Result = %d\n", result);
    return TEST_FAILED;
  }

  ddi_ra_handle remote_access_handle;
  initialize_remote_server(&remote_access_handle);
  // Test multiple master instances
  int master = 0;
  for ( master = 0; master < MAX_MASTER_TEST_INSTANCES; master++ )
  {
    // Initialize a DDI EtherCAT Master Instance
    g_em_master_handles[master] = test_ddi_em_initialization(master);
    if ( g_em_master_handles[master] == DDI_EM_INVALID_HANDLE )
    {
      ELOG("DDI Initialization Test Failed: Handle  = %d\n", g_em_master_handles[master]);
      return TEST_FAILED;
    }
    execute_basic_test(g_em_master_handles[master], &g_em_slave_handles[master]);
    ddi_em_get_alstatus(g_em_master_handles[master], g_em_slave_handles[master],&al_status, &al_status_code, 10000);
    printf("al_status 0x%x \n", al_status);
    printf("al_status_code 0x%x \n", al_status_code);
  }


  while ( 1) 
  {
  // Test UART before exiting

  // Get master statistics
  for ( master = 0; master < 4; master++ )
  {
    ddi_em_get_master_stats(g_em_master_handles[master], &master_stats);
    if ( result != DDI_EM_STATUS_OK )
    {
      ELOG("DDI Master Stats Failed: Result  = %d\n", result);
      return TEST_FAILED;
    }
    printf("**** Stats for master %d \n", master);
    printf("master_stats.cur_consecutive_lost_frame_count 0x%04x\n", master_stats.cur_consecutive_err_frame_count);
    printf("master_stats.max_consecutive_lost_frame_count 0x%04x\n", master_stats.max_consecutive_err_frame_count);
    printf("master_stats.cyclic_err_frame_count 0x%04x\n", master_stats.cyclic_err_frame_count);
    printf("master_stats.cyclic_frames_with_no_errors %" PRIu64 "\n", master_stats.cyclic_frames_with_no_errors);
    printf("master_stats.max_cyclic_timestamp_diff_ns %d\n", master_stats.max_cyclic_timestamp_diff_ns);
    printf("master_stats.min_cyclic_timestamp_diff_ns %d\n", master_stats.min_cyclic_timestamp_diff_ns);
    printf("master_stats.average_cyclic_timestamp_diff_ns %d\n", master_stats.average_cyclic_timestamp_diff_ns);
  }
    sleep (60);
  }

  // De-initialize a DDI EtherCAT Master Instance
  for ( master = 0; master < 4; master++ )
  {
    result = ddi_em_deinit(master);
    if ( result != DDI_EM_STATUS_OK )
    {
      ELOG("DDI De-init Test Failed: Result  = %d\n", result);
      return TEST_FAILED;
    }
  }  
  printf("%d callbacks received \n", g_pd_callbacks);

  return DDI_EM_STATUS_OK;
}
