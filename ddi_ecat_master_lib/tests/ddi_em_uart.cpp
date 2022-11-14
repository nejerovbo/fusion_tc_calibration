/**************************************************************************
(c) Copyright 2021 Digital Dynamics Inc. Scotts Valley CA USA.
Unpublished copyright. All rights reserved. Contains proprietary and
confidential trade secrets belonging to DDI. Disclosure or release without
prior written authorization of DDI is prohibited.
**************************************************************************/

// Performs UART tests on the ddi_em_master library

#include <stdio.h>
#include <string.h>
#include "ddi_em_api.h"
#include "ddi_defines.h"
#include "ddi_defines.h"
#include "ddi_ntime.h"
#include "ddi_debug.h"
#include "ddi_status.h"
#include "ddi_em_common.h"
#include "cram_process_data.h"

uint16_t g_status_value;

void UART_cyclic_method (void *arg)
{
  static int count  = 0;
  static uint16_t toggle_on = 0;

  static CRAM_pd_out_config_t g_output_pd;

  // Validate parameters
  if (!arg)
    return;

  if ( count == 1000 ) // Update and display data every 1 millisecond (TODO: use ntime)
  {
    toggle_on = ~toggle_on;
    memset(&g_output_pd, toggle_on, sizeof(CRAM_pd_out_config_t));
    //for ( int ain_count = 0; ain_count < 8; ain_count++)
    //  printf("Val = %x \n", g_input_pd[instance].ain_x16_slot13[ain_count]);
    ddi_em_set_process_data(0,39,(uint8_t*)&g_output_pd, sizeof(CRAM_pd_out_config_t));
    count=0;
  }
  count++;

  // Populate the global status value so it can be used by the test application
  uint16_t status;
  ddi_em_get_process_data(0, 39, (uint8_t*)&status, sizeof(status), false);
  g_status_value = status;
}

#define UART_INFORMATION_SDO 0x5001

PACKED_BEGIN
typedef struct PACKED{
  uint8_t  uart_select;
  uint16_t uart_bytes_rxtx;
  uint8_t  payload[253];
} uart_data_t;
PACKED_END

ddi_em_result UART_RX (ddi_em_handle master_handle, ddi_em_handle slave_handle, uint8_t *dest_buffer, uint16_t *rx_length )
{
  uint32_t length, result;
  uint8_t data[256]; // UART Rx SDO is 256 Bytes
  uart_data_t *uart_rx = (uart_data_t *) data;
#ifdef ACONTIS_PROFILE_SDO_ACCESS // Enable this to track performance of the ecatCoeSdoUpload call
  ntime_t perf_start, perf_end;
  ddi_ntime_get_systime(&perf_start);
#endif
  memset(data,0, sizeof(data));
  //printf("SDO Rx Start\n");
  // Set the Synchronization type in the SM output parameter
  result = ddi_em_coe_read(master_handle, slave_handle, UART_INFORMATION_SDO, 2,\
    (uint8_t*)&data, 256, &length, TEST_DEFAULT_TIMEOUT, 0);
#ifdef ACONTIS_PROFILE_SDO_ACCESS
  ddi_ntime_get_systime(&perf_end);
  uint64_t diff_ns = ddi_ntime_diff_ns(&perf_end, &perf_start );
  printf("Upload SDO access took %" PRIu64 " ns \n", diff_ns);
#endif
  //printf("SDO Rx End\n");
  int count;
  for ( count = 0; count < 10; count++)
  {
    //printf("Rx[%02d] = 0x%02x \n", count, data[count]);
  }
  //printf("uart_bytes_rxtx = %d \n", uart_rx->uart_bytes_rxtx);
  // Copy payload to destination buffer
  memcpy (dest_buffer, uart_rx->payload, uart_rx->uart_bytes_rxtx);
  // Set the number of bytes read
  *rx_length = uart_rx->uart_bytes_rxtx;
  if ( result == 0)
    return DDI_EM_STATUS_OK;
  else
    return DDI_ES_COE_ERR;
}

// Start or stop streaming mode
ddi_em_result UART_tx_match(ddi_em_handle master_handle, ddi_em_handle slave_handle, bool start_streaming)
{
  ddi_em_result result;
  uint8_t data[512];
  uart_data_t *tx_data = (uart_data_t *)data;
  tx_data->uart_select = 0;
  tx_data->uart_bytes_rxtx = 3;

  if ( start_streaming == true )
  {
    tx_data->payload[0] = 'S';
    tx_data->payload[1] = '3';
    tx_data->payload[2] = 0xd;
  }
  else
  {
    tx_data->payload[0] = 'S';
    tx_data->payload[1] = 'P';
    tx_data->payload[2] = 0xd;
  }

#ifdef ACONTIS_PROFILE_SDO_ACCESS // Enable this to track performance of the ecatCoeSdoDownload call
  ntime_t perf_start, perf_end;
  ddi_ntime_get_systime(&perf_start);
#endif
  //printf("***SDO tx start \n");
  // Set the Synchronization type in the SM output parameter
  result = ddi_em_coe_write(master_handle, slave_handle, UART_INFORMATION_SDO, 2,\
    (uint8_t*)&data, 10, TEST_DEFAULT_TIMEOUT, 0);
  //printf("***SDO tx end\n");
#ifdef ACONTIS_PROFILE_SDO_ACCESS
  ddi_ntime_get_systime(&perf_end);
  uint64_t diff_ns = ddi_ntime_diff_ns(&perf_end, &perf_start );
  printf("UART write took %" PRIu64 "\n", diff_ns);
#endif

  return result;
}

