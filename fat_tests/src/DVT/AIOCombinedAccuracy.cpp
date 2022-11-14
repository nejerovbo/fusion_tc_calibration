// For console output and gflag testing
#include "AcontisTestFixture.h"
#include "CRAMProcessData.h"

using namespace std;


float quanta_to_volts(int16_t quanta);

#define PROJECT_NAME "AnalogInputStepResponse_EtherCAT_test"
#define PROJECT_VERSION "v1.0.0"
#define ONE_SEC_DELAY_MS 1000


#define NUM_VOLTAGES        9
#define NUM_SAMPLES         100

// Defines for Six Slot RIM
#define NUM_CHANNELS_06     8
#define AOUT_OFFSET_06      0     // 0 for 6 slot RIM
#define AIN_OFFSET_06       0     // 0 for 6 slot RIM
#define SLEEP_TIME_06       6     // 6 minutes for 8 channel

// Defines for 12 Slot RIM
#define NUM_CHANNELS_12     16
#define AOUT_OFFSET_12      48   // 48 for 12 slot RIM, 0 for 6 slot RIM
#define AIN_OFFSET_12       68   // 68 for 12 slot RIM, 0 for 6 slot RIM
#define SLEEP_TIME_12       12   // 12 minutes for 16 channel

#define MAX_READ_CYCLES     20   // Number of cycles to skip from writing an 
                                //  Analog Out to Reading an Analog In

AcontisTestFixture *g_fixture;

class AO_IO_ACCURACY : public CallbackTest
{
  public:
    AO_IO_ACCURACY (AcontisTestFixture *p): CallbackTest(p) {}
    virtual void cyclic_method (void*);
};

