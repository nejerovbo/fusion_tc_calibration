// For console output and gflag testing
#include "AcontisTestFixture.h"
#include "CRAMProcessData.h"
#include "DDITestCommon.h"
#include <inttypes.h>
#include <math.h>
#include "ddi_defines.h"

using namespace std;

//#define ACONTIS_PROFILE_SDO_ACCESS

// Provide a simple SDO test for the 0x5001 UART SDO

#define PROJECT_NAME     "EtherCAT UART_SDO_Test"
#define LOGFILE_NAME     "EtherCAT_UART_SDO.csv"
#define PROJECT_VERSION  "v1.0.0"
#define ONE_SEC_DELAY_MS 1000

uint8_t  enable_status_debug = 0;
uint8_t  g_cycle_count = 0;
uint8_t  g_disable_compare = 0;
uint16_t g_status_value = 0;
bool     g_enable_match_streaming_mode = true;

class UART_IO : public CallbackTest
{
  public:
    UART_IO (AcontisTestFixture *p): CallbackTest(p) {}
    virtual void cyclic_method (void*);
};

void UART_IO::cyclic_method (void *arg)
{
  uint16_t current_state, requested_state;
  ddi_fusion_instance_t* local_fusion_instance;
  int count;
  static int toggle_on = 0;
  static uint16_t aout_output_value = 0, dout_output_value = 0, din_input_value = 0;

  // Validate parameters
  if (!arg)
    return;

  // set the fusion interface from the argument to the cyclic data function
  local_fusion_instance = (ddi_fusion_instance_t*)arg;

  ddi_sdk_ecat_get_slave_state(local_fusion_instance->slave,&current_state,&requested_state);
  // don't process data unless the slave is in OP mode
  if ( current_state != DDI_ECAT_OP)
    return;

  uint16_t *ctrl = (uint16_t *)&local_fusion_instance->slave->pd_output[39];
  if ( g_cycle_count == 10 )
  {
    //ELOG("Enable hold \n");
    //*ctrl = 1;
  }
  else if ( g_cycle_count == 20 )
  {
    //ELOG("Clear hold \n");
    //*ctrl = 0;
  }

  uint16_t *status = (uint16_t *)&local_fusion_instance->slave->pd_input[39];
  //if ( enable_status_debug )
  //printf("status 0x%x \n", *status);
  g_status_value = *status;
}

#define UART_INFORMATION_SDO 0x5001

PACKED_BEGIN
typedef struct PACKED{
  uint8_t  uart_select;
  uint16_t uart_bytes_rxtx;
  uint8_t  payload[253];
} uart_data_t;
PACKED_END

