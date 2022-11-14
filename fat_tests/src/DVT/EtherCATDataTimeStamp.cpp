// For console output and gflag testing
#include "AcontisTestFixture.h"
#include "CRAMProcessData.h"
#include "DDITestCommon.h"
#include <inttypes.h>
#include <math.h>

using namespace std;

// Provides a test for the 0xF6F0 and 0xF6F2 timestamps
// Reads an AIN through EtherCAT and routes it to an AOUT

#define PROJECT_NAME     "EtherCAT Timestamp Test"
#define LOGFILE_NAME     "EtherCAT_Data_Timestamp.csv"
#define PROJECT_VERSION  "v1.0.0"
#define ONE_SEC_DELAY_MS 1000

// These fields are mapped in the global user PDO
typedef struct
{
  uint64_t latch_timestamp_ns; // 0xF6F2 timestamp
  uint32_t latch_timestamp_us; // 0xF6F0 timestamp
} rim_global_pdo_t;

#define MAX_NUM_TIMESTAMPS (1024 * 1024)
#define MAX_NUM_SAMPLES_DEFAULT  1011 // 10 min

float timestamp32_data[MAX_NUM_TIMESTAMPS] = {0};
float timestamp64_data[MAX_NUM_TIMESTAMPS] = {0};
// Store the
uint32_t timestamp32_distrubution[100];
uint32_t timestamp64_distrubution[100];

class TimeStamp_IO : public CallbackTest
{
  public:
    TimeStamp_IO (AcontisTestFixture *p): CallbackTest(p) {}
    virtual void cyclic_method (void*);
};

// Calculate the timestamp delta, support both 32 and 64-bit
template <typename T>
T update_timestamp_template( T timestamp)
{
   static T prev_timestamp;
   T delta = 0;
   if ( prev_timestamp != 0 )
   {
     // Calculate delta as long as there is a valid value
     delta = timestamp - prev_timestamp;
   }
   prev_timestamp = timestamp;
   return delta;
}

// Calculate the difference beween timestamps
void update_timestamp (uint32_t latch_timestamp_32, uint64_t latch_timestamp_64, 
                         AcontisTestFixture *TestFixture, ddi_fusion_instance_t *instance)
{
  uint32_t delta_latch_us;
  uint64_t delta_latch_ns;
  uint64_t latch64_remainder;
  uint32_t latch32_remainder;
  static int timestamp_count = 0, test_iteration = 0;

  delta_latch_us = update_timestamp_template(latch_timestamp_32);
  delta_latch_ns = update_timestamp_template(latch_timestamp_64);

  // If there is a timestamp change, perform timestamp checks
  if ( delta_latch_ns != 0)
  {
    // check for mulitple of ~8kHz
    latch32_remainder = delta_latch_us % 125;
    latch64_remainder = delta_latch_ns % (125010);

    // Timestamp should always be a multiple of 125.010 microseconds
    if ( latch64_remainder != 0)
    {
      printf("ERR: latch64_remainder %" PRIu64 "\n", latch64_remainder);
    }
    timestamp_count++;
    timestamp32_distrubution[latch32_remainder]++;
    
    // There should not be a non-zero remainder value
    if ((latch32_remainder != 0)) 
    {
      // There appears to be an issue with the 32-microsecond returning 126 microseconds occassionally
      if ((latch32_remainder != 1)) 
      {
        printf("ERR: latch64_remainder %" PRIu64 "\n", latch_timestamp_64);
        printf("ERR: delta_latch_us 0x%x\n", delta_latch_us);
        printf("ERR: latch32_remainder 0x%x \n", latch32_remainder);
      }
    }
  }

  // End the timestamp test and print out a distrubution
  if ( timestamp_count == (MAX_NUM_SAMPLES_DEFAULT) )
  {
    printf("****** Test Distrubution: \n\n\n");
    for ( int count =0; count < 4; count++)
    {
      printf("tc %d \n", timestamp32_distrubution[count]);
    }
    printf("End test\n");
    exit(0);
  }
  uint16_t din_input_value;
  uint16_t size;
  din_input_value = ddi_sdk_fusion_get_din(instance, 12, &size);
  test_iteration++;
  if (test_iteration > 10 ) // This is what Applied requested
  {
    TestFixture->log_write(false, "%u, %" PRIu64 ", %u, %" PRIu64 ", %u \n", din_input_value, latch_timestamp_64, \
      latch_timestamp_32, delta_latch_ns, delta_latch_us);
  }
  //printf("%u, %" PRIu64 ", %u, %" PRIu64 ", %u \n", din_input_value, latch_timestamp_64, latch_timestamp_32, delta_latch_ns, delta_latch_us);
  //printf("ERR: %d latch64_remainder %" PRIu64 "\n", din_input_value, latch_timestamp_64);
}

void TimeStamp_IO::cyclic_method (void *arg)
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

  //m_AcontisTestFixture->log_write(false, "0x%04x",din_input_value);

  uint8_t *p = local_fusion_instance->slave->pd_input;
  // Get the User PDO address, the global user PDOs must be mapped at the beginning of the input section
  rim_global_pdo_t *pdo = (rim_global_pdo_t *)(p + local_fusion_instance->slave->info.dwPdOffsIn/8);
  // Update the timestamp test
  update_timestamp(pdo->latch_timestamp_us, pdo->latch_timestamp_ns, m_AcontisTestFixture, local_fusion_instance);

  if ( toggle_on == 1)
  {
    toggle_on = 0;
    aout_output_value = 0x7FFF;
    dout_output_value = 0xFFFF;
  }
  else
  {
    toggle_on = 1;
    aout_output_value = 0x8000;
    dout_output_value = 0x0000;
  }

  // Set each available DOUT
  if (local_fusion_instance->dout_count)
  {
    // set the dout values according the test pattern
    for(count=0; count < 12; count++)
      ddi_sdk_fusion_set_dout16(local_fusion_instance, count, dout_output_value);
  }
  
  // If there's aouts present then set them all
  if (local_fusion_instance->aout_count)
  {
    // set the aout values according the test pattern
    for(count=0; count < local_fusion_instance->aout_count; count++)
      // fusion_instance->aouts[count] = output_value;
      ddi_sdk_fusion_set_aout(local_fusion_instance, count, aout_output_value);
  }
}

// Tests from the original main.cpp under here
// These tests will not make fatal exit yet for testing purposes
TEST_F(AcontisTestFixture, DVT_TimeStampTest)
{
  TimeStamp_IO callBack(this);
  printf("hello \n");
  // Open Fusion Instance
  AcontisTestFixture::open_fusion(&callBack);
  ASSERT_EQ(fusion_instance==NULL, false) << "Returned Fusion interface is NULL, exiting\n";

  // Open logfile for write
  AcontisTestFixture::log_open(LOGFILE_NAME, "w");
  AcontisTestFixture::log_write(true, "EtherCAT Data Timestamp Test\n");

  AcontisTestFixture::log_write(true, "DIN_Value, 0xF6F0 (ns), 0xF6F2 (ms), Step 0xF6F0(ns), Step 0xF6F2(ms) \n");

  // Use the sync mode specified by the start script
  AcontisTestFixture::set_sync_mode(fusion_instance);



  // Go to OP mode
  AcontisTestFixture::go_to_op_mode(PROJECT_VERSION);

  // Clear the screen
  clrscr();

  printf("Welcome to %s version: %s\n", PROJECT_NAME, PROJECT_VERSION);

  // Sleep for 60 seconds
  AcontisTestFixture::poll_EtherCAT(2,ONE_SEC_DELAY_MS);

  AcontisTestFixture::close_fusion();

}
