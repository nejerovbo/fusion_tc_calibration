// For console output and gflag testing
#include "AcontisTestFixture.h"
#include "CRAMProcessData.h"
#include <fcntl.h>
#include <termios.h>



using namespace std;

#define PROJECT_NAME "AnalogInputStepResponse_EtherCAT_test"
#define PROJECT_VERSION "v1.0.0"
#define ONE_SEC_DELAY_MS 1000


//#define TTY_FILE "/dev/ttyACM0"   // This is from Zach's code but didn't work for me
#define TTY_FILE "/dev/ttyUSB0"
extern int serial_port;


#define NUM_CHANNELS        2
#define NUM_SAMPLES         1000
#define NUM_VOLTAGES        9


#define AIN_OFFSET_SLOT_06   00
#define AIN_OFFSET_SLOT_12   68

#define OUTPUT_CYCLES       500     // Number of cycles to delay from writing an output voltage to starting to read inputs.
                                

AcontisTestFixture *g_fixture;

class AO_INPUT_ACCURACY : public CallbackTest
{
  public:
    AO_INPUT_ACCURACY (AcontisTestFixture *p): CallbackTest(p) {}
    virtual void cyclic_method (void*);
};

void AO_INPUT_ACCURACY::cyclic_method (void *arg)
{
  static FILE *fp1, *fp2;
  static int  channel_count, voltage_count, sample_count, output_count, execute_state;
  static int  in_quanta_06[NUM_VOLTAGES][NUM_CHANNELS][NUM_SAMPLES];
  static int  in_quanta_12[NUM_VOLTAGES][NUM_CHANNELS][NUM_SAMPLES];

  static float  diff, max_diff_06[NUM_CHANNELS], diff_voltage_06[NUM_CHANNELS], max_diff_12[NUM_CHANNELS], diff_voltage_12[NUM_CHANNELS];

  static int channel[NUM_CHANNELS] = {1, 7};
  static int out_values[NUM_VOLTAGES] = {0x8000, 0xa000, 0xc000, 0xe000, 0x0000, 0x2000, 0x4000, 0x6000, 0x7FFF};

  uint16_t current_state, requested_state, input_value;
  ddi_fusion_instance_t* fusion_instance;

  static float  voltage;
  char          cmd_buff[50];
  int           len;

  //check parameters
  if (!arg)
    return;

  //set the fusion interface from the argument to the cyclic data function
  fusion_instance = (ddi_fusion_instance_t *)arg;

  ddi_sdk_ecat_get_slave_state(fusion_instance->slave,&current_state,&requested_state);
  //don't process data unless the slave is in OP mode
  if ( current_state != DDI_ECAT_OP)
    return;


  if ((fusion_instance->aout_count))
  {
    // Do "start stuff"
    // Create the log files.
    if(0 == execute_state)
    {
      fp1 = fopen("log_ain/ain_log_x8.csv", "w");
      fp2 = fopen("log_ain/ain_log_x16.csv", "w");
      fprintf(fp1, "Analog Input Accuracy log file  6 Slot RIM, .5 kHz\n\n");
      fprintf(fp2, "Analog Input Accuracy log file 12 Slot RIM, .5 kHz\n\n");
      execute_state++;
    }


    // Come here every cycle until we collect all of the data
    // For every voltage:
    //  Set the voltage on the meter
    //  Delay some number of cycles, output_count
    //  for each sample
    //    Delay some numbe of cycles, input_count
    //    for each channel
    //      get input voltage for the  6 slot rim
    //      get input voltage for the 12 slot rim
    //  
    if(1 == execute_state)
    {
      if(voltage_count < NUM_VOLTAGES)
      {
        if(0 == output_count)
        {
          voltage = g_fixture->quanta_to_volts(out_values[voltage_count]);
          len = sprintf(cmd_buff, "VOLT %.2f;\r", voltage);
          len = write(serial_port, cmd_buff, len);
          printf("New voltage\n");
          output_count++;
        }
        else if(output_count < OUTPUT_CYCLES)
        {
          output_count++;
        }
        else
        {
          if(sample_count < NUM_SAMPLES)
          {
            for(channel_count = 0; channel_count < NUM_CHANNELS; channel_count++)
            {
              in_quanta_06[voltage_count][channel_count][sample_count] = 
                              ddi_sdk_fusion_get_ain(fusion_instance, channel[channel_count] + AIN_OFFSET_SLOT_06);
              in_quanta_12[voltage_count][channel_count][sample_count] = 
                              ddi_sdk_fusion_get_ain(fusion_instance, channel[channel_count] + AIN_OFFSET_SLOT_12);
            }
            sample_count++;
          }
          else
          {
            printf("Done sampling\n");
            output_count = 0;
            sample_count = 0;
            voltage_count++;
          }
        }
      }
      else
      {
        execute_state++;
      }
    }


  // We have the data, now write it to the log file
    if(2 == execute_state)
    {
      for(voltage_count = 0; voltage_count < NUM_VOLTAGES; voltage_count++)
      {
        fprintf(fp1, "Voltage = %8.4f", g_fixture->quanta_to_volts(out_values[voltage_count]));
        for(channel_count = 0; channel_count < NUM_CHANNELS; channel_count++)
        {
          fprintf(fp1, ",ch = %3d", channel[channel_count]);
        }

        for(channel_count = 0; channel_count < NUM_CHANNELS; channel_count++)
        {
          fprintf(fp1, ",dv %3d", channel[channel_count]);
        }
        fprintf(fp1, "\n");


        for(sample_count = 0; sample_count < NUM_SAMPLES; sample_count++)
        {
          for(channel_count = 0; channel_count < NUM_CHANNELS; channel_count++)
          {
            fprintf(fp1, ",%8.4f", g_fixture->quanta_to_volts(in_quanta_06[voltage_count][channel_count][sample_count]));
          }

          for(channel_count = 0; channel_count < NUM_CHANNELS; channel_count++)
          {
            diff = abs(g_fixture->quanta_to_volts(in_quanta_06[voltage_count][channel_count][sample_count] - out_values[voltage_count]));
            fprintf(fp1, ",%8.4f", diff);

            if(diff > max_diff_06[channel_count])
            {
              max_diff_06[channel_count] = diff;
              diff_voltage_06[channel_count] = g_fixture->quanta_to_volts(out_values[voltage_count]);
            }
          }
          fprintf(fp1, "\n");          
        }
        fprintf(fp1, "\n");

        fprintf(fp2, "Voltage = %8.4f", g_fixture->quanta_to_volts(out_values[voltage_count]));
        for(channel_count = 0; channel_count < NUM_CHANNELS; channel_count++)
        {
          fprintf(fp2, ",ch = %3d", channel[channel_count]);
        }

        for(channel_count = 0; channel_count < NUM_CHANNELS; channel_count++)
        {
          fprintf(fp2, ",dv %3d", channel[channel_count]);
        }
        fprintf(fp2, "\n");


        for(sample_count = 0; sample_count < NUM_SAMPLES; sample_count++)
        {
          for(channel_count = 0; channel_count < NUM_CHANNELS; channel_count++)
          {
            fprintf(fp2, ",%8.4f", g_fixture->quanta_to_volts(in_quanta_12[voltage_count][channel_count][sample_count]));
          }

          for(channel_count = 0; channel_count < NUM_CHANNELS; channel_count++)
          {
            diff = abs(g_fixture->quanta_to_volts(in_quanta_12[voltage_count][channel_count][sample_count] - out_values[voltage_count]));
            fprintf(fp2, ",%8.4f", diff);

            if(diff > max_diff_12[channel_count])
            {
              max_diff_12[channel_count] = diff;
              diff_voltage_12[channel_count] = g_fixture->quanta_to_volts(out_values[voltage_count]);
            }
          }
          fprintf(fp2, "\n");          
        }
        fprintf(fp2, "\n");

      }

      fprintf(fp1, "\n");
      for(channel_count = 0; channel_count < NUM_CHANNELS; channel_count++)
      {
        fprintf(fp1, "max diff ch %3d = %8.4f V at %8.4f V\n", channel[channel_count], max_diff_06[channel_count], diff_voltage_06[channel_count]);
      }

      fprintf(fp2, "\n");
      for(channel_count = 0; channel_count < NUM_CHANNELS; channel_count++)
      {
        fprintf(fp2, "max diff ch %3d = %8.4f V at %8.4f V\n", channel[channel_count], max_diff_12[channel_count], diff_voltage_12[channel_count]);
      }

      execute_state++;
    }


  // Close the log files
    if(3 == execute_state)
    {
      fclose(fp1);
      fclose(fp2);
      printf("Done\n");
      execute_state++;
    }
  }
}