void rx_UART_SDO_compare (ddi_fusion_instance_t *instance, uint8_t *cmp_str, uint32_t len)
{
  uint8_t data[256] = { 0 };
  uart_data_t *uart_data;
  uint32_t length = 0;
  uint16_t sync_mode = 0;
  int count = 0;
  ddi_status_t result;

#ifdef ACONTIS_PROFILE_SDO_ACCESS // Enable this to track performance of the ecatCoeSdoDownload call
  ntime_t perf_start, perf_end;
  ddi_ntime_get_systime(&perf_start);
#endif
  memset(data,0, sizeof(data));
  uart_data = (uart_data_t *)data;
  // Set the Synchronization type in the SM output parameter
  result = ecatCoeSdoUpload(instance->slave->info.dwSlaveId, UART_INFORMATION_SDO, 2,\
    (uint8_t*)&data, 256, &length, DEFAULT_ACONTIS_TIMEOUT, 0);
#ifdef ACONTIS_PROFILE_SDO_ACCESS    
  ddi_ntime_get_systime(&perf_end);
  uint64_t diff_ns = ddi_ntime_diff_ns(&perf_end, &perf_start );
  printf("Upload SDO access took %" PRIu64 " ns \n", diff_ns);
#endif
  if ( result == 0 )
  {
    bool to_exit = false;
    uart_data = (uart_data_t *)data;
    printf("performing compare \n");
    printf("bytes rx %d \n", uart_data->uart_bytes_rxtx);
    if (uart_data->uart_bytes_rxtx == 0)
    {
      ELOG("0 bytes recieved \n");
      while(1);
      exit(EXIT_SUCCESS);
    }
#ifdef DEBUG_MSG
    printf("length %d \n", length);
    printf("result %d \n", result);
    printf("str %s \n", (char*)uart_data->payload);
    
    printf("uart selected %d \n ", uart_data->uart_select);
#endif
    if ( memcmp((void *)uart_data->payload, (void*)cmp_str, uart_data->uart_bytes_rxtx) != 0 )
    {
      printf("Compare failed \n");
      for (count = 0; count < len; count++)
      {
        if ( uart_data->payload[count] != cmp_str[count] )
        {
          printf("Index[%04d] : rx 0x%04x expected 0x%04x \n", count, uart_data->payload[count], cmp_str[count]);
        }
      }
      printf("Exiting\n");
      while(1)
        sleep(1);
      exit(EXIT_FAILURE);
    }
  }
  else
  {
    printf("error during Upload result 0x%x \n", result);
    exit(EXIT_FAILURE);
  }
}

void UART_RX (ddi_fusion_instance_t *instance , uint8_t *dest_buffer, uint16_t *rx_length )
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
  result = ecatCoeSdoUpload(instance->slave->info.dwSlaveId, UART_INFORMATION_SDO, 2,\
    (uint8_t*)&data, 256, &length, DEFAULT_ACONTIS_TIMEOUT, 0);
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
}

void UART_SDO_tx (ddi_fusion_instance_t *instance)
{
  const int tx_amount = 120;
  int count;
  uint8_t data[128];
  uint16_t sync_mode = 0;
  ddi_status_t result;

  data[0] = 0;
  uint16_t *amount = (uint16_t *)&data[1];
  *amount = tx_amount;

  for (count = 0; count < sizeof(data); count++)
  {
    data[count+3] = rand();
  }

#ifdef ACONTIS_PROFILE_SDO_ACCESS // Enable this to track performance of the ecatCoeSdoDownload call
  ntime_t perf_start, perf_end;
  ddi_ntime_get_systime(&perf_start);
#endif
  // Set the Synchronization type in the SM output parameter
  result = ecatCoeSdoDownload(instance->slave->info.dwSlaveId, UART_INFORMATION_SDO, 2,\
    (uint8_t*)&data, sizeof(data), DEFAULT_ACONTIS_TIMEOUT, 0);

  if ( result != 0 )
  {
    printf("error during Download result 0x%x \n", result);
    exit(EXIT_FAILURE);
  }
  usleep(1000 * 1000);

}

void UART_SDO_loopback (ddi_fusion_instance_t *instance, uint8_t channel)
{
  uint tx_amount = rand()%240;
  int count;
  uint8_t data[512];
  uint16_t sync_mode = 0;
  ddi_status_t result;
  uart_data_t *tx_data = (uart_data_t *)data;
  tx_data->uart_select = channel;
  uint16_t *amount = (uint16_t *)&data[1];
  if ( tx_amount == 0 )
    tx_amount = 1;

  *amount = tx_amount;
  printf("SDO loopback %d bytes \n", tx_amount);
  for (count = 0; count < sizeof(data); count++)
  {
    data[count+3] = rand();
  }

#ifdef ACONTIS_PROFILE_SDO_ACCESS // Enable this to track performance of the ecatCoeSdoDownload call
  ntime_t perf_start, perf_end;
  ddi_ntime_get_systime(&perf_start);
#endif
  // Set the Synchronization type in the SM output parameter
  result = ecatCoeSdoDownload(instance->slave->info.dwSlaveId, UART_INFORMATION_SDO, 2,\
    (uint8_t*)&data, tx_amount+sizeof(uart_data_t), DEFAULT_ACONTIS_TIMEOUT, 0);
#ifdef ACONTIS_PROFILE_SDO_ACCESS    
  ddi_ntime_get_systime(&perf_end);
  uint64_t diff_ns = ddi_ntime_diff_ns(&perf_end, &perf_start );
  printf("UART write took %" PRIu64 "\n", diff_ns);
#endif

  if ( result != 0 )
  {
    printf("error during Download result 0x%x \n", result);
    exit(EXIT_FAILURE);
  }
  // Give time for loopback to propagate
  usleep((tx_amount + 100) * 1000);
  //usleep(1000 * 100);
  rx_UART_SDO_compare(instance, &data[3], tx_amount);

}