#define MS_100 ( NSEC_PER_MSEC * 100 )

#define STATUS_STARTED 1
#define STATUS_STOPPED 2
#define STATUS_PAUSED  3

ddi_em_result UART_channel_select (ddi_em_handle master_handle, ddi_em_handle slave_handle, uint8_t channel)
{
  uint32_t length;
  uint8_t data[1];
  uint8_t verify[1];
  ddi_em_result result;

  data[0] = channel;

  // Set the UART channel select
  result = ddi_em_coe_write(master_handle, slave_handle, UART_INFORMATION_SDO, 1,\
    (uint8_t*)&data, sizeof(data), TEST_DEFAULT_TIMEOUT, 0);

  // Verify the channel select matches the set value
  result = ddi_em_coe_read(master_handle, slave_handle, UART_INFORMATION_SDO, 1,\
    (uint8_t*)&verify, sizeof(verify), &length, TEST_DEFAULT_TIMEOUT, 0);

  return result;
}

ddi_em_result UART_channel_flush (ddi_em_handle master_handle, ddi_em_handle slave_handle)
{
  uint16_t data[1];
  ddi_em_result result;

  // Set the UART channel select
  result = ddi_em_coe_write(master_handle, slave_handle, UART_INFORMATION_SDO, 3,\
    (uint8_t*)&data, sizeof(data), TEST_DEFAULT_TIMEOUT, 0);

  return result;
}

// Preop-mode only
ddi_em_result UART_select_baud (ddi_em_handle master_handle, ddi_em_handle slave_handle, const char *baud)
{
  ddi_em_result result;

  // Set the UART baud rate
  result = ddi_em_coe_write(master_handle, slave_handle, UART_INFORMATION_SDO, 6,\
    (uint8_t*)baud, strlen(baud), TEST_DEFAULT_TIMEOUT, 0);
  
  return result;
}

ddi_em_result UART_SDO_impedance_match (ddi_em_handle master_handle, ddi_em_handle slave_handle)
{
  uint8_t rx_data[512];
  int streaming_status = STATUS_STARTED;
  int test_iteration = 0;

  clrscr();

  UART_tx_match(master_handle, slave_handle, true);

  ntime_t poll_deadline;
  ntime_t current_time;
  int64_t poll_result;
  
  uint16_t rx_bytes;

  printf(BLUE "UART Center Match Demonstration Application\n");
  printf(BLUE "-------------------------------------------\n");

  printf("\033[%d;%dH", 4, 0);
  printf("\33[2K");
  printf(GREEN "Status: Streaming" CLEAR "\n");

  ddi_ntime_get_systime(&poll_deadline);
  // Poll in 100 milliseconds from now
  ddi_ntime_add_ns(&poll_deadline,0 , MS_100 );
  while ( 1 )
  {
    usleep(5000); // Throttle CPU usage
    ddi_ntime_get_systime(&current_time);
    // If diff is negative then the deadline has expired
    poll_result = ddi_ntime_diff_ns(&poll_deadline, &current_time);
    if ( poll_result < 0 )
    {
      test_iteration++;
      ddi_status_t result;
      char key;
      result = ddi_get_keyboard_input(&key, false);
      if ( result == ddi_status_ok )
      {
        if ( key == 'p')
        {
          printf("\033[%d;%dH", 4, 0);
          printf("\33[2K");
          printf(ORANGE "Status: Paused " CLEAR "\n");
          // Pause streaming - Rx buffer may fill if the previous state was streaming
          streaming_status = STATUS_PAUSED;
        }
        else if ( key == 's')
        {
          printf("\033[%d;%dH", 4, 0);
          printf("\33[2K");
          printf(GREEN "Status: Streaming " CLEAR "\n");
      
          // Start streaming with 'S3' command
          UART_tx_match(master_handle, slave_handle, true);
          streaming_status = STATUS_STARTED;
        }
        else if ( key == 'o')
        {
          printf("\033[%d;%dH", 4, 0);
          printf("\33[2K");
          printf(YELLOW "Status: Stopped" CLEAR "\n");
          // Stop streaming with 'SP' command
          UART_tx_match(master_handle, slave_handle, false);
          streaming_status = STATUS_STOPPED;
        }
      }
      printf("\033[%d;%dH", 5, 0);
      printf(CYAN "Status[0] = 0x%04x" CLEAR "\n", g_status_value);
      //printf("g_status_value = 0x%x \n", g_status_value);
      if ( g_status_value >= 0x37)
      {
        if ( streaming_status != STATUS_PAUSED)
          UART_RX(master_handle, slave_handle, rx_data, &rx_bytes);
        printf("%d Bytes Received \n", rx_bytes);
        if ( rx_bytes != 0 )
        {
          int count = 0;
          for ( count = 0; count < rx_bytes; count++)
          {
            if (rx_data[count] == 0xd )
              rx_data[count] = 0;
            //printf("RxData[%04d] = 0x%02x (" GREEN "%c" CLEAR ") \n", count, rx_data[count], rx_data[count]);
          }
          if (rx_data[0] == 'S')
          {
            printf("\033[%d;%dH", 3, 0);
            printf("\33[2K");
            printf("Rx String = " GREEN "%s" CLEAR "\n", (char *)&rx_data[0]);
          }
        }
      }
      // Schedule for 100 milliseconds from the last polling deadline
      ddi_ntime_add_ns(&poll_deadline,0, MS_100);
    }
  }
}
