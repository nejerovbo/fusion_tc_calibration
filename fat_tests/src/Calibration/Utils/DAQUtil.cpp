
#include <inttypes.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <vector>
#include "sys/socket.h"
#include "arpa/inet.h"
#include "ddi_ntime.h"
#include "DAQUtil.h"
#include "indsock.h"
#include "telnet_open.h"
#include "CalibrationCommon.h"

// This is derived from Fusion Calibration Fixture's field connect to DAQ mapping
// these are 4 banks of 8-channel DACs.
// refer to Field Connect to DAQ Wiring Diagram at 
// https://ddieng.atlassian.net/wiki/spaces/DE/pages/1482915881/DAQ970+Notes
static int g_daq_offset_bank_channel_16_chn[4][8] =
{
  { 1, 17,  2, 18,  3, 19,  4, 20},
  { 5, 21,  6, 22,  7, 23,  8, 24},
  {25,  9, 26, 10, 27, 11, 28, 12},
  {29, 13, 30, 14, 31, 15, 32, 16}
};

static int g_daq_offset_bank_channel_8_chn[2][8] =
{
  {1, 2, 3, 4, 5, 6, 7, 8},
  {25, 26, 27, 28, 29, 30, 31, 32}
};

DAQ::DAQ(void)
{
  daq_init = false;
  meter_data[MAX_DAQ_ENTRIES];
  daq_socket = 0;
  requests = {};
}

double DAQ::daq_get_channel_reading(uint channel)
{
  if ( daq_init == 0 )
  {
    printf("Please initialize the DAQ \n");
    exit(EXIT_FAILURE);
  }
  return meter_data[channel];
}

/*-----------------------------------------------------------------------------
 * Collect analog voltages from channel_start to channel_stop on the DAQ790A
 * Store the data in global variable, meter_data
 * This should be called from a non-realtime thread, becaue the recv() takes a long time.
 * This assumes the connection to the DAQ has already been created.
 *-----------------------------------------------------------------------------*/

void DAQ::daq_collect_data(uint channel_start, uint channel_stop)
{
  char  meter_cmd[SEND_BUFF_SIZE], meter_reply[RECV_BUFF_SIZE];
  char  *ptr = meter_reply;
  int   len, offset, number_of_channels;
  double data;
  static uint32_t accum = 0;
  static uint32_t iterations = 0;
  if (daq_init == 0)
  {
    printf("DAQ is not initialized, exiting the application\n");
    exit(EXIT_FAILURE);
  }
  number_of_channels = channel_stop - channel_start + 1;

  ntime_t start, end;
  int64_t diff;
  ddi_ntime_get_systime(&start);
  // sprintf(meter_cmd, "SENSe:VOLTage:DC:NPLCycles 10,(@%03d:%03d)\n", channel_start, channel_stop);
  // len = strlen(meter_cmd);
  // send(daq_socket, meter_cmd, len, 0);

  sprintf(meter_cmd, "MEASure:VOLTage:DC? 10,.000005,(@%03d:%03d)\n", channel_start, channel_stop);
  len = strlen(meter_cmd);
  send(daq_socket, meter_cmd, len, 0);

  // Get the data from the meter.
  memset(meter_reply, 0x00, sizeof(RECV_BUFF_SIZE));
  recv(daq_socket, meter_reply, RECV_BUFF_SIZE, 0);
  ddi_ntime_get_systime(&end);

  diff = ddi_ntime_diff_ns(&end, &start);
 // printf("DAQ time = %" PRId64 "\n", diff/(1000*1000));
  accum += diff/(1000*1000);
  iterations++;
  printf("Avergae DAQ access time (ms)= %d \n", accum/iterations);
 
  // Transfer each voltage to the global array.
  for(int i = 0; i < number_of_channels; i++)
  {
    sscanf(ptr, "%lf,%n", &meter_data[i], &offset);
    data=meter_data[i];
    printf("i%d = %f \n", i, data);
    ptr += offset;
  }
  
  //printf("done\n");
  return;
}

