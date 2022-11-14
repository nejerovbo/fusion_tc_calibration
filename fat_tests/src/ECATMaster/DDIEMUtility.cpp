/**************************************************************************
(c) Copyright 2021 Digital Dynamics Inc. Scotts Valley CA USA.
Unpublished copyright. All rights reserved. Contains proprietary and
confidential trade secrets belonging to DDI. Disclosure or release without
prior written authorization of DDI is prohibited.
**************************************************************************/
#include "DDIEMUtility.h"
#include "ddi_em_api.h"

uint16_t g_uart_pd_in[12];

void DDIEMUtility::UART_cyclic_function (void *user_args)
{
  uint16_t *uart_pd;
  ddi_em_handle em_handle;
  uint32_t pd_input_offset;
  // Validate parameters
  if (!user_args)
    return;
  pd_callback_args *callback_args;
  if ( user_args == NULL )
  {
    ELOG("Callback arguments are NULL \n");
    return;
  }
  callback_args = (pd_callback_args *)user_args;
 
  em_handle = callback_args->em_handle;
  pd_input_offset = callback_args->es_cfg->pd_input_offset;
  // Populate the global input status value so it can be used by the UART test application
  ddi_em_get_process_data(em_handle, pd_input_offset, (uint8_t*)&g_uart_pd_in, sizeof(g_uart_pd_in), false);
  uart_pd = g_uart_pd_in;
  //for ( int count = 0; count < 12; count++ )
  //{
  //  printf("0x%x \n", *uart_pd++);
  //}
  //clrscr();
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

// Handle display of the input values
void DDIEMUtility::handle_pd_input_update (pd_callback_args *callback_args)
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
void DDIEMUtility::handle_pd_output_update (pd_callback_args *callback_args)
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

// Simulate a thread manager creating the thread used by the DDI ECAT Master SDK
int DDIEMUtility::start_thread_manager (uint32_t em_handle, pthread_t *pthread_handle)
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
  return 0;
}

uint32_t event_handler (ddi_em_event *event)
{
  static int frame_response_count = 0;
  switch ( event->event_code )
  {
    case DDI_EM_EVENT_STATE_CHANGED:
      printf("Master state changed \n");
      break;
    case DDI_ES_EVENT_STATE_CHANGED:
      printf("Slave state changed \n");
      break;
    case DDI_EM_EVENT_FRAMELOSS:
      printf("Frameloss detected \n");
      break;
    case DDI_ES_EVENT_PRESENCE:
      printf("Slave presence changed \n");
      break;
    case DDI_ES_EVENT_ERR_FRAME_RESPONSE:
      printf("Frame response error \n");
      frame_response_count++;
      printf("Frame response error count %d \n", frame_response_count);
      break;
    default:
      ELOG("Coming from DDIEMUtility::Unsupported notification code 0x%x \n", event->event_code);
      break;
  }
  return 0;
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
  //handle_pd_input_update(callback_args);
  // Handle process data output update
  //handle_pd_output_update(callback_args);
}