// Start or stop streaming mode
ddi_status_t UART_tx_match(ddi_fusion_instance_t *instance, bool start_streaming)
{
  uint8_t data[512];
  uint16_t sync_mode = 0;
  ddi_status_t result;
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
  result = ecatCoeSdoDownload(instance->slave->info.dwSlaveId, UART_INFORMATION_SDO, 2,\
    (uint8_t*)&data, 10, DEFAULT_ACONTIS_TIMEOUT, 0);
  //printf("***SDO tx end\n");
#ifdef ACONTIS_PROFILE_SDO_ACCESS    
  ddi_ntime_get_systime(&perf_end);
  uint64_t diff_ns = ddi_ntime_diff_ns(&perf_end, &perf_start );
  printf("UART write took %" PRIu64 "\n", diff_ns);
#endif

  return ddi_status_ok;
}

#define MS_100 ( NSEC_PER_MSEC * 100 )

#define STATUS_STARTED 1
#define STATUS_STOPPED 2
#define STATUS_PAUSED  3

void UART_SDO_impedance_match (AcontisTestFixture *tf_obj, ddi_fusion_instance_t *instance)
{
  const int tx_amount = 3;
  int count;
  uint8_t rx_data[512];
  int streaming_status = STATUS_STARTED;

  UART_tx_match(instance, true);

  ntime_t poll_deadline;
  ntime_t current_time;
  int64_t poll_result;
  ddi_ntime_get_systime(&poll_deadline);
  uint16_t rx_bytes;

  printf(BLUE "UART Center Match Demonstration Application\n");
  printf(BLUE "-------------------------------------------\n");

  printf("\033[%d;%dH", 4, 0);
  printf("\33[2K");
  printf(GREEN "Status: Streaming" CLEAR "\n");

  // Poll in 100 milliseconds from now
  ddi_ntime_add_ns(&poll_deadline,0 , MS_100 );
  while ( 1 )
  {
    ddi_ntime_get_systime(&current_time);
    // If diff is negative then the deadline has expired
    poll_result = ddi_ntime_diff_ns(&poll_deadline, &current_time);
    if ( poll_result < 0 )
    {
      ddi_status_t result;
      char key;
      result = tf_obj->get_keyboard_input(&key);
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
          ASSERT_EQ(UART_tx_match(instance, true), ddi_status_ok);
          streaming_status = STATUS_STARTED;
        }
        else if ( key == 'o')
        {
          printf("\033[%d;%dH", 4, 0);
          printf("\33[2K");
          printf(YELLOW "Status: Stopped" CLEAR "\n");
          // Stop streaming with 'SP' command
          ASSERT_EQ(UART_tx_match(instance, false), ddi_status_ok);
          streaming_status = STATUS_STOPPED;
        }
      }
      printf("\033[%d;%dH", 5, 0);
      printf(CYAN "Status[0] = 0x%04x" CLEAR "\n", g_status_value);
      //printf("g_status_value = 0x%x \n", g_status_value);
      if ( g_status_value >= 0x37)
      {
        if ( streaming_status != STATUS_PAUSED)
          UART_RX(instance, rx_data, &rx_bytes);
        //printf("%d Bytes Received \n", rx_bytes);
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
        // Schedule for 100 milliseconds from the last polling deadline
        ddi_ntime_add_ns(&poll_deadline,0, MS_100);

        //exit(0);
      }
    }
  }
}