//-----------------------------------------------------------------------------
// Connect to the DAQ on the given ip address.  This address can be changed
//  but needs to match what is set on the meter.
//-----------------------------------------------------------------------------
int DAQ::daq_connect(const char *daq_ip_addr)
{
  struct  sockaddr_in this_saddr;
  char  meter_cmd[SEND_BUFF_SIZE], meter_reply[RECV_BUFF_SIZE];
  int len;

  // Create the socket to talk to the DAQ790A
  daq_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if(-1 == daq_socket)
  {
    printf("Failed to create socket!\n");
    return -1;
  }

  this_saddr.sin_family      = AF_INET;
  this_saddr.sin_addr.s_addr = inet_addr(daq_ip_addr); // dots to long
  this_saddr.sin_port        = htons(5025);

  if(connect( daq_socket, (struct sockaddr *) &this_saddr, sizeof(this_saddr)) < 0)
  {
    printf("Failed to connect\n");
    return -1;
  }

  sprintf(meter_cmd, "*CAL?\n");
  len = strlen(meter_cmd);
  // Start the autocal process
  send(daq_socket, meter_cmd, len, 0);

  memset(meter_reply, 0x00, sizeof(RECV_BUFF_SIZE));
  // Wait for the autocal process to complete
  recv(daq_socket, meter_reply, RECV_BUFF_SIZE, 0);

  // Exit if there's an error with autocal
  if ( strncmp (meter_reply, "+0", 2) )
  {
    printf("Autocalibration did not succeed \n");
    exit(EXIT_FAILURE);
  }
  daq_init = 1;
  return 0;
}

void DAQ::add_channel(int module, int bank, int channel, int is_16_ch)
{
  if(is_16_ch)
  {
    requests.push_back(std::to_string((100 * (module + 1)) + g_daq_offset_bank_channel_16_chn[bank][channel]));
  }
  else
  {
    requests.push_back(std::to_string((100 * (module + 1)) + g_daq_offset_bank_channel_8_chn[bank][channel]));
  }
}

void DAQ::daq_collect_data_multi(int start_index, int end_index)
{
  char meter_reply[RECV_BUFF_SIZE];
  string meter_command;
  char  *ptr = meter_reply;
  int   len, offset, number_of_channels;
  double data;
  static uint32_t accum = 0;
  static uint32_t iterations = 0;
  if (daq_init == 0)
  {
    printf("DAQ is not initialized, exiting the application\n");
    exit(EXIT_FAILURE);
  }

  number_of_channels = end_index - start_index;

  ntime_t start, end;
  int64_t diff;
  ddi_ntime_get_systime(&start);
  
  // Populating the command with specific channels
  meter_command.append("MEASure:VOLTage:DC? 10,.0000050,(@");

  for(int i = start_index; i < end_index; i++)
  {
    meter_command.append(requests.at(i).c_str());
    if( i < (end_index - 1) )
      meter_command.append(",");
  }

  meter_command.append(")\n");
  
  len = strlen(meter_command.c_str());
  send(daq_socket, meter_command.c_str(), len, 0);

  // Get the data from the meter.
  memset(meter_reply, 0x00, sizeof(RECV_BUFF_SIZE));
  recv(daq_socket, meter_reply, RECV_BUFF_SIZE, 0);
  ddi_ntime_get_systime(&end);

  diff = ddi_ntime_diff_ns(&end, &start);
 // printf("DAQ time = %" PRId64 "\n", diff/(1000*1000));
  accum += diff/(1000*1000);
  iterations++;
 
  // Transfer each voltage to the global array.
  for(int i = 0; i < number_of_channels; i++)
  {
    sscanf(ptr, "%lf,%n", &meter_data[i], &offset);
    data=meter_data[i];
    ptr += offset;
  }
  return;
}

void DAQ::clear_meter_data_and_requests()
{
  memset(meter_data, 0x00, sizeof(meter_data));
  requests.clear();
}
