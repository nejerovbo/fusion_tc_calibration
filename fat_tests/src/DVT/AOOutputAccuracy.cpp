// For console output and gflag testing
#include "AcontisTestFixture.h"
#include "CRAMProcessData.h"
#include "sys/socket.h"
#include "arpa/inet.h"
#include "stdio.h"
#include "stdbool.h"
#include "math.h"

using namespace std;

extern AcontisTestFixture *g_fixture;


extern  int   collect_data_24();
extern  int   g_this_socket;
extern  double g_meter_data[24];

// Flags to keep track of what is going on with the data collection.
// g_active2 is a flag from the cyclic to the main thread that we are collecting data,
// ao_go_collect2 is a flag from the cyclic thread to the main thread to go collect data.
//    the cyclic thread sets it and the main thread clears it
// g_data_collected2 is a flag from the main thread to the cyclic thread to tell the cyclic
//    thread the data has been collected.  The Main thread sets it and the cyclic thread
//    clear it.
// start_read is a flag for the cyclic thread to only initiate data collection once, 
//    the rest of the time it polls g_data_collected2 waiting for completion.
bool  g_active2 = true;
bool ao_go_collect2 = false;
bool g_data_collected2 = false;

FILE *csv_log_fd;


#define PROJECT_NAME "AnalogInputStepResponse_EtherCAT_test"
#define PROJECT_VERSION "v1.0.0"
#define ONE_SEC_DELAY_MS 1000



#define NUM_CHANNELS_x8       8
#define AOUT_OFFSET_x8        0     // 0 for 6 slot RIM
#define AIN_OFFSET_x8         0     // 0 for 6 slot RIM

#define NUM_CHANNELS_x16     16
#define AOUT_OFFSET_x16      (48)   // 48 for 12 slot RIM, 0 for 6 slot RIM
#define AIN_OFFSET_x16       68   // 68 for 12 slot RIM, 0 for 6 slot RIM


#define NUM_VOLTAGES         41


// Cycle count definitions
#define WRITE_CYCLE         0
#define READ_CYCLE          100   // Read a sample every N cycles
#define NUM_SAMPLES         10

int volts_to_quanta_ao(double volts)
{
  int volt_conv = round(volts * 3276.8f);
  printf("volt conv %d \n", volt_conv);
  return volt_conv;
}

extern AcontisTestFixture *g_fixture;

class AO_OUT_ACCURACY : public CallbackTest
{
  public:
    AO_OUT_ACCURACY (AcontisTestFixture *p): CallbackTest(p) {}
    virtual void cyclic_method (void*);
};

#define AOUT_MAX_POS_VOLTAGE (9.99969f)
#define AOUT_MAX_NEG_VOLTAGE (-10.0f)

