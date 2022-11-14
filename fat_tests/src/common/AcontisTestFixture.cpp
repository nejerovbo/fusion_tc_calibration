#include "AcontisTestFixture.h"
#include "AcontisEnvironment.h"
#include "EnvironmentRegistry.h"
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
#include "math.h"

int is_opmode; // Externally set by ddi_acontis_sdk
int skip_frames;
int ddi_log_level = LOG_LEVEL_VERBOSE;

AcontisTestFixture::AcontisTestFixture(void)
{
  m_test_result = ddi_status_ok;
  m_test_log_fd = NULL;
}

AcontisTestFixture::~AcontisTestFixture(void)
{
  ddi_status_t result;
  result = ddi_sdk_fusion_close(fusion_instance);
  delete cyclic_data;
  if ( m_test_log_fd )
  {
    fclose(m_test_log_fd);
    m_test_log_fd = NULL;
  }
}


void AcontisTestFixture::SetUp(void)
{
  m_AcontisEnvironment = (AcontisEnvironment *)g_environment_registry.GetEnvironment(ACONTIS_ENVIRONMENT_NAME);
}


void AcontisTestFixture::open_fusion(cyclic_func_t *callback)
{
  ddi_status_t result;
  // open Fusion interface, register process data callback
  // TODO: add station address to this function to support multiple instances
  result = ddi_sdk_fusion_open(&fusion_instance, callback);
  ASSERT_EQ(result, ddi_status_ok) << "DDI SDK failed to open result = 0x%x" << result << "\n";
  cyclic_data = new AcontisCyclicData((void*)fusion_instance->slave->pd_input,(void*)fusion_instance->slave->pd_output);
}


void AcontisTestFixture::open_fusion(AcontisCallback *callback)
{
  ddi_status_t result;
  // open Fusion interface, register process data callback
  // TODO: add station address to this function to support multiple instances
  result = ddi_sdk_fusion_open(&fusion_instance, callback);
  ASSERT_EQ(result, ddi_status_ok) << "DDI SDK failed to open result = 0x%x" << result << "\n";
  cyclic_data = new AcontisCyclicData((void*)fusion_instance->slave->pd_input,(void*)fusion_instance->slave->pd_output);
}


void AcontisTestFixture::close_fusion()
{
  ddi_status_t result;
  // open Fusion interface, register process data callback
  // TODO: add station address to this function to support multiple instances
  result = ddi_sdk_fusion_close(fusion_instance);
  ASSERT_EQ(result, ddi_status_ok) << "DDI SDK failed to close result = 0x%x" << result << "\n";
}


// Open a handle to a log file
int AcontisTestFixture::log_open(const char *filename, const char *mode)
{
  ddi_status_t result = ddi_status_err;
  m_test_log_fd = fopen(filename, mode);
  EXPECT_EQ(m_test_log_fd == NULL, false) << "Failed to open log file";
  if ( m_test_log_fd == NULL )
    result = ddi_status_param_err;
  else
    result = ddi_status_ok;
  return result;
}


// Write to a log file, set flush to true to flush the contents to disk
ddi_status_t AcontisTestFixture::log_write(bool flush, const char *msg, ...)
{
  // Validate paramters
  if ((msg == NULL) || (m_test_log_fd == NULL))
    return ddi_status_param_err;
  // log the variable arguments to file and console
  va_list args;
  va_start( args, msg );
  vfprintf( m_test_log_fd, msg, args );
  va_end( args );
  if ( flush )
    fflush(m_test_log_fd);
  return ddi_status_ok;
}


// Write to a log file, set flush to true to flush the contents to disk
void AcontisTestFixture::log_close()
{
  // Close the log interface
  if ( m_test_log_fd )
  {
    m_test_log_fd = NULL;
    fclose(m_test_log_fd);
  }
}

// Return the Fusion part number
void AcontisTestFixture::get_PN(char sn[])
{
  uint32_t result;
  uint8_t buffer[256];
  uint32_t ret_len;
  printf("before upload %d \n", result);
 // Set the EtherCAT master to PREOP
  result = ddi_sdk_ecat_set_master_state(DDI_ECAT_PREOP);
  ASSERT_EQ(result, ddi_status_ok) << "error setting ethercat master state = 0x" << result << "\n";
  // Get the Fusion part number
  result = ecatCoeSdoUpload(fusion_instance->slave->info.dwSlaveId, 0x2000, 1, buffer, 256, &ret_len, 1000, 0);
  printf("result %d \n", result);
  ASSERT_EQ(result, ddi_status_ok) << "ecatCoeSdoUpload \n";
  memcpy(sn, buffer, ret_len);
}