// Tests from the original main.cpp under here
// These tests will not make fatal exit yet for testing purposes
TEST_F(AcontisTestFixture, AIInputAccuracy)
{
  AO_INPUT_ACCURACY callBack(this);

  serial_port = open(TTY_FILE, O_RDWR);

  if (serial_port < 0) {
    printf("Error %i from open: %s\n", errno, strerror(errno));
  }

struct termios tty;

  if(tcgetattr(serial_port, &tty) != 0) {
    printf("Error %i from tcgetattr: %s\n", errno, strerror(errno));
  }

  tty.c_cflag &= ~PARENB;
  tty.c_cflag &= ~CSTOPB;
  tty.c_cflag &= ~CSIZE;
  tty.c_cflag |= CS8;
  tty.c_cflag &= ~CRTSCTS;
  tty.c_cflag |= CREAD | CLOCAL;
  tty.c_cflag &= ~ICANON;
  
  tty.c_lflag &= ~ECHO;
  tty.c_lflag &= ~ISIG;

  tty.c_iflag &= ~(IXON | IXOFF | IXANY);
  tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL);

  tty.c_oflag &= ~OPOST;
  tty.c_oflag &= ~ONLCR;

  tty.c_cc[VTIME] = 10;
  tty.c_cc[VMIN] = 0;

  cfsetispeed(&tty, B9600);

  if (tcsetattr(serial_port, TCSANOW, &tty) != 0) {
    printf("Error %i from tcsetattr: %s\n", errno, strerror(errno));
  }

  // Open Fusion Instance
  open_fusion(&callBack);
  ASSERT_EQ(fusion_instance==NULL, false) << "Returned Fusion interface is NULL, exiting\n";

  g_fixture = this;

  // Go to OP mode
  go_to_op_mode(PROJECT_VERSION);

  // 16 minutes, for 9 voltages, 2 channels, 1000 samples
  AcontisTestFixture::poll_EtherCAT(60, ONE_SEC_DELAY_MS);
}
