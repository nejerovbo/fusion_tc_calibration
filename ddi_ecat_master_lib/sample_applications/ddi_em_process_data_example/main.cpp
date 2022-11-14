/**************************************************************************
(c) Copyright 2021 Digital Dynamics Inc. Scotts Valley CA USA.
Unpublished copyright. All rights reserved. Contains proprietary and
confidential trade secrets belonging to DDI. Disclosure or release without
prior written authorization of DDI is prohibited.
**************************************************************************/

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <pthread.h>
#include "ddi_em_api.h"
#include "ddi_em_fusion.h"
#include "ddi_200143_00_rim_pd.h"
#include "ddi_sample_util.h"
#include "ddi_em_fusion_uart_api.h"

// This application demonstrates initializing the DDI ECAT SDK with a application-created thread
// This application will toggle outputs at 1 kHz and display input process data at 1 Hz

/*! @var SAMPLE_APP_SUCCESS
  @brief Sample app success code
*/
#define SAMPLE_APP_SUCCESS         0
/*! @var SAMPLE_APP_ERROR
  @brief Sample app error code
*/
#define SAMPLE_APP_ERROR           -1
/*! @var TEST_DEFAULT_TIMEOUT
  @brief Value used as a default timeout for Acontis calls
*/
#define TEST_DEFAULT_TIMEOUT       10000
/*! @var DISPLAY_UPDATE_CYCLE_COUNT
  @brief Display rate in cyclic frames
*/
#define DISPLAY_UPDATE_CYCLE_COUNT 1000

/** @struct pd_callback_args
 *  @brief This structure is used for the process data callback
 */
typedef struct {
  ddi_em_handle em_handle;     /**< DDI EtherCAT Master handle */
  ddi_em_slave_config *es_cfg; /**< Slave Configuration Paramters (pd_input_offset, pd_output_offset etc..) */
} pd_callback_args;

// Global copy of the input process data
static six_slot_pd_in_struct  g_pd_input;
// Global copy of the output process data
static six_slot_pd_out_struct g_pd_output;
// Global handle used by the thread manager
static ddi_em_handle          g_em_handle;

// Handle display of the input values
void handle_pd_input_update (pd_callback_args *callback_args)
{
  static int display_update_count = 0;
  static int iteration = 0;
  int channel = 0;
  ddi_em_result result;
  ddi_em_handle em_handle;
  uint32_t pd_input_offset;  
  em_handle = callback_args->em_handle;
  pd_input_offset = callback_args->es_cfg->pd_input_offset;
  result = ddi_em_get_process_data(em_handle,pd_input_offset,(uint8_t*)&g_pd_input, sizeof(six_slot_pd_in_struct), false);
  if ( result != DDI_EM_STATUS_OK )
  {
    ELOG("ddi_em_get_process_data()= 0x%04x (%s) \n", result, ddi_em_get_error_string(result));
    return;
  }
  display_update_count++;
  if ( display_update_count <= DISPLAY_UPDATE_CYCLE_COUNT) // Delay display for DISPLAY_UPDATE_CYCLE_COUNT cycles
  {
    return; // Return if an update isn't required
  }
  display_update_count = 0;
  iteration++;
  PRINTF_GOTO_XY(0,2); // Display process data from 0,0 on the screen
  // Display AIN 16-channel data
  printf(GREEN "Test iteration 0x%x " CLEAR "\n", iteration);
  for ( channel = 0; channel < CH16; channel++ )
  {
    printf("AIN_slot1[%02d] = 0x%04x \n", channel, g_pd_input.ain_slot1[channel]);
  }
  for ( channel = 0; channel < CH16; channel++ )
  {
    printf("AIN_slot2[%02d] = 0x%4x \n", channel, g_pd_input.ain_slot4[channel]);
  }
  // Display DOUT readback data
  printf("DOUT_readback_slot_6 = 0x%04x \n", g_pd_input.dout_readback_slot_6);
  return;
}

// Handle updates of the output values
void handle_pd_output_update (pd_callback_args *callback_args)
{
  uint32_t pd_output_offset;
  uint16_t aout_value, dout_value;
  ddi_em_result result;
  int channel = 0;
  static int toggle_on = 0; 
  pd_output_offset = callback_args->es_cfg->pd_output_offset;
  if ( toggle_on == 0 )
  {
    dout_value = 0;      // Write 0's to all channels
    aout_value = 0x8000; // -10 V
    toggle_on  = 1;
  }
  else
  {
    dout_value = 0xFFFF; // Write 1's to all channels
    aout_value = 0x7FFF; // +10 V
    toggle_on  = 0;
  }
  // Update the AOUT outputs each cyclic frame
  for (channel = 0; channel < CH8; channel++)
  {
    g_pd_output.aout_slot2[channel] = aout_value;
    g_pd_output.aout_slot3[channel] = aout_value;
  }
  g_pd_output.dout_slot_6 = dout_value;
  result = ddi_em_set_process_data(callback_args->em_handle,pd_output_offset,(uint8_t*)&g_pd_output, sizeof(six_slot_pd_out_struct));
  if ( result != DDI_EM_STATUS_OK )
  {
    ELOG("ddi_em_set_process_data() = 0x%04x (%s) \n", result, ddi_em_get_error_string(result));
    return;
  }
  return;
}