void UART_SDO_tenma_handshake (ddi_fusion_instance_t *instance, uint8_t channel, int continuous)
{
  int count;
  uint8_t data[512];
  uint16_t sync_mode = 0;
  static int test_count = 0;
  ddi_status_t result;
  uart_data_t *tx_data = (uart_data_t *)data;
  tx_data->uart_select = channel;
  tx_data->uart_bytes_rxtx = 5;
  //clrscr();
  printf("\033[%d;%dH", 0, 0);
  printf(ORANGE "***UART_SDO Handshake test\n" CLEAR);
  printf(YELLOW "***channel %d\n" CLEAR, channel);
  printf(VIOLET "Iteration %08d \n", test_count);
  test_count++;

  tx_data->payload[0] = '*';
  tx_data->payload[1] = 'I';
  tx_data->payload[2] = 'D';
  tx_data->payload[3] = 'N';
  tx_data->payload[4] = '?';

#ifdef ACONTIS_PROFILE_SDO_ACCESS // Enable this to track performance of the ecatCoeSdoDownload call
  ntime_t perf_start, perf_end;
  ddi_ntime_get_systime(&perf_start);
#endif
  // Set the Synchronization type in the SM output parameter
  result = ecatCoeSdoDownload(instance->slave->info.dwSlaveId, UART_INFORMATION_SDO, 2,\
    (uint8_t*)&data, 9, DEFAULT_ACONTIS_TIMEOUT, 0);
#ifdef ACONTIS_PROFILE_SDO_ACCESS    
  ddi_ntime_get_systime(&perf_end);
  uint64_t diff_ns = ddi_ntime_diff_ns(&perf_end, &perf_start );
  printf("UART write took %" PRIu64 "\n", diff_ns);
#endif

  if ( result != 0 )
  {
    printf("error during Download result 0x%x \n", result);
    exit(EXIT_FAILURE);
  }

  ntime_t poll_deadline;
  ntime_t current_time;
  int64_t poll_result;
  ddi_ntime_get_systime(&poll_deadline);
  uint16_t rx_bytes;

  // Poll in 100 milliseconds from now
  ddi_ntime_add_ns(&poll_deadline,0, MS_100 );
  while ( 1 )
  {
    ddi_ntime_get_systime(&current_time);
    // If diff is negative then the deadline has expired
    poll_result = ddi_ntime_diff_ns(&poll_deadline, &current_time);
    if ( poll_result < 0 )
    {
      //printf("g_status_value = 0x%x \n", g_status_value);
      if ( g_status_value >= 18 )
      {
        memset(data,0, sizeof(data));
        UART_RX(instance, data, &rx_bytes);
        if ( rx_bytes != 0 )
        {
          printf(CYAN "*** Raw Bytes Received From the Tenma Unit: " CLEAR "\n");
          int count = 0;
          for ( count = 0; count < rx_bytes; count++)
          {
            printf(BLUE "RxData[%04d] = 0x%02x " CLEAR" \n", count, data[count]);
          }
          printf(VIOLET "Rx String from the Tenma Unit = " GREEN  "%s " CLEAR "\n", (char *)data);
          if ( strcmp ((char*)data, (char*)"TENMA 72-2705 V2.5") != 0 )
          {
            ELOG("Rx string %s\n", data);
            ELOG("Ex string %s\n", (char *)"TENMA 72-2705 V2.5");
            while(1);
          }
          return;
        }
        // Schedule for 100 milliseconds from the last polling deadline
        ddi_ntime_add_ns(&poll_deadline,0, MS_100);
      }
    }

  }
}

