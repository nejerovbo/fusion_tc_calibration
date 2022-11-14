// For console output and gflag testing
#include "AcontisTestFixture.h"
#include "CRAMProcessData.h"
#include "sys/socket.h"
#include "arpa/inet.h"
#include "stdio.h"
#include "stdbool.h"

using namespace std;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

extern int   collect_data_24();
extern int   g_this_socket;
extern float g_meter_data[24];

// Flags to keep track of what is going on with the data collection.
// g_dc_active is a flag from the cyclic to the main thread that we are still doing stuff,
// g_dc_go_collect is a flag from the cyclic thread to the main thread to go collect data.
//    the cyclic thread sets it and the main thread clears it after it collects the data.
// g_dc_data_collected is a flag from the main thread to the cyclic thread to tell the cyclic
//    thread the data has been collected.  The Main thread sets it and the cyclic thread
//    clears it when it starts recording the data.
// start_read is a flag for the cyclic thread to only initiate data collection once, 
//    the rest of the time it polls g_dc_data_collected waiting for completion.
bool  g_dc_active = false;
bool  g_dc_go_collect = false;
bool  g_dc_data_collected = false;

int g_meter_channel_x8;
int g_meter_channel_x16;

#pragma GCC diagnostic pop

#define PROJECT_NAME "AnalogOutputDCCoupling_EtherCAT_test"
#define PROJECT_VERSION "v1.0.0"
#define ONE_SEC_DELAY_MS 1000


#define NUM_CHANNELS_x8       8
#define AOUT_OFFSET_x8        0     // 0 for 6 slot RIM
#define AIN_OFFSET_x8         0     // 0 for 6 slot RIM

#define NUM_CHANNELS_x16     16
#define AOUT_OFFSET_x16      48   // 48 for 12 slot RIM, 0 for 6 slot RIM
#define AIN_OFFSET_x16       68   // 68 for 12 slot RIM, 0 for 6 slot RIM


#define NUM_VOLTAGES         2


// Cycle count definitions
#define WRITE_CYCLE         0
#define READ_CYCLE          100   // Start reading N cycles after the write
#define NUM_SAMPLES         10
                                

#define PLUS_SIX_QUANTA     ((uint16_t)(6 * 32768/10))
#define MINUS_SIX_QUANTA    ((uint16_t)(-6 * 32768/10))
#define PLUS_TEN_QUANTA     0x7FFF
#define MINUS_TEN_QUANTA    0x8000

extern AcontisTestFixture *g_fixture;

class AO_DC_COUPLING : public CallbackTest
{
  public:
    AO_DC_COUPLING (AcontisTestFixture *p): CallbackTest(p) {}
    virtual void cyclic_method (void*);
};