void AcontisTestFixture::go_to_op_mode(const char *project_version)
{
  char msg_buffer[1024];
  char sw_ver[1024];
  uint32_t msg_length;
  uint32_t *module_ptr;
  uint32_t number_of_detected_modules;
  uint32_t count;
  int status;
  ddi_status_t result;

 // Set the EtherCAT master to PREOP
  result = ddi_sdk_ecat_set_master_state(DDI_ECAT_PREOP);
  EXPECT_EQ(result, ddi_status_ok) << "error setting ethercat master state = 0x" << result << "\n";

  // Get the SW version
  result = ddi_sdk_ecat_sdo_msg(fusion_instance->slave, DDI_COE_GET_SW_VERSION, msg_buffer, &msg_length);
  EXPECT_EQ(result, ddi_status_ok) << "ddi_sdk_ecat_sdo_msg error \n";
  memcpy(sw_ver, msg_buffer, strlen(msg_buffer));

  // Get the number of detected modules
  result = ddi_sdk_ecat_sdo_msg(fusion_instance->slave, DDI_COE_GET_NUMBER_OF_DETECTED_MODULES, msg_buffer, &msg_length);
  EXPECT_EQ(result, ddi_status_ok) << "number of detected modules sdo error \n";
  number_of_detected_modules = msg_buffer[0];
  DLOG(" number_of_detected_modules 0x%x \n", number_of_detected_modules);

  // Log the project startup to file and database (if the database is present)
  ddi_sdk_fusion_log_test_startup(fusion_instance, 0, project_version, "InterlockBridge Test");
  ddi_sdk_fusion_log_system_info(fusion_instance, (char*)"log.csv", 0 );
  // clear msg buffer
  memset(msg_buffer,0,sizeof(msg_buffer));

  result = ddi_sdk_ecat_sdo_msg(fusion_instance->slave, DDI_COE_GET_DETECTED_MODULES, msg_buffer, &msg_length);
  EXPECT_EQ(result, ddi_status_ok) << "detected modules sdo error \n";

  printf("\\nn ******* Start Detected Modules ******* \n");
  module_ptr = (uint32_t *)msg_buffer;
  for(int count = 0; count < number_of_detected_modules; count++)
  {
    DLOG(" detected_module slot[0x%x] = 0x%x \n", count, module_ptr[count]);
    module_ptr++;
  }
  DLOG("******* End Detected Modules ******* \n\n");

  DLOG(" Fusion is in OP mode, but process data will only display every 10000 trains by defaut \n \
          Use the -d argument to control how often process data displays \n \
          DOUT toggling is disabled by default as it can cause relays to toggle too fast if they are present  \n\n" );
  DLOG("Software Version: %s\n", sw_ver);

  time_t curtime;
  time(&curtime);

  ddi_sdk_log_error("*******************************\n");
  ddi_sdk_log_error("Op mode transition started at %s", ctime(&curtime));
  ddi_sdk_log_error("Scan rate %d (us) \n", 0);
  ddi_sdk_log_error("Firmware version %s\n", sw_ver);

  // go to OP mode
  result = ddi_sdk_ecat_set_master_state(DDI_ECAT_OP);
  ASSERT_EQ(result, ddi_status_ok) << "error setting ethercat master state = 0x" << result << "\n";
}