void UART_SDO_tenma_on_off (ddi_fusion_instance_t *instance, bool on)
{
  const int tx_amount = 3;
  int count;
  uint8_t data[512];
  uint16_t sync_mode = 0;
  ddi_status_t result;
  uart_data_t *tx_data = (uart_data_t *)data;
  tx_data->uart_select = 0;
  //*amount = tx_amount;
  tx_data->uart_bytes_rxtx = 4;
  //printf("***UART_SDO Tenma \n");

  if ( on )
  {
    tx_data->payload[0] = 'O';
    tx_data->payload[1] = 'U';
    tx_data->payload[2] = 'T';
    tx_data->payload[3] = '1';
  }
  else
  {
    tx_data->payload[0] = 'O';
    tx_data->payload[1] = 'U';
    tx_data->payload[2] = 'T';
    tx_data->payload[3] = '0';
  }
  //tx_data->payload[4] = ' ';

#ifdef ACONTIS_PROFILE_SDO_ACCESS // Enable this to track performance of the ecatCoeSdoDownload call
  ntime_t perf_start, perf_end;
  ddi_ntime_get_systime(&perf_start);
#endif
  // Set the Synchronization type in the SM output parameter
  result = ecatCoeSdoDownload(instance->slave->info.dwSlaveId, UART_INFORMATION_SDO, 2,\
    (uint8_t*)&data, 7, DEFAULT_ACONTIS_TIMEOUT, 0);
#ifdef ACONTIS_PROFILE_SDO_ACCESS    
  ddi_ntime_get_systime(&perf_end);
  uint64_t diff_ns = ddi_ntime_diff_ns(&perf_end, &perf_start );
  printf("UART write took %" PRIu64 "\n", diff_ns);
#endif

  if ( result != 0 )
  {
    printf("error during Download result 0x%x \n", result);
    exit(EXIT_FAILURE);
  }
}

void UART_channel_select (ddi_fusion_instance_t *instance, uint8_t channel)
{
  int count;
  uint32_t length;
  uint8_t data[1];
  uint8_t verify[1];
  uint16_t sync_mode = 0;
  ddi_status_t result;

  data[0] = channel;

#ifdef ACONTIS_PROFILE_SDO_ACCESS // Enable this to track performance of the ecatCoeSdoDownload call
  ntime_t perf_start, perf_end;
  ddi_ntime_get_systime(&perf_start);
#endif
  // Set the UART channel select
  result = ecatCoeSdoDownload(instance->slave->info.dwSlaveId, UART_INFORMATION_SDO, 1,\
    (uint8_t*)&data, sizeof(data), DEFAULT_ACONTIS_TIMEOUT, 0);
#ifdef ACONTIS_PROFILE_SDO_ACCESS    
  ddi_ntime_get_systime(&perf_end);
  uint64_t diff_ns = ddi_ntime_diff_ns(&perf_end, &perf_start );
  printf("UART select took %" PRIu64 "ns \n", diff_ns);
#endif

  ASSERT_EQ(result, 0 ) << "SDO Download failed";

  // Verify the channel select matches the set value
  result = ecatCoeSdoUpload(instance->slave->info.dwSlaveId, UART_INFORMATION_SDO, 1,\
    (uint8_t*)&verify, sizeof(verify), &length, DEFAULT_ACONTIS_TIMEOUT, 0);

  ASSERT_EQ(result, 0 ) << "SDO Upload failed";

  ASSERT_EQ(verify[0], channel ) << "Channel Verification Failed";  

}

#define CTRL_FLUSH 0x2
#define CTRL_HOLD  0x1

void UART_channel_flush (ddi_fusion_instance_t *instance)
{
  int count;
  uint32_t length;
  uint16_t data[1];
  uint16_t verify[1];
  uint16_t sync_mode = 0;
  ddi_status_t result;

  data[0] = CTRL_FLUSH;

  // Set the UART channel select
  result = ecatCoeSdoDownload(instance->slave->info.dwSlaveId, UART_INFORMATION_SDO, 3,\
    (uint8_t*)&data, sizeof(data), DEFAULT_ACONTIS_TIMEOUT, 0);

  ASSERT_EQ(result, 0 ) << "SDO Download failed";
}