void AO_OUT_ACCURACY::cyclic_method (void *arg)
{
  static FILE *fp1, *fp2;
  static int execute_state, channel, voltage_count, cycle_count, sample_count;

  static int    out_values[NUM_VOLTAGES] = {0x8000, 0xa000, 0xc000, 0xe000, 0x0000, 0x2000, 0x4000, 0x6000, 0x7FFF};
  static double  in_voltage_x8[NUM_VOLTAGES][NUM_CHANNELS_x8][NUM_SAMPLES], in_voltage_x16[NUM_VOLTAGES][NUM_CHANNELS_x16][NUM_SAMPLES];
  static double  diff, max_diff_x8[NUM_CHANNELS_x8], diff_voltage_x8[NUM_CHANNELS_x8], max_diff_x16[NUM_CHANNELS_x16], diff_voltage_x16[NUM_CHANNELS_x16], running_diff[3];
  static bool   start_read = true, diff_change = 0;
  char          meter_cmd[SEND_BUFF_SIZE];
  int           len;
  static double output_voltage = AOUT_MAX_NEG_VOLTAGE;

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
      fp1 = fopen("log_aout/aout_log_x8.csv", "w");
      fprintf(fp1, "Analog Output Accuracy x8\n\n");
      fp2 = fopen("log_aout/aout_log_x16.csv", "w");
      fprintf(fp2, "Analog Output Accuracy x16\n\n");

      execute_state++;
    }

    // Set the output voltage to all channels of both the 6 Slot RIM(x8) and the
    //  12 slot RIM (x16).
    // Delay some number of cycles, then read the voltage from the DAQ 790A, 
    //  Data Acquisition Unit.  
    // The read is actually done in the main thread, triggered by ao_go_collect2
    //    
    if(1 == execute_state)
    {
      if(voltage_count < NUM_VOLTAGES)
      {
        if(cycle_count == WRITE_CYCLE)
        {
          // Set the output voltage for the current cycle
          out_values[voltage_count] = volts_to_quanta_ao(output_voltage);
          printf("\n\n\n\n output_voltage %f \n", output_voltage);

          // Increment for the next cycle, while less than the max voltage
          if (output_voltage < AOUT_MAX_POS_VOLTAGE)
          {
            output_voltage += 0.5;
          }

          // Cap the output voltage at AOUT_MAX_POS_VOLTAGE
          if ( output_voltage >= AOUT_MAX_POS_VOLTAGE )
          {
            output_voltage = AOUT_MAX_POS_VOLTAGE;
          }

          // Set all the aout channels to 0 V, then set the specific test channels later on
          if (fusion_instance->aout_count)
          {
            // set the aout values according the test pattern
            for(channel=0; channel < fusion_instance->aout_count; channel++)
              // fusion_instance->aouts[count] = output_value;
              ddi_sdk_fusion_set_aout(fusion_instance, channel, 0);
          }
          static int toggle_on = 0;
          uint16_t aout_val;
          if ( toggle_on )
          {
             aout_val = 0xFFFF;
             toggle_on = 0;
          }
          else
          {
            aout_val = 0;
            toggle_on = 1;
          }
          for(channel = 0; channel < NUM_CHANNELS_x8; channel++)
          {
            ddi_sdk_fusion_set_aout(fusion_instance, channel + AOUT_OFFSET_x8, out_values[voltage_count]);
          }
          for(channel = 0; channel < NUM_CHANNELS_x16; channel++)
          {
            // printf("0x%x \n", out_values[voltage_count]);
            // ddi_sdk_fusion_set_aout(fusion_instance, channel + AOUT_OFFSET_x8, out_values[voltage_count]);
            // ddi_sdk_fusion_set_aout(fusion_instance, AOUT_OFFSET_x16, 0x8000);
            ddi_sdk_fusion_set_aout(fusion_instance, channel + AOUT_OFFSET_x16,  out_values[voltage_count]);
          }
        }
        else if(cycle_count == (READ_CYCLE * (sample_count + 1)))
        {
          if(true == start_read)  // Only initiate the data collection once per matching cycle count
          {
            ao_go_collect2 = true;
            start_read = false;
          }

          if ( g_data_collected2 == false) // Wait for g_data_collected2 to be true before proceeding
          {
            return;
          }
          g_data_collected2 = false;
          start_read = true;            // Reset for next time we need to start collecting

          // Move the data 8 values at a time.  This will probably change when we decide which output on Fusion
          //  is connected to which channel on the DAQ
          for(channel = 0; channel < 8; channel++)
          {
            in_voltage_x8[voltage_count][channel][sample_count]       = g_meter_data[channel];
            in_voltage_x16[voltage_count][channel][sample_count]      = g_meter_data[channel + 8];
            in_voltage_x16[voltage_count][channel + 8][sample_count]  = g_meter_data[channel + 16];

            diff = abs(in_voltage_x8[voltage_count][channel][sample_count] - g_fixture->quanta_to_volts(out_values[voltage_count]));
            if(diff > running_diff[0])
            {
              running_diff[0] = diff;
              diff_change = 1;
            }
            printf("in_voltage_x16[%d][%d][%d] %f \n", voltage_count, channel, sample_count, in_voltage_x16[voltage_count][channel][sample_count]);
            diff = abs(in_voltage_x16[voltage_count][channel][sample_count] - g_fixture->quanta_to_volts(out_values[voltage_count]));
            if(diff > running_diff[1])
            {
              running_diff[1] = diff;
              diff_change = 1;
            }
            diff = abs(in_voltage_x16[voltage_count][channel + 8][sample_count] - g_fixture->quanta_to_volts(out_values[voltage_count]));
            if(diff > running_diff[2])
            {
              running_diff[2] = diff;
              diff_change = 1;
            }
            if(diff_change == 1)
            {
              sprintf(meter_cmd, "DISPlay:TEXT \"diff %4.2f\ndiff %4.2f\ndiff %4.2f\n\"\n", running_diff[0] * 1000, running_diff[1] * 1000, running_diff[2] * 1000);
              len = strlen(meter_cmd);
              send(g_this_socket, meter_cmd, len, 0);              
              printf("diffx8       = %8.2f mV\ndiffx16_low  = %8.2f mV\ndiffx16_high = %8.2f mV\n\n", running_diff[0] * 1000, running_diff[1] * 1000, running_diff[2] * 1000);
              
              diff_change = 0;
            }
          }

//strcpy(meter_cmd, "DISPlay:TEXT \"Hello Dave.\n  You're looking well today.\"\n");

          sample_count++;
        }
        cycle_count++;
        if(sample_count == NUM_SAMPLES)
        {
          sample_count = 0;
          cycle_count = 0;
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
            // fprintf(csv_log_fd, ",%d", volts_to_quanta_ao(in_voltage_x16[voltage_count][channel][sample_count]));
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
        for(channel = 0; channel < 1; channel++)
        {
          for(int volt_log_count = 0; volt_log_count < NUM_VOLTAGES; volt_log_count++)
          {
            double local_volts =  g_fixture->quanta_to_volts(out_values[volt_log_count]);
            double local_diff_accumulator = 0;
            double dmm_reading_accumulator = 0;
            fprintf(csv_log_fd,"%d,%f,%f", channel, local_volts, local_volts);
            uint average_quanta_accumulator = 0;
            local_diff_accumulator = 0;
            dmm_reading_accumulator = 0;

            // For each sample, log the data point and calculate an average.
            for(sample_count = 0; sample_count < NUM_SAMPLES; sample_count++)
            {
              double local_diff;
              // Get the DMM reading in unsigned 16-bit format
              printf("dmm %f \n", in_voltage_x16[volt_log_count][channel][sample_count]);
              uint16_t reading = volts_to_quanta_ao(in_voltage_x16[volt_log_count][channel][sample_count]);
              printf("reading %d \n", reading);
              dmm_reading_accumulator += in_voltage_x16[volt_log_count][channel][sample_count];
              // Log the Data column
              fprintf(csv_log_fd, ",%d", reading);
              // DMM voltage - commanded voltage
              local_diff = in_voltage_x16[volt_log_count][channel][sample_count] - g_fixture->quanta_to_volts(out_values[volt_log_count]);
              printf("\n\n\n local diff = %f \n\n\n", local_diff);
              // Accumate the differences
              local_diff_accumulator += local_diff;
            }
            
            fprintf(csv_log_fd, ",%f,%f\n", dmm_reading_accumulator/NUM_SAMPLES, local_diff_accumulator/NUM_SAMPLES);
            
          }
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
      fclose(csv_log_fd);
      g_active2 = false;   // Signal the main thread we can exit.
      printf("Done\n");
      execute_state++;
      exit(0);
    }
  }

}


// Tests from the original main.cpp under here
// These tests will not make fatal exit yet for testing purposes
TEST_F(AcontisTestFixture, AOOutputAccuracy)
{
  char meter_cmd[SEND_BUFF_SIZE], meter_reply[RECV_BUFF_SIZE];
  struct  sockaddr_in this_saddr;
  int  len, count;

  g_fixture = this;

  if(0 != g_fixture->connect_to_DAQ())
  {
    return;
  }

  send(g_this_socket, "*IDN?\n", 6, 0);
  recv(g_this_socket, meter_reply, RECV_BUFF_SIZE, 0);
  printf("%s", meter_reply);

  strcpy(meter_cmd, "DISPlay:TEXT \"Hello Dave.\n  You're looking well today.\"\n");
  len = strlen(meter_cmd);
  send(g_this_socket, meter_cmd, len, 0);

  csv_log_fd = fopen("results.csv", "w");
  fprintf(csv_log_fd, "Channel,PowerSource,MultiMeter,");
  for (count = 1; count < 1; count++)
  {
    fprintf(csv_log_fd, "Data %d,", count);
  }
  
  fprintf(csv_log_fd,"Data %d, Calculated Voltage, Residual Error (mV)\n", count);
  fflush(csv_log_fd);

  AO_OUT_ACCURACY callBack(this);
  // Open Fusion Instance
  open_fusion(&callBack);
  ASSERT_EQ(fusion_instance==NULL, false) << "Returned Fusion interface is NULL, exiting\n";

  // Go to OP mode
  go_to_op_mode(PROJECT_VERSION);

  


  // Sleep for 60 seconds
//  AcontisTestFixture::poll_EtherCAT(SEC_PER_MIN,ONE_SEC_DELAY_MS);
//  AcontisTestFixture::poll_EtherCAT(30,ONE_SEC_DELAY_MS);

  while(1 )
  {
    if(true == ao_go_collect2)
    {
      collect_data_24();
      g_data_collected2 = true;
      ao_go_collect2 = false;
    }
  }

  send(g_this_socket, "DISPlay:TEXT:CLEar\n", 19, 0);
}