// Called when the cyclic process data receive is complete
void process_data_callback (void *args)
{
  pd_callback_args *callback_args;
  if ( args == NULL )
  {
    ELOG("Callback arguments are NULL \n");
    return;
  }
  callback_args = (pd_callback_args *)args;
 
  // Handle process data input update
  handle_pd_input_update(callback_args);
  // Handle process data output update
  handle_pd_output_update(callback_args);
}

// Called when a registered event occurs in the system
uint32_t event_handler (ddi_em_event *event)
{
  static int frame_response_count = 0;
  switch ( event->event_code )
  {
    case DDI_EM_EVENT_STATE_CHANGED:
      printf(GREEN "Master state changed " CLEAR "\n");
      break;
    case DDI_ES_EVENT_STATE_CHANGED:
      printf(GREEN "Slave state changed " CLEAR "\n");
      break;
    case DDI_EM_EVENT_FRAMELOSS:
      printf(RED "Frameloss detected " CLEAR "\n");
      break;
    case DDI_ES_EVENT_PRESENCE:
      printf(YELLOW "Slave presence changed " CLEAR "\n");
      break;
    case DDI_ES_EVENT_ERR_FRAME_RESPONSE:
      printf(RED "Frame response error " CLEAR "\n");
      frame_response_count++;
      printf("Frame response error count %d \n", frame_response_count);
      break;
    default:
      ELOG("Unsupported notification code 0x%x \n", event->event_code);
      break;
  }
  return 0;
}

// This thread is created by the application's system manager
void *ddi_cyclic_thread (void * args)
{
  ddi_em_result result;
  ddi_em_handle *em_handle;
  if ( args == NULL ) // Valid 
  {
    ELOG("ddi_cyclic_thread arguments are null \n");
    return NULL;
  }
  em_handle = (ddi_em_handle *)args;
  // This call will block until ddi_em_cyclic_task_stop() is called in main
  result = ddi_em_cyclic_task_start(*em_handle);
  if ( result != DDI_EM_STATUS_OK )
  {
    ELOG("ddi_em_cyclic_task_start error \n");
    exit(EXIT_FAILURE);
  }
  return NULL;
}

// Simulate a thread manager creating the thread used by the DDI ECAT Master SDK
int start_thread_manager (uint32_t em_handle, pthread_t *pthread_handle)
{
  int policy;
  int pthread_result;
  pthread_attr_t tattr;
  struct sched_param param;
  // Create the pthread attribute structure
  pthread_attr_init(&tattr);
  // Set the scheduler for FIFO scheduling
  pthread_attr_setschedpolicy(&tattr, SCHED_FIFO);
  pthread_attr_setinheritsched(&tattr, PTHREAD_EXPLICIT_SCHED);
  // Get the min and max priorities of the actual system
  pthread_attr_getschedpolicy(&tattr, &policy);
  pthread_attr_getschedparam(&tattr, &param);
  int m = sched_get_priority_min(policy);
  int x = sched_get_priority_max(policy);
  int priority = 0;
  if (priority < m) priority = m;
  if (priority > x) priority = x;
  param.sched_priority = priority;
  // Set scheduling priority of the cyclic thread
  pthread_attr_setschedparam(&tattr, &param);
  // Pass the thread a global handle so it will still be be valid when the thread executes later
  g_em_handle = em_handle;
  pthread_result = pthread_create(pthread_handle,&tattr, ddi_cyclic_thread, &g_em_handle);
  if ( pthread_result != 0 )
  {
    perror("Error creating thread");
    exit(EXIT_FAILURE);
  }
  return SAMPLE_APP_SUCCESS;
}