void UART_channel_hold (ddi_fusion_instance_t *instance, bool hold)
{
  int count;
  uint32_t length;
  uint16_t data[1];
  uint16_t verify[1];
  uint16_t sync_mode = 0;
  ddi_status_t result;

  if ( hold )
    data[0] = CTRL_HOLD;
  else
    data[0] &= ~CTRL_HOLD;

  // Set the UART channel select
  result = ecatCoeSdoDownload(instance->slave->info.dwSlaveId, UART_INFORMATION_SDO, 3,\
    (uint8_t*)&data, sizeof(data), DEFAULT_ACONTIS_TIMEOUT, 0);

  ASSERT_EQ(result, 0 ) << "SDO Download failed";
}

// Preop-mode only
void UART_select_baud (ddi_fusion_instance_t *instance, const char *baud)
{
  int result;
  printf("UART: Selecting baud rate  %s \n", baud);

  // Set the UART baud rate
  result = ecatCoeSdoDownload(instance->slave->info.dwSlaveId, UART_INFORMATION_SDO, 6,\
    (uint8_t*)baud, strlen(baud), DEFAULT_ACONTIS_TIMEOUT, 0);

  ASSERT_EQ(result, 0 ) << "SDO Download failed";
}

const char *baud_rates[] = {"9600", "38400", "115200","9600"};

void UART_test_baud (ddi_fusion_instance_t *instance)
{
  int count;
  uint32_t length;
  uint16_t data[1];
  char verified_baud[10];
  uint16_t sync_mode = 0;
  ddi_status_t result;
  int entries = ARRAY_ELEMENTS(baud_rates);

  // Test the entries in the baud rate table
  for ( count = 0; count < entries; count++)
  {
    // Set the Baud Rate
    UART_select_baud (instance, baud_rates[count]);
    // Set the UART channel select
    result = ecatCoeSdoUpload(instance->slave->info.dwSlaveId, UART_INFORMATION_SDO, 6,\
      (uint8_t*)verified_baud, 6, &length, DEFAULT_ACONTIS_TIMEOUT, 0);

    ASSERT_EQ(result, 0 ) << "SDO Upload failed";
    ASSERT_EQ(strncmp (baud_rates[count], verified_baud, strlen(baud_rates[count])),0 ) << "Baud Verification failed";
  }
}

// Tests basic UART SDO functionality
TEST_F(AcontisTestFixture, UART_SDO)
{
  ddi_status_t result;
  printf("UART SDO test \n");
  UART_IO callBack(this);
  // Open Fusion Instance
  AcontisTestFixture::open_fusion(&callBack);
  ASSERT_EQ(fusion_instance==NULL, false) << "Returned Fusion interface is NULL, exiting\n";

  // Open logfile for write
  AcontisTestFixture::log_open(LOGFILE_NAME, "w");
  AcontisTestFixture::log_write(true, "UART SDO Test\n");

  // Use the sync mode specified by the start script
  // AcontisTestFixture::set_sync_mode(fusion_instance);

  // Set the EtherCAT master to PREOP
  result = ddi_sdk_ecat_set_master_state(DDI_ECAT_PREOP);
  EXPECT_EQ(result, ddi_status_ok) << "error setting ethercat master state = 0x" << std::hex << result << "\n";

  //UART_channel_select(fusion_instance, 0);
  UART_test_baud(fusion_instance);

  // Go to OP mode
  AcontisTestFixture::go_to_op_mode(PROJECT_VERSION);

  // Clear the screen
  clrscr();

  printf("Welcome to %s version: %s\n", PROJECT_NAME, PROJECT_VERSION);

  UART_channel_select(fusion_instance, 0);
  UART_channel_flush(fusion_instance);
  AcontisTestFixture::close_fusion();

}

