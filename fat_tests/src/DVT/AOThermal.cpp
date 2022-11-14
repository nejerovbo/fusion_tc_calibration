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

FILE *thermal_csv_log_fd;

#define PROJECT_NAME "AO_Thermal_EtherCAT_test"
#define PROJECT_VERSION "v1.0.0"
#define ONE_SEC_DELAY_MS 1000

#define NUM_CHANNELS_x8       8
#define AOUT_OFFSET_x8        0     // 0 for 6 slot RIM
#define AIN_OFFSET_x8         0     // 0 for 6 slot RIM

#define NUM_CHANNELS_x16     16
#define AOUT_OFFSET_x16      (48+32)   // 48 for 12 slot RIM, 0 for 6 slot RIM
#define AIN_OFFSET_x16       68   // 68 for 12 slot RIM, 0 for 6 slot RIM
#define NUM_VOLTAGES         41

// Cycle count definitions
#define WRITE_CYCLE         0
#define READ_CYCLE          100   // Read a sample every N cycles
#define NUM_SAMPLES         10

extern AcontisTestFixture *g_fixture;

class AO_OUT_THERMAL : public CallbackTest
{
  public:
    AO_OUT_THERMAL (AcontisTestFixture *p): CallbackTest(p) {}
    virtual void cyclic_method (void*);
};

#define AOUT_MAX_POS_VOLTAGE (9.99969f)
#define AOUT_MAX_NEG_VOLTAGE (-10.0f)

static int g_warmup_complete = 0; // Is card warmup complete?

void AO_OUT_THERMAL::cyclic_method (void *arg)
{
  static FILE *fp1, *fp2;
  static int execute_state, channel, voltage_count, cycle_count, sample_count;

  static int    out_values[NUM_VOLTAGES] = {0x8000, 0xa000, 0xc000, 0xe000, 0x0000, 0x2000, 0x4000, 0x6000, 0x7FFF};
  static float  in_voltage_x8[NUM_VOLTAGES][NUM_CHANNELS_x8][NUM_SAMPLES], in_voltage_x16[NUM_VOLTAGES][NUM_CHANNELS_x16][NUM_SAMPLES];
  static float  diff, max_diff_x8[NUM_CHANNELS_x8], diff_voltage_x8[NUM_CHANNELS_x8], max_diff_x16[NUM_CHANNELS_x16], diff_voltage_x16[NUM_CHANNELS_x16], running_diff[3];
  static bool   start_read = true, diff_change = 0;
  char          meter_cmd[SEND_BUFF_SIZE];
  int           len;
  static double output_voltage = AOUT_MAX_POS_VOLTAGE;

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

  static int toggle_on = 0;
  uint16_t aout_value;
  static int test_iteration = 0;

  
  // Set AOUTs to a known value (-10 V)
  uint16_t ain_value;
  for ( channel = 0; channel < 16; channel++ )
  {
    ddi_sdk_fusion_set_aout(fusion_instance, channel + AOUT_OFFSET_x16, 0x8000);
  }
}


// Capture and log thermal results
TEST_F(AcontisTestFixture, AOOutputThermal)
{
  char meter_cmd[SEND_BUFF_SIZE], meter_reply[RECV_BUFF_SIZE];
  struct  sockaddr_in this_saddr;
  int  len, count, test_iteration;

  g_fixture = this;

  if(0 != g_fixture->connect_to_DAQ())
  {
    return;
  }

  printf("Running AO thermal test\n");

  send(g_this_socket, "*IDN?\n", 6, 0);
  recv(g_this_socket, meter_reply, RECV_BUFF_SIZE, 0);
  printf("%s", meter_reply);

  strcpy(meter_cmd, "DISPlay:TEXT \"AOUT Thermal test\n  ...\"\n");
  len = strlen(meter_cmd);
  send(g_this_socket, meter_cmd, len, 0);

  thermal_csv_log_fd = fopen("ao_thermal.csv", "w");
  
  fprintf(thermal_csv_log_fd,"Seconds, Channel, Voltage\n");
  fflush(thermal_csv_log_fd);

  AO_OUT_THERMAL callBack(this);
  // Open Fusion Instance
  open_fusion(&callBack);
  ASSERT_EQ(fusion_instance==NULL, false) << "Returned Fusion interface is NULL, exiting\n";

  // Go to OP mode
  go_to_op_mode(PROJECT_VERSION);

  // while (g_warmup_complete == 0 );
  // sleep(5); // Start measurements
  while (1)
  {
    collect_data_24();
    int channel = 0;
    for (channel = 8; channel < 9; channel++)
    {
      printf("Chn %d = %f \n", channel, g_meter_data[channel]);
      fprintf(thermal_csv_log_fd, "%d, %d, %f \n", test_iteration, channel-16, g_meter_data[channel]);
      fflush(thermal_csv_log_fd);
    }
    sleep(1);
    test_iteration++;
    if ( test_iteration == (60 * 55))
    {
      printf("Test exiting...\n");
      fclose(thermal_csv_log_fd);
      exit(EXIT_SUCCESS);
    }
  }

  send(g_this_socket, "DISPlay:TEXT:CLEar\n", 19, 0);
}