void AO_DC_COUPLING::cyclic_method (void *arg)
{
  static FILE   *fp1, *fp2;
  static int    execute_state, channel, toggle_quanta, test_channel, voltage_count, cycle_count, sample_count;
  static bool   start_read = true;
  static float  in_voltage_x8[NUM_VOLTAGES][NUM_CHANNELS_x8][NUM_SAMPLES], in_voltage_x16[NUM_VOLTAGES][NUM_CHANNELS_x16][NUM_SAMPLES];
  static int    out_values[NUM_VOLTAGES] = {PLUS_SIX_QUANTA, MINUS_SIX_QUANTA};  
  static float  diff, max_diff_x8[NUM_CHANNELS_x8], diff_voltage_x8[NUM_CHANNELS_x8], max_diff_x16[NUM_CHANNELS_x16], diff_voltage_x16[NUM_CHANNELS_x16];

  // Map from Fusion I/O channels to DAQ790A meter channels
  static const  int meter_channel_x8[8] = {1,2,3,4,5,6,7,8};
  static const  int meter_channel_x16[16] = {9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24};

  uint16_t current_state, requested_state;
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

  if (fusion_instance->aout_count)
  {
    // Create the log file.
    if(0 == execute_state)
    {
      fp1 = fopen("log_aout/dc_coup_x8.csv", "w");
      fprintf(fp1, "Analog Output DC Coupling x8\n\n");
      fp2 = fopen("log_aout/dc_coup_x16.csv", "w");
      fprintf(fp2, "Analog Output DC Coupling x16\n\n");

      execute_state++;
    }

    // For this test we hold 1 channel to a steady voltage, eiher +6 or -6 volts.
    // We then toggle all of the other channels, rail-to-rail, every cycle.
    // While this is going on we read the input on the steady channel and look for deltas.
    if(1 == execute_state)
    {
      if(voltage_count < NUM_VOLTAGES)
      {
        // Toggle from rail-to-rail every cycle except for the test_channel
        if(PLUS_TEN_QUANTA == toggle_quanta)
        {
          toggle_quanta = MINUS_TEN_QUANTA;
        }
        else
        {
          toggle_quanta = PLUS_TEN_QUANTA;
        }

        for(channel = 0; channel < NUM_CHANNELS_x8; channel++)
        {
          if(channel != test_channel)
          {
            ddi_sdk_fusion_set_aout(fusion_instance, channel + AOUT_OFFSET_x8, toggle_quanta);
          }
        }
        for(channel = 0; channel < NUM_CHANNELS_x16; channel++)
        {
          if(channel != test_channel)
          {
            ddi_sdk_fusion_set_aout(fusion_instance, channel + AOUT_OFFSET_x16, toggle_quanta);
          }
        } 

        if(test_channel < NUM_CHANNELS_x16)
        {
          if(WRITE_CYCLE == cycle_count)
          {
            if(test_channel < NUM_CHANNELS_x8)
            { // Write a known voltage to the test channel
                ddi_sdk_fusion_set_aout(fusion_instance, test_channel + AOUT_OFFSET_x8, out_values[voltage_count]);
            }
            ddi_sdk_fusion_set_aout(fusion_instance, test_channel + AOUT_OFFSET_x16, out_values[voltage_count]);
          }
          else if(READ_CYCLE == cycle_count)
          { // Delay for a while, then start reading
            if(sample_count < NUM_SAMPLES)
            {
              if(true == start_read)  // Only initiate the data collection once per matching cycle count
              {
                if(test_channel < NUM_CHANNELS_x8)
                {
                  g_meter_channel_x8 = meter_channel_x8[test_channel];
                }
                g_meter_channel_x16 = meter_channel_x16[test_channel];
                g_dc_go_collect = true;
                start_read = false;
              }

              if ( g_dc_data_collected == false) // Exit if the read hasn't completed
              {
                return;
              }
              g_dc_data_collected = false;
              start_read = true;            // Reset for next time we need to start collecting
              // Record the test data
              if(test_channel < NUM_CHANNELS_x8)
              {
                in_voltage_x8[voltage_count][test_channel][sample_count] = g_meter_data[0];
              }
              in_voltage_x16[voltage_count][test_channel][sample_count] = g_meter_data[1];

              sample_count++;
              return;
            }
            else
            { // We've finished all the samples, go to next test channel
              sample_count = 0;
              cycle_count = 0;
              test_channel++;
              return;
            }
          }
          // cycle_count only counts up from 0 to READ_CYCLE
          //  it stays at this value until all of the samples are collectd.
          cycle_count++;

        }
        else
        {
          test_channel = 0;
          voltage_count++;
        }
      }
      else
      {
        execute_state++;
      }
    }


    // We have the data, now send it to the log files
    if(2 == execute_state)
    {
      // Output for x8
      for(voltage_count = 0; voltage_count < NUM_VOLTAGES; voltage_count++)
      {
        fprintf(fp1, "Voltage = %8.4f", g_fixture->quanta_to_volts(out_values[voltage_count]));
        for(channel = 0; channel < NUM_CHANNELS_x8; channel++)
        {
          fprintf(fp1, ",ch = %3d", channel);
        }

        for(channel = 0; channel < NUM_CHANNELS_x8; channel++)
        {
          fprintf(fp1, ",dv %3d", channel);
        }
        fprintf(fp1, "\n");

  
        for(sample_count = 0; sample_count < NUM_SAMPLES; sample_count++)
        {
          fprintf(fp1, "                sample = %3d", sample_count);
          for(channel = 0; channel < NUM_CHANNELS_x8; channel++)
          {
            fprintf(fp1, ",%8.4f", in_voltage_x8[voltage_count][channel][sample_count]);
          }

          for(channel = 0; channel < NUM_CHANNELS_x8; channel++)
          {
            diff = abs(in_voltage_x8[voltage_count][channel][sample_count] - g_fixture->quanta_to_volts(out_values[voltage_count]));
            fprintf(fp1, ",%8.4f", diff);

            if(diff > max_diff_x8[channel])
            {
              max_diff_x8[channel] = diff;
              diff_voltage_x8[channel] = g_fixture->quanta_to_volts(out_values[voltage_count]);
            }
          }
          fprintf(fp1, "\n");          
        }
        fprintf(fp1, "\n");
      }

      fprintf(fp1, "\n");
      for(channel = 0; channel< NUM_CHANNELS_x8; channel++)
      {
        fprintf(fp1, "max diff ch %3d = %8.4f V at %8.4f V\n", channel, max_diff_x8[channel], diff_voltage_x8[channel]);
      }

      // Output for x16
      for(voltage_count = 0; voltage_count < NUM_VOLTAGES; voltage_count++)
      {

        fprintf(fp2, "Voltage = %8.4f", g_fixture->quanta_to_volts(out_values[voltage_count]));
        for(channel = 0; channel < NUM_CHANNELS_x16; channel++)
        {
          fprintf(fp2, ",ch = %3d", channel);
        }

        for(channel = 0; channel < NUM_CHANNELS_x16; channel++)
        {
          fprintf(fp2, ",dv %3d", channel);
        }
        fprintf(fp2, "\n");

        for(sample_count = 0; sample_count < NUM_SAMPLES; sample_count++)
        {
          fprintf(fp2, "                sample = %3d", sample_count);
          for(channel = 0; channel < NUM_CHANNELS_x16; channel++)
          {
            fprintf(fp2, ",%8.4f", in_voltage_x16[voltage_count][channel][sample_count]);
          }

          for(channel = 0; channel < NUM_CHANNELS_x16; channel++)
          {
            diff = abs(in_voltage_x16[voltage_count][channel][sample_count] - g_fixture->quanta_to_volts(out_values[voltage_count]));
            fprintf(fp2, ",%8.4f", diff);

            if(diff > max_diff_x16[channel])
            {
              max_diff_x16[channel] = diff;
              diff_voltage_x16[channel] = g_fixture->quanta_to_volts(out_values[voltage_count]);
            }
          }
          fprintf(fp2, "\n");          
        }
        fprintf(fp2, "\n");
      }

      fprintf(fp2, "\n");
      for(channel = 0; channel< NUM_CHANNELS_x16; channel++)
      {
        fprintf(fp2, "max diff ch %3d = %8.4f V at %8.4f V\n", channel, max_diff_x16[channel], diff_voltage_x16[channel]);
      }
     
      execute_state++;
    }


  // Close the log files
    if(3 == execute_state)
    {
      fclose(fp1);
      fclose(fp2);
      g_dc_active = false;   // Signal the main thread we can exit.
      execute_state++;
    }
  }

}