// Tests UART functionality
TEST_F(AcontisTestFixture, UART_Loopback)
{
  ddi_status_t result;
  printf("UART test \n");
  UART_IO callBack(this);
  // Open Fusion Instance
  AcontisTestFixture::open_fusion(&callBack);
  ASSERT_EQ(fusion_instance==NULL, false) << "Returned Fusion interface is NULL, exiting\n";

  // Open logfile for write
  AcontisTestFixture::log_open(LOGFILE_NAME, "w");
  AcontisTestFixture::log_write(true, "UART SDO Test\n");

  // Use the sync mode specified by the start script
  AcontisTestFixture::set_sync_mode(fusion_instance);

  // Set the EtherCAT master to PREOP
  result = ddi_sdk_ecat_set_master_state(DDI_ECAT_PREOP);
  EXPECT_EQ(result, ddi_status_ok) << "error setting ethercat master state = 0x" << std::hex << result << "\n";

  UART_select_baud(fusion_instance, "9600");

  // Go to OP mode
  AcontisTestFixture::go_to_op_mode(PROJECT_VERSION);

  // Clear the screen
  clrscr();

  printf("Welcome to %s version: %s\n", PROJECT_NAME, PROJECT_VERSION);
  uint8_t uart_channel = 1;

  UART_channel_select(fusion_instance, uart_channel);
  UART_channel_flush(fusion_instance);

  ntime_t exit_deadline;
  ntime_t current_time;
  int64_t exit_ns;
  ddi_ntime_get_systime(&exit_deadline);

  // Exit in 500 seconds from now
  ddi_ntime_add_ns(&exit_deadline,500,0);
  while ( 1 )
  {
    ddi_ntime_get_systime(&current_time);
    // If diff is negative then the deadline has expired
    exit_ns = ddi_ntime_diff_ns(&exit_deadline, &current_time);
    if ( exit_ns < 0 )
    {
      printf("Timer expired - Exiting \n");
      break;
    }
    // Test the loopback mechansim
    UART_SDO_loopback(fusion_instance,uart_channel);
  }

  AcontisTestFixture::close_fusion();
}

// Tests UART functionality with Mattson Impedance Match
TEST_F(AcontisTestFixture, UART_Match)
{
  ddi_status_t result;
  printf("UART test \n");
  UART_IO callBack(this);
  // Open Fusion Instance
  AcontisTestFixture::open_fusion(&callBack);
  ASSERT_EQ(fusion_instance==NULL, false) << "Returned Fusion interface is NULL, exiting\n";

  // Open logfile for write
  AcontisTestFixture::log_open(LOGFILE_NAME, "w");
  AcontisTestFixture::log_write(true, "UART SDO Test\n");

  // Use the sync mode specified by the start script
  AcontisTestFixture::set_sync_mode(fusion_instance);

  // Set the EtherCAT master to PREOP
  result = ddi_sdk_ecat_set_master_state(DDI_ECAT_PREOP);
  EXPECT_EQ(result, ddi_status_ok) << "error setting ethercat master state = 0x" << std::hex << result << "\n";

  UART_select_baud(fusion_instance, "9600");

  // Go to OP mode
  AcontisTestFixture::go_to_op_mode(PROJECT_VERSION);

  // Clear the screen
  clrscr();

  printf("Welcome to %s version: %s\n", PROJECT_NAME, PROJECT_VERSION);

  // Select UART channel 0
  UART_channel_select(fusion_instance, 0);
  UART_channel_flush(fusion_instance);

  while ( 1 )
  {
    UART_SDO_impedance_match(this,fusion_instance);
    sleep(1);
  }

  AcontisTestFixture::close_fusion();

}