void AcontisTestFixture::poll_EtherCAT(uint32_t sec_to_wait, uint32_t poll_rate_ms)
{
  ntime_t current, exit_deadline, poll_deadline;
  int64_t diff_ns, exit_ns;
  ddi_status_t result;
  ddi_ecat_master_stats_t ecat_master_stats;

  ddi_ntime_get_systime(&poll_deadline);
  ddi_ntime_get_systime(&exit_deadline);
  ddi_ntime_add_ns(&poll_deadline,0,poll_rate_ms);
  ddi_ntime_add_ns(&exit_deadline,sec_to_wait,0);
  while ( true )
  {
    //if the amount of lost frames exceeds 15, then exit the program
    result = ddi_sdk_ecat_get_master_stats(&ecat_master_stats);
    EXPECT_EQ(result, ddi_status_ok) << "ddi_sdk_ecat_sdo_msg error \n";
    if (result != ddi_status_ok)
    {
      ddi_sdk_log_error("error getting master state = 0x%x \n", result);
    }

    if (ecat_master_stats.lost_frame_count)
    {
      skip_frames = 5; // Skip the next 5 frames if there's a lost frame
    }

    ASSERT_EQ(ecat_master_stats.lost_frame_count == 0, true) << "lost frames exceeded maximum value \n";
    ddi_ntime_get_systime(&current);

#ifdef DEBUG_NTIME
    printf("Current:\n");
    printf("%d.%09d \n", current.sec, current.ns);
    printf("Exit Deadline:\n");
    printf("%d.%09d \n", exit_deadline.sec, exit_deadline.ns);
#endif
    exit_ns = ddi_ntime_diff_ns(&exit_deadline, &current);
    if ( exit_ns < 0 )
    {
      printf("Returning\n");
      return;
    }
    diff_ns = ddi_ntime_diff_ns(&poll_deadline, &current );
    while (diff_ns > 0 )
    {
      ddi_ntime_get_systime(&current);
      diff_ns = ddi_ntime_diff_ns(&poll_deadline, &current );
      usleep(1);
    }

    if (ecat_master_stats.lost_frame_count >= MAX_LOST_FRAMES_BEFORE_EXIT)
    {
        ddi_sdk_log_error("lost frames exceeded max value of %d\n", MAX_LOST_FRAMES_BEFORE_EXIT);
        ELOG("lost frames exceeded max value of %d \n", MAX_LOST_FRAMES_BEFORE_EXIT);
    }
    usleep(1); // Yield the CPU
  }
}


void AcontisTestFixture::hello_world_demo(void *arg)
{
  ddi_fusion_instance_t *fusion_instance;
  static uint32_t display_count=0;
  int count;
  uint16_t output_value;
  bool output_to_display=0;
  int newline_count=0;
  uint16_t input_value;
  uint16_t input_length;
  uint16_t current_state, requested_state;

  //check parameters
  if (!arg)
    return;

  //set the fusion interface from the argument to the cyclic data function
  fusion_instance = (ddi_fusion_instance_t *)arg;

  ddi_sdk_ecat_get_slave_state(fusion_instance->slave,&current_state,&requested_state);
  //don't process data unless the slave is in OP mode
  if ( current_state != DDI_ECAT_OP)
    return;

  display_count++;
  if( display_count > 1000)
  {
    output_to_display=1;
    display_count=0;
  }
  //send an pattern provided by ddi_sdk_common
  output_value = ddi_sdk_get_next_test_value();
  if (fusion_instance->aout_count)
  {
    //set the aout values according the test pattern
    for(count=0; count < fusion_instance->aout_count; count++)
      //fusion_instance->aouts[count] = output_value;
      ddi_sdk_fusion_set_aout(fusion_instance, count, output_value);
  }
  if (fusion_instance->ain_count)
  {
    //copy over ain values from process data
    if(output_to_display)
    {
      newline_count=0;
      DLOG(" ***** ANALOG IN ***** \n");
      for(count = 0; count < fusion_instance->ain_count; count++)
      {
        newline_count++;
        input_value = ddi_sdk_fusion_get_ain(fusion_instance, count);
        DLOG(" AIN [%04d]= 0x%04x", count, input_value);
        if(newline_count == 7)
        {
          newline_count = 0;
          DLOG("\n");
        }
      }
      DLOG("\n");
   }
  }
  if (fusion_instance->dout_count)
  {
    //send an pattern provided by ddi_sdk_common
    for(count=0; count < fusion_instance->dout_count; count++)
    {
      //dont toggle relays if this happens to be a lowside unit (DOUT modules 2 and 3)
      //if one toggles relay there will be a horrible sound that is caused by the relays
      //switching too fast.

      //uncomment these lines if you want digital output
      if( ( count <= 7 ) )
      //  ddi_sdk_fusion_set_dout8(fusion_instance, count, 0xFF);
      //else
        ddi_sdk_fusion_set_dout16(fusion_instance, count, output_value);
    }
  }
  if (fusion_instance->din_count)
  {
    if(output_to_display)
    {
      newline_count = 0;
      DLOG(" ***** DIGITAL IN ***** \n");
      for(count = 0; count < fusion_instance->din_count; count++)
      {
        newline_count++;
        input_value = ddi_sdk_fusion_get_din(fusion_instance,count,&input_length);
        DLOG(" DIN [%04d]= 0x%04x", count, input_value);
        if(newline_count == 7)
        {
          newline_count = 0;
          DLOG("\n");
        }
      }
      DLOG("\n");
    }
  }
}