// Tests from the original main.cpp under here
// These tests will not make fatal exit yet for testing purposes
TEST_F(AcontisTestFixture, AODCCoupling)
{
  char meter_cmd[SEND_BUFF_SIZE], meter_reply[RECV_BUFF_SIZE];
  int  len;

   g_fixture = this;

  if(0 != g_fixture->connect_to_DAQ())
  {
    return;
  }

  AO_DC_COUPLING callBack(this);
  // Open Fusion Instance
  open_fusion(&callBack);
  ASSERT_EQ(fusion_instance==NULL, false) << "Returned Fusion interface is NULL, exiting\n";
  // Go to OP mode
  go_to_op_mode(PROJECT_VERSION);


  send(g_this_socket, "*IDN?\n", 6, 0);
  recv(g_this_socket, meter_reply, RECV_BUFF_SIZE, 0);
  printf("%s\n", meter_reply);

  strcpy(meter_cmd, "DISPlay:TEXT \"Hello Dave.\n  You're looking well today.\"\n");
  len = strlen(meter_cmd);
  send(g_this_socket, meter_cmd, len, 0);

// Loop as long as the test is active
while(true == g_dc_active )
{
  if(true == g_dc_go_collect)
  {
    g_meter_data[0] = collect_data_1(g_meter_channel_x8);
    g_meter_data[1] = collect_data_1(g_meter_channel_x16);
    g_dc_data_collected = true;
    g_dc_go_collect = false;
  }
}

  send(g_this_socket, "DISPlay:TEXT:CLEar\n", 19, 0);
}