// Tests UART functionality with Tenma Serial
TEST_F(AcontisTestFixture, UART_Single_Tenma)
{
  ddi_status_t result;
  printf("UART Tenma test \n");
  UART_IO callBack(this);
  // Open Fusion Instance
  AcontisTestFixture::open_fusion(&callBack);
  ASSERT_EQ(fusion_instance==NULL, false) << "Returned Fusion interface is NULL, exiting\n";

  // Open logfile for write
  AcontisTestFixture::log_open(LOGFILE_NAME, "w");
  AcontisTestFixture::log_write(true, "UART SDO Test\n");

  // Use the sync mode specified by the start script
  AcontisTestFixture::set_sync_mode(fusion_instance);

  // Set the EtherCAT master to PREOP
  result = ddi_sdk_ecat_set_master_state(DDI_ECAT_PREOP);
  EXPECT_EQ(result, ddi_status_ok) << "error setting ethercat master state = 0x" << std::hex << result << "\n";

  UART_select_baud(fusion_instance, "9600");

  // Go to OP mode
  AcontisTestFixture::go_to_op_mode(PROJECT_VERSION);

  // Clear the screen
  clrscr();

  int channel = 0;

  printf("Welcome to %s version: %s\n", PROJECT_NAME, PROJECT_VERSION);

  // Select UART channel
  UART_channel_select(fusion_instance, channel);
  UART_channel_flush(fusion_instance);

  //UART_SDO_tenma_on_off(fusion_instance, false);
  //sleep(5);
  //UART_SDO_tenma_on_off(fusion_instance, true);
  //sleep(5);

  clrscr();

  while ( 1 )
  {
    UART_SDO_tenma_handshake(fusion_instance, channel,0);
    usleep(30 * 1000);
  }
  AcontisTestFixture::close_fusion();

}

// Tests UART functionality with Tenma Serial
TEST_F(AcontisTestFixture, UART_Tenma)
{
  ddi_status_t result;
  printf("UART Multiple Tenma test \n");
  UART_IO callBack(this);
  // Open Fusion Instance
  AcontisTestFixture::open_fusion(&callBack);
  ASSERT_EQ(fusion_instance==NULL, false) << "Returned Fusion interface is NULL, exiting\n";

  // Open logfile for write
  AcontisTestFixture::log_open(LOGFILE_NAME, "w");
  AcontisTestFixture::log_write(true, "UART SDO Test\n");

  // Use the sync mode specified by the start script
  AcontisTestFixture::set_sync_mode(fusion_instance);

  // Set the EtherCAT master to PREOP
  result = ddi_sdk_ecat_set_master_state(DDI_ECAT_PREOP);
  EXPECT_EQ(result, ddi_status_ok) << "error setting ethercat master state = 0x" << std::hex << result << "\n";

  UART_select_baud(fusion_instance, "9600");

  // Go to OP mode
  AcontisTestFixture::go_to_op_mode(PROJECT_VERSION);

  // Clear the screen
  clrscr();

  int channel = 0;

  printf("Welcome to %s version: %s\n", PROJECT_NAME, PROJECT_VERSION);

  for ( channel = 0 ; channel < 1; channel++)
  {
    // Select UART channel
    UART_channel_select(fusion_instance, channel);
    UART_channel_flush(fusion_instance);
  }

  //UART_SDO_tenma_on_off(fusion_instance, false);
  //sleep(5);
  //UART_SDO_tenma_on_off(fusion_instance, true);
  //sleep(5);

  clrscr();

  int iteration = 0;
  while ( 1 )
  {
    iteration++;
    int count;
    for ( channel = 0 ; channel < 1; channel++)
    {
      UART_channel_select(fusion_instance, channel);
      UART_SDO_tenma_handshake(fusion_instance, channel,false);
      usleep(30 * 1000);
    }
    if ( iteration == 200 )
    {
      break;
    }
  }
  AcontisTestFixture::close_fusion();

}