int main (int argc, char *argv[])
{
  // DDI EtherCAT Master Handle
  ddi_em_handle em_handle;
  // DDI 
  ddi_es_handle es_handle;
  // DDI EtherCAT result type
  ddi_em_result result;
  ddi_em_slave_config es_cfg;
  // DDI EtherCAT Master Initialization Parameters
  ddi_em_init_params init_params;
  // Process data callback arguments
  pd_callback_args pd_callback_args;
  pthread_t pthread_handle;

  const char *ddi_six_slot_config = "ddi_200143_00_six_slot_rim_eni.xml";
  memset(&init_params, 0, sizeof(ddi_em_init_params));
  memset(&g_pd_input, 0, sizeof(g_pd_input));
  memset(&g_pd_output, 0, sizeof(g_pd_output));

  // Initialize the global DDI SDK Instance
  result = ddi_em_sdk_init();
  if ( result != DDI_EM_STATUS_OK )
  {
    ELOG("ddi_em_sdk_init failed: 0x%04x (%s) \n", result, ddi_em_get_error_string(result));
    return -1;
  }
  // EtherCAT network adapter to use
  init_params.network_adapter         = DDI_EM_NIC_2;
  // Disable the remote client capability
  init_params.remote_client_enable    = DDI_EM_REMOTE_DISABLED;
  // Enable direct call-in from an external thread
  init_params.enable_cyclic_thread    = DDI_EM_FALSE;
  init_params.scan_rate_us            = DDI_EM_DEFAULT_CYCLIC_RATE;
  // Use the default thread priority for the EtherCAT Cyclic Thread
  init_params.polling_thread_priority = DDI_EM_CYCLIC_THREAD_PRI_DEFAULT;
  // Initialize a master instance
  result = ddi_em_init(&init_params, &em_handle);
  if ( result != DDI_EM_STATUS_OK )
  {
    ELOG("ddi_em_init failed: 0x%04x (%s) \n", result, ddi_em_get_error_string(result));
    return SAMPLE_APP_ERROR;
  }

  if ( start_thread_manager(em_handle, &pthread_handle) != SAMPLE_APP_SUCCESS )
  {
    ELOG("create_ddi_cyclic_thread failed: 0x%04x (%s) \n", result, ddi_em_get_error_string(result));
    return SAMPLE_APP_ERROR;
  }

  // Configure the master with an ENI file
  result = ddi_em_configure_master(em_handle, ddi_six_slot_config);
  if ( result != DDI_EM_STATUS_OK )
  {
    ELOG("ddi_em_configure_master failed: 0x%04x (%s) \n", result, ddi_em_get_error_string(result));
    return SAMPLE_APP_ERROR;
  }

  // Open the Fusion with vendor and product ID
  result = ddi_em_open_slave_by_id(em_handle, DDI_ETHERCAT_VENDOR_ID, DDI_FUSION_PRODUCT_CODE, 0, 0, &es_handle);
  if ( result != DDI_EM_STATUS_OK )
  {
    ELOG("ddi_em_open_slave_by_id failed: 0x%04x (%s) \n", result, ddi_em_get_error_string(result));
    return SAMPLE_APP_ERROR;
  }

  // Retrieve the Fusion process data information
  result = ddi_em_get_slave_config(em_handle, es_handle, &es_cfg);
  if ( result != DDI_EM_STATUS_OK)
  {
    ELOG("ddi_em_get_slave_config failed: 0x%04x (%s) \n", result, ddi_em_get_error_string(result));
    return SAMPLE_APP_ERROR;
  }

  // Display slave pd parameters
  printf("Fusion pd input offset 0x%04x \n", es_cfg.pd_input_offset);
  printf("Fusion pd output offset 0x%04x \n", es_cfg.pd_output_offset);
  printf("Fusion pd input size 0x%04x \n", es_cfg.pd_input_size);
  printf("Fusion pd output size 0x%04x \n", es_cfg.pd_output_size);
  pd_callback_args.em_handle = em_handle;
  pd_callback_args.es_cfg = &es_cfg;

  // Register the cyclic callback
  result = ddi_em_register_cyclic_callback(em_handle, process_data_callback, &pd_callback_args);
  if ( result != DDI_EM_STATUS_OK )
  {
    ELOG("ddi_em_register_cyclic_callback failed: 0x%04x (%s) \n", result, ddi_em_get_error_string(result));
    return SAMPLE_APP_ERROR;
  }

  // Register the notification event handler
  result = ddi_em_set_event_handler(em_handle, event_handler);
  if ( result != DDI_EM_STATUS_OK )
  {
    ELOG("ddi_em_set_event_handler failed: 0x%04x (%s) \n", result, ddi_em_get_error_string(result));
    return SAMPLE_APP_ERROR;
  }

  // Set the EtherCAT Master State to OP mode
  // This will also set the EtherCAT slave(s) mode to the desired mode
  result = ddi_em_set_master_state(em_handle, DDI_EM_STATE_OP, TEST_DEFAULT_TIMEOUT);  
  if ( result != DDI_EM_STATUS_OK )
  {
    // Return an error
    return SAMPLE_APP_ERROR;
  }
 
  // Clear display and wait 10 seconds to shutdown the cyclic task for this instance
  clrscr();
  printf(CYAN "**** DDI EtherCAT Master Sample App version 1.0.0\n" CLEAR);
  sleep(120); // Delay for two minutes
  // Stop the cylcic thread started by ddi_em_cyclic_task_start()
  result = ddi_em_cyclic_task_stop(em_handle);
  if ( result != DDI_EM_STATUS_OK )
  {
    // Return an error
    return SAMPLE_APP_ERROR;
  }
  printf("Sample application SUCCESS \n");
  return SAMPLE_APP_SUCCESS;
}