//---------------------------------------------------------------------
// Given quanta, the hex value from 0x8000(-10 volts) to 0x7FFF(+10 volts)
// return the voltage.
//---------------------------------------------------------------------
#define COUNT_TO_VOLTS      3276.8f
double AcontisTestFixture::quanta_to_volts(int16_t quanta)
{
  if(0x7FFF == quanta)
  {
    return 10.00;
  }

  return (quanta/COUNT_TO_VOLTS);
}


void AcontisTestFixture::set_sync_mode(ddi_fusion_instance_t *instance)
{
  // Get from environment....
  ethercat_sync_mode_t mode;

  if ( m_AcontisEnvironment->GetEcatSyncMode() == "sync")
    mode = SYNC_MODE_SM_SYNCHRON;
  else
    mode = SYNC_MODE_FREE_RUN;

  set_sync_mode(instance, mode);
}


// Sets sync mode based on what the command line argument value is
void AcontisTestFixture::set_sync_mode(ddi_fusion_instance_t *instance, ethercat_sync_mode_t mode )
{
  uint16_t sync_mode = 0;;
  ddi_status_t result;

  if (mode == SYNC_MODE_FREE_RUN) // Set free run mode for the SM output parameter
    sync_mode = FREE_RUN_SM_SETTING;
  else // Set SM-Synchron for the SM input parameter
    sync_mode = SM_SYNCHRON_INPUT_SM_SETTING;

  // Set the EtherCAT master to PREOP
  result = ddi_sdk_ecat_set_master_state(DDI_ECAT_PREOP);
  EXPECT_EQ(result, ddi_status_ok) << "error setting ethercat master state = 0x" << std::hex << result << "\n";

#ifdef ACONTIS_PROFILE_SDO_ACCESS // Enable this to track performance of the ecatCoeSdoDownload call
  ntime_t perf_start, perf_end;
  ddi_ntime_get_systime(&perf_start);
#endif
  // Set the Synchronization type in the SM output parameter
  result = ecatCoeSdoDownload(instance->slave->info.dwSlaveId, SM_INPUT_SYNC_MANAGER, 1,\
    (uint8_t*)&sync_mode, sizeof(uint16_t), DEFAULT_ACONTIS_TIMEOUT, 0);

  ASSERT_EQ(result, 0) << "error setting SM parameter = 0x" << std::hex << result << "\n";

#ifdef ACONTIS_PROFILE_SDO_ACCESS
  ddi_ntime_get_systime(&perf_end);
  uint64_t diff_ns = ddi_ntime_diff_ns(&perf_end, &perf_start );
  printf("SDO access took %" PRIu64 "\n", diff_ns);
#endif

  if (mode == SYNC_MODE_FREE_RUN)
    // Set free run mode for the SM output paramter
    sync_mode = FREE_RUN_SM_SETTING;
  else
    // Set SM-Synchron for the SM output parameter
    sync_mode = SM_SYNCHRON_OUTPUT_SM_SETTING;

  // Set the Synchronization type in the SM input parameter
  result = ecatCoeSdoDownload(instance->slave->info.dwSlaveId, SM_OUTPUT_SYNC_MANAGER, 1,\
    (uint8_t*)&sync_mode, sizeof(uint16_t), DEFAULT_ACONTIS_TIMEOUT, 0);

  ASSERT_EQ(result, 0) << "error setting SM parameter = 0x" << std::hex << result << "\n";
}


string AcontisTestFixture::get_sync_mode(ddi_fusion_instance_t *instance)
{
  return m_AcontisEnvironment->GetEcatSyncMode();
}

string AcontisTestFixture::get_train_rate()
{
  return m_AcontisEnvironment->GetTrainRate();
}

// Common function to log Sync mode and cyclic data rate to a file based
// on the arguments passed to the test
void AcontisTestFixture::log_csv_header()
{
  // Log the appropriate data to a CSV file
  string sync_mode;
  sync_mode = get_sync_mode(fusion_instance);
  string train_rate;
  train_rate = get_train_rate();
  if ( sync_mode == "sync" )
  {
    AcontisTestFixture::log_write(true, "Sync Mode: SM-Synchron\n");
  }
  else
  {
    AcontisTestFixture::log_write(true, "Sync Mode: Free-Run\n");
  }

  if ( train_rate == "1000" )
  {
    AcontisTestFixture::log_write(true, "Cyclic Data Rate: 1kHz\n");
  }
  else
  {
    AcontisTestFixture::log_write(true, "Cyclic Data Rate: 500Hz\n");
  }
}