void AO_IO_ACCURACY::cyclic_method (void *arg)
{
  static FILE *fp1, *fp2;
  static int out_values[NUM_VOLTAGES] = {0x8000, 0xa000, 0xc000, 0xe000, 0x0000, 0x2000, 0x4000, 0x6000, 0x7FFF};
                                     // -10.00v, -7.50v, -5.00v, -2.50v,  0.00v,  2.50v,  5.00v,  7.50v, 10.00v
  static int    channel, cycle_count, sample, voltage_count, execute_state;;
  static int    out_quanta[NUM_VOLTAGES], in_quanta_06[NUM_VOLTAGES][NUM_CHANNELS_06][NUM_SAMPLES], in_quanta_12[NUM_VOLTAGES][NUM_CHANNELS_12][NUM_SAMPLES];
  static float  diff, max_diff_06[NUM_CHANNELS_06], diff_voltage_06[NUM_CHANNELS_06], max_diff_12[NUM_CHANNELS_12], diff_voltage_12[NUM_CHANNELS_12];
  uint16_t      current_state, requested_state, input_value;
  ddi_fusion_instance_t* fusion_instance;
  
  //check parameters
  if (!arg)
    return;

  //set the fusion interface from the argument to the cyclic data function
  fusion_instance = (ddi_fusion_instance_t *)arg;

  ddi_sdk_ecat_get_slave_state(fusion_instance->slave,&current_state,&requested_state);
  //don't process data unless the slave is in OP mode
  if ( current_state != DDI_ECAT_OP)
    return;

 
  // Test starts here
  // I created a state machine to control when I do stuff.
  if (fusion_instance->aout_count)
  {
    // Create the log files, 1 for the 6 slot and 1 for the 12 slot.
    if(0 == execute_state)
    {
      fp1 = fopen("log_aio/aio_log_06.txt", "w");
      fp2 = fopen("log_aio/aio_log_12.txt", "w");
      fprintf(fp1, "Analog Input/Output Combined Accuracy log file, 6 SLOT 8 channels\n");
      fprintf(fp2, "Analog Input/Output Combined Accuracy log file, 12 SLOT 16 channels\n");

      execute_state++;
    }


    // for each voltage
    //   for each sample
    //     on cycle 0, set the output voltage on all channels
    //     on max cycle count, collect the input voltage on all channels
    if(1 == execute_state)
    {
      if (voltage_count < NUM_VOLTAGES)
      {
        if (sample < NUM_SAMPLES)
        {
          if (cycle_count++ == 0)
          {
            for (channel = 0; channel < NUM_CHANNELS_06; channel++)
            {
              ddi_sdk_fusion_set_aout(fusion_instance, channel + AOUT_OFFSET_06, out_values[voltage_count]);
            }
            for (channel = 0; channel < NUM_CHANNELS_12; channel++)
            {
              ddi_sdk_fusion_set_aout(fusion_instance, channel + AOUT_OFFSET_12, out_values[voltage_count]);
            }
          }else 
          if ( cycle_count == MAX_READ_CYCLES )
          {
            for (channel = 0; channel < NUM_CHANNELS_06; channel++)
            {
              input_value = ddi_sdk_fusion_get_ain(fusion_instance, channel + AIN_OFFSET_06);
              in_quanta_06[voltage_count][channel][sample] = input_value;
            }
            for (channel = 0; channel < NUM_CHANNELS_12; channel++)
            {
              input_value = ddi_sdk_fusion_get_ain(fusion_instance, channel + AIN_OFFSET_12);
              in_quanta_12[voltage_count][channel][sample] = input_value;              
            }
            sample++;
            cycle_count = 0;
          }
        }
        else
        {
          sample = 0;
          voltage_count++;
        }
      }
      else
      {
        execute_state++;
      }
    }


    // Now that we've collected the data write it to the log files.
    // Write the voltages, diffs, and keep track of largest diff
    // per channel.
    if(2 == execute_state)
    {  
      fprintf(fp1, "Out volts,");
      for(channel = 0; channel < NUM_CHANNELS_06; channel++)
      {
        fprintf(fp1, "ch %02d, ", channel);
      }
      for(channel = 0; channel < NUM_CHANNELS_06; channel++)
      {
        fprintf(fp1, "dv %02d, ", channel);
      }
      fprintf(fp1, "\n");

      fprintf(fp2, "Out volts,");
      for(channel = 0; channel < NUM_CHANNELS_12; channel++)
      {
        fprintf(fp2, "ch %02d, ", channel);
      }
      for(channel = 0; channel < NUM_CHANNELS_12; channel++)
      {
        fprintf(fp2, "dv %02d, ", channel);
      }
      fprintf(fp2, "\n");


      for(voltage_count = 0; voltage_count < NUM_VOLTAGES; voltage_count++)
      {
        for(sample = 0; sample < NUM_SAMPLES; sample++)
        {
          fprintf(fp1, "%8.4f,", g_fixture->quanta_to_volts(out_values[voltage_count]));
          fprintf(fp2, "%8.4f,", g_fixture->quanta_to_volts(out_values[voltage_count]));
          for(channel = 0; channel < NUM_CHANNELS_06; channel++)
          {
            fprintf(fp1, "%8.4f, ", g_fixture->quanta_to_volts(in_quanta_06[voltage_count][channel][sample]));
          }
          for(channel = 0; channel < NUM_CHANNELS_06; channel++)
          {
            diff = abs(g_fixture->quanta_to_volts(in_quanta_06[voltage_count][channel][sample] - out_values[voltage_count]));
            fprintf(fp1, "%8.4f, ", diff);
            if(diff > max_diff_06[channel])
            {
              max_diff_06[channel] = diff;
              diff_voltage_06[channel] = g_fixture->quanta_to_volts(out_values[voltage_count]);
            }
          }
          fprintf(fp1, "\n");

          for(channel = 0; channel < NUM_CHANNELS_12; channel++)
          {
            fprintf(fp2, "%8.4f, ", g_fixture->quanta_to_volts(in_quanta_12[voltage_count][channel][sample]));
          }
          for(channel = 0; channel < NUM_CHANNELS_12; channel++)
          {
            diff = abs(g_fixture->quanta_to_volts(in_quanta_12[voltage_count][channel][sample] - out_values[voltage_count]));
            fprintf(fp2, "%8.4f, ", diff);
            if(diff > max_diff_12[channel])
            {
              max_diff_12[channel] = diff;
              diff_voltage_12[channel] = g_fixture->quanta_to_volts(out_values[voltage_count]);
            }
          }
          fprintf(fp2, "\n");          
        }
      }

      fprintf(fp1, "\n");
      for(channel = 0; channel < NUM_CHANNELS_06; channel++)
      {
        fprintf(fp1, "max diff ch %3d = %8.4f V at %8.4f V\n", channel, max_diff_06[channel], diff_voltage_06[channel]);
      }

      fprintf(fp2, "\n");
      for(channel = 0; channel < NUM_CHANNELS_12; channel++)
      {
        fprintf(fp2, "max diff ch %3d = %8.4f V at %8.4f V\n", channel, max_diff_12[channel], diff_voltage_12[channel]);
      }
      execute_state++;
    }


    // Close the log files
    if(3 == execute_state)
    {
      fclose(fp1);
      fclose(fp2);
      printf("Done\n"); // This makes it easy to know when the test is done.
      execute_state++;
    }
  }
}
   

// Tests from the original main.cpp under here
// These tests will not make fatal exit yet for testing purposes
TEST_F(AcontisTestFixture, AIOCombinedAccuracy)
{
  AO_IO_ACCURACY callBack(this);
  // Open Fusion Instance
  open_fusion(&callBack);
  ASSERT_EQ(fusion_instance==NULL, false) << "Returned Fusion interface is NULL, exiting\n";

  g_fixture = this;

  // Go to OP mode
  go_to_op_mode(PROJECT_VERSION);

  // Sleep while test runs
  AcontisTestFixture::poll_EtherCAT(40, ONE_SEC_DELAY_MS);
}
