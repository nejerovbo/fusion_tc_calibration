// For console output and gflag testing
#include "AcontisTestFixture.h"
#include "CRAMProcessData.h"

using namespace std;

// Provides a test for measuring the analog input step response
// Sets an AOUT through EtherCAT and reads the AIN through EtherCAT
// The response can be plotted through Excel

#define PROJECT_NAME "AnalogInputStepResponse_EtherCAT_test"
#define PROJECT_VERSION "v1.0.0"
#define ONE_SEC_DELAY_MS 1000

FILE *log_fp;
int test_count = 0;

int ai_step_log_open(const char *filename, const char *mode)
{
  log_fp = fopen(filename, mode);
  if (log_fp == NULL )
  {
   ELOG( "Failed to open log file");
   return -1;
  }
  return 0;
}

float convert_hex_to_volts (int16_t read_value )
{
  return (float)read_value/3276.8f;
}


// Used for testing mutiple callbacks operating in mutiple test fixtures
void update_IO_analog_step_response(void *arg)
{
  static int pattern_count = 0, toggle_on = 1, output_value = 0, count;
  uint16_t current_state, requested_state, input_value;
  ddi_fusion_instance_t * fusion_instance;
  //check parameters
  if (!arg)
    return;

  //set the fusion interface from the argument to the cyclic data function
  fusion_instance = (ddi_fusion_instance_t *)arg;

  ddi_sdk_ecat_get_slave_state(fusion_instance->slave,&current_state,&requested_state);
  //don't process data unless the slave is in OP mode
  if ( current_state != DDI_ECAT_OP)
    return;

  pattern_count++;
  test_count++;
  if ( pattern_count == 100 ) // Toggle the AOUTs at 5 Hz
  {
    pattern_count= 0;
    if ( toggle_on == 1)
    {
      toggle_on = 0;
      output_value = 0x7FFF;
    }
    else
    {
      toggle_on = 1;
      output_value = 0x8000;
    }
  }

  // If there's AOUTS present then set all the AOUTs
  if (fusion_instance->aout_count)
  {
    //set the aout values according the test pattern
    for(count=0; count < fusion_instance->aout_count; count++)
      //fusion_instance->aouts[count] = output_value;
      ddi_sdk_fusion_set_aout(fusion_instance, count, output_value);
  }

  // If there's AIN present then read the first channel to perform a measurement
  if (fusion_instance->ain_count)
  {
    input_value = ddi_sdk_fusion_get_ain(fusion_instance, 0);
    fprintf(log_fp, "%04d, %.03f, %.03f \n", test_count, convert_hex_to_volts(output_value), convert_hex_to_volts(input_value));
    fflush(log_fp);
  }
}

// Tests from the original main.cpp under here
// These tests will not make fatal exit yet for testing purposes
TEST_F(AcontisTestFixture, DVT_AINStepResponse)
{
  // Open Fusion Instance
  open_fusion((cyclic_func_t *)update_IO_analog_step_response);
  ASSERT_EQ(fusion_instance==NULL, false) << "Returned Fusion interface is NULL, exiting\n";

  ai_step_log_open("AnalogInputStepResponse.csv", "w+");
  fprintf(log_fp, "Iteration, AO Value, AI Value\n");
  fflush(log_fp);

  // Go to OP mode
  go_to_op_mode(PROJECT_VERSION);

  // Sleep for 60 seconds
  while ( test_count < 1000)
  {
    usleep(1000);
  }
}