// Return the keyboard input. Only the non-blocking version is implemented currently.
ddi_status_t AcontisTestFixture::get_keyboard_input(char *key, bool blocking)
{
  static bool initialized = false;
  static int fd;
  static const char *device = "/dev/tty";
  int bytes;
  if ( initialized == false)
  {
    // Open Keyboard
    fd = open(device, O_RDONLY | O_NONBLOCK);
    if(fd == -1)
    {
        ELOG("ERROR Opening %s\n", device);
        return ddi_status_err;
    }
    initialized = true;
  }
  // Read Keyboard Data
  bytes = read(fd, key, 1);
  if(bytes > 0)
  {
    return ddi_status_ok;
  }
  else
  {
    return ddi_status_err;
  }
}



int   g_this_socket;
//-----------------------------------------------------------------------------
// Connect to the DAQ on the given ip address.  This address can be changed
//  but needs to match what is set on the meter.
//-----------------------------------------------------------------------------
int AcontisTestFixture::connect_to_DAQ(void)
{
  struct  sockaddr_in this_saddr;

  // Create the socket to talk to the DAQ790A
  g_this_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if(-1 == g_this_socket)
  {
    printf("Failed to create socket!\n");
    return -1;
  }

  this_saddr.sin_family      = AF_INET;
  this_saddr.sin_addr.s_addr = inet_addr("169.254.9.70"); // dots to long
  this_saddr.sin_port        = htons(5025);

  if(connect( g_this_socket, (struct sockaddr *) &this_saddr, sizeof(this_saddr)) < 0)
  {
    printf("Failed to connect\n");
    return -1;
  }
  return 0;
}


double g_meter_data[24];
//-----------------------------------------------------------------------------
// Collect analog voltages from channel 1 thru 24 on the DAQ790A
// Store the data in global variables, g_meter_data[0] thru g_meter_data[23]
// This should be called from the main thread, becaue the recv() takes a long
//  time.
// This assumes the connection to the DAQ has already been created.
//-----------------------------------------------------------------------------
void AcontisTestFixture::collect_data_24()
{
  char  meter_cmd[SEND_BUFF_SIZE], meter_reply[RECV_BUFF_SIZE];
  char  *ptr = meter_reply;
  int   len, offset;

  strcpy(meter_cmd, "SENSe:VOLTage:DC:NPLCycles 10,(@101:124)\n");
  len = strlen(meter_cmd);
  send(g_this_socket, meter_cmd, len, 0);
  strcpy(meter_cmd, "MEASure:VOLTage:DC? 10,.000001,(@101:124)\n");
  len = strlen(meter_cmd);
  send(g_this_socket, meter_cmd, len, 0);

  // Get the data from the meter.
  memset(meter_reply, 0x00, sizeof(RECV_BUFF_SIZE));
  recv(g_this_socket, meter_reply, RECV_BUFF_SIZE, 0);
 
 
  // Transfer each voltage to the global array.
  for(int i = 0; i < 24; i++)
  {
    // ptr = atof(g_meter_data[i]);
    sscanf(ptr, "%lf,%n", &g_meter_data[i], &offset);
    // printf("g_meter_data[i] %f \n", g_meter_data[i]);
    ptr += offset;
  }
}


//-----------------------------------------------------------------------------
// Read 1 channel from the DAQ790A.
//-----------------------------------------------------------------------------
float AcontisTestFixture::collect_data_1(int channel)
{
  char  meter_cmd[SEND_BUFF_SIZE], meter_reply[RECV_BUFF_SIZE];
  char  *ptr = meter_reply;
  int   len, offset;
  float meter_data;

  sprintf(meter_cmd, "MEASure:VOLTage:DC? 10,(@1%02d)\n", channel);
  len = strlen(meter_cmd);

  send(g_this_socket, meter_cmd, len, 0);

  // Get the data from the meter.
  memset(meter_reply, 0x00, sizeof(RECV_BUFF_SIZE));

  // check this when I get the module back.
  recv(g_this_socket, meter_reply, RECV_BUFF_SIZE, 0);
  sscanf(ptr, "%f", &meter_data);

  return meter_data;
}

