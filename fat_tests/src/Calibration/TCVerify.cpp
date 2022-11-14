//----------------------------------------------------------------------------
// (c) Copyright 2022 Digital Dynamics Inc. Scotts Valley CA USA.
// Unpublished copyright. All rights reserved. Contains proprietary and
// confidential trade secrets belonging to DDI. Disclosure or release without
// prior written authorization of DDI is prohibited.
//----------------------------------------------------------------------------

#include "AcontisTestFixture.h"
#include "CRAMProcessData.h"
#include "sys/socket.h"
#include "telnet_open.h"
#include "arpa/inet.h"
#include "stdio.h"
#include "stdbool.h"
#include "math.h"
#include "CalibrationCommon.h"
#include "DAQUtil.h"
#include "inttypes.h"
#include "DCUtil.h"
#include "DMMUtil.h"
#include <fstream>
#include <string>
#include <thread>
#include <termios.h>
#include <fcntl.h>

using namespace std;

extern AcontisTestFixture *g_fixture;
extern void  display_cal_parms(ddi_fusion_instance_t* fusion_instance);

//-----------------------------------------------------------------------------
// Calibration command codes, these are shared between the calibration
//  program and the card, so they need to be synched.
//-----------------------------------------------------------------------------
#define CAL_COMMAND_DISPLAY_NORMAL          0x5A00
#define CAL_COMMAND_DISPLAY_TC_uVOLTS       0x5A01
#define CAL_COMMAND_DISPLAY_TC_OHMS         0x5A02
#define CAL_COMMAND_DISPLAY_CJ_uVOLTS       0x5A03
#define CAL_COMMAND_PREP_WRITE              0x5A0E
#define CAL_COMMAND_WRITE_CAL_TO_FLASH      0x5A0F

#define CAL_COMMAND_SET_VERSION             0x5A10
#define CAL_COMMAND_GET_VERSION             0x5A11
#define CAL_COMMAND_SET_DATE                0x5A12
#define CAL_COMMAND_GET_DATE                0x5A13
#define CAL_COMMAND_SET_UV_OFFSET_LO        0x5A14
#define CAL_COMMAND_SET_UV_OFFSET_HI        0x5A15
#define CAL_COMMAND_GET_UV_OFFSET_LO        0x5A16
#define CAL_COMMAND_GET_UV_OFFSET_HI        0x5A17
#define CAL_COMMAND_SET_UV_GAIN_LO          0x5A18      // Channels 3 - 0
#define CAL_COMMAND_SET_UV_GAIN_HI          0x5A19      // Channels 7 - 4
#define CAL_COMMAND_GET_UV_GAIN_LO          0x5A1A      // Channels 3 - 0
#define CAL_COMMAND_GET_UV_GAIN_HI          0x5A1B      // Channels 7 - 4
#define CAL_COMMAND_SET_UA_GAIN             0x5A1C
#define CAL_COMMAND_GET_UA_GAIN             0x5A1D
#define CAL_COMMAND_SET_TC_OHMS             0x5A1E
#define CAL_COMMAND_GET_TC_OHMS             0x5A1F
#define CAL_COMMAND_SET_CJ_OFFSET           0x5A20
#define CAL_COMMAND_GET_CJ_OFFSET           0x5A21
#define CAL_COMMAND_SET_CJ_GAIN             0x5A22
#define CAL_COMMAND_GET_CJ_GAIN             0x5A23

#define ONE_SECOND    1000000
#define HUNDRED_MS    100000
#define TEN_MS        10000

union f_to_w
{
    float fl;
    uint16_t  wds[2];
};


//-----------------------------------------------------------------
// Define the collection parameters
//-----------------------------------------------------------------
#define NUM_TC_CHANNELS   8
#define NUM_RTD_CHANNELS  4
#define NUM_SAMPLES       5
#define NUM_VOLTAGES      140
#define VOLTAGE_STEP      .0005   // .5 milli-volts per step

#define FULL_SCALE_VOLTS  .070    // 70 milli-volts

#define TC_OFFSET_NORMAL  0
#define TC_PARM_REG       8

void  verify_tc(ddi_fusion_instance_t* fusion_instance);
void  verify_cj(ddi_fusion_instance_t* fusion_instance);
void  verify_rtd(ddi_fusion_instance_t* fusion_instance);

float convert_uV_to_mV(int arg);

float convert_mV_to_V(float arg);
int mV_to_temp(float arg);
float convert_V_to_mV(float arg);


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
class VERIFIER : public CallbackTest
{
  public:
    VERIFIER (AcontisTestFixture *p): CallbackTest(p) {}
    virtual void cyclic_method (void*);
};


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void VERIFIER::cyclic_method (void *arg)
{
  uint16_t                current_state, requested_state;
  ddi_fusion_instance_t*  fusion_instance;

  //check parameters
  if (!arg)
    return;

  //set the fusion interface from the argument to the cyclic data function
  fusion_instance = (ddi_fusion_instance_t *)arg;

  ddi_sdk_ecat_get_slave_state(fusion_instance->slave,&current_state,&requested_state);
  //don't process data unless the slave is in OP mode
  if ( current_state != DDI_ECAT_OP)
    return;
  
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
TEST_F(AcontisTestFixture, RTDVerify)
{
  char c;
  int status, meter_status;

  VERIFIER callBack(this);
  // Open Fusion Instance
  open_fusion(&callBack);

  status = ddi_sdk_ecat_set_license_file((char *)"acontis_licenses.csv");
  ASSERT_EQ(status, 0) << "Bad or no acontis_licenses.csv\n";


  // Check for fusion instance != NULL after calling open_fusion()
  ASSERT_EQ(fusion_instance==NULL, false) << "Returned Fusion interface is NULL, exiting\n";

  this->go_to_op_mode(CALIBRATION_PROJECT_VERSION);

  // Establish connection to DC-205
  status = DC_init();
  ASSERT_EQ(status, 1);

  // Establish connection to DMM
  meter_status = open_connection_to_meter();
  ASSERT_EQ(meter_status, 1);

  set_DC_range(ONE_VOLT);
  sleep(1);

  verify_rtd(fusion_instance);

  disable_DC_out();

  // Disconnect from DMM
  meter_status = close_connection_to_meter();
  ASSERT_EQ(meter_status, 0);
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
TEST_F(AcontisTestFixture, TCVerify)
{
  char c;
  int status, meter_status;

  VERIFIER callBack(this);
  // Open Fusion Instance
  open_fusion(&callBack);

  status = ddi_sdk_ecat_set_license_file((char *)"acontis_licenses.csv");
  ASSERT_EQ(status, 0) << "Bad or no acontis_licenses.csv\n";

  // Check for fusion instance != NULL after calling open_fusion()
  ASSERT_EQ(fusion_instance==NULL, false) << "Returned Fusion interface is NULL, exiting\n";

  this->go_to_op_mode(CALIBRATION_PROJECT_VERSION);

  // Establish connection to DC-205
  status = DC_init();
  ASSERT_EQ(status, 1);

  set_DC_range(ONE_VOLT);

  // Establish connection to DMM
  meter_status = open_connection_to_meter();
  ASSERT_EQ(meter_status, 1);

  display_cal_parms(fusion_instance);

  sleep(1);

  verify_tc(fusion_instance);
//  verify_cj(fusion_instance);

  display_cal_parms(fusion_instance);

  disable_DC_out();

  // Disconnect from DMM
  meter_status = close_connection_to_meter();
  ASSERT_EQ(meter_status, 0);
}




#undef NUM_TC_CARDS
#define NUM_TC_CARDS 1
//-----------------------------------------------------------------------------
//  Walk thru all possible voltages, read back thru ECAT, also thru DMM, record
//-----------------------------------------------------------------------------
void verify_tc(ddi_fusion_instance_t* fusion_instance)
{
  float command_voltage, residual_error, data, dmm_reading;
  double voltage;
  int i, num_channels = 8, value, card;

  // Creating log files for all cards and initializing with header
  vector<FILE *> file_handles;
  for(card = 0; card < NUM_TC_CARDS; card++)
  {
    char buff[256];
    snprintf(buff, 256, "tc_verification_log_%d.csv", card);
    file_handles.push_back(fopen(buff, "w"));
    fprintf(file_handles.back(), "Channel,Commanded(mV),DMM(mV),Data(mV),RE(mV)\n");    
  }

  string readback;

  ifstream k_type_file("k_type.csv");

  if(!(k_type_file.is_open()))
  {
    // printf("Error opening file\n");
    cout << "error";
    exit(0);
  }
  
  // -------------------------------------------
  // THIS IS WHERE YOU PLAY WITH THE OFFSET TO MAKE 
  // IT ALL WORK
  // -------------------------------------------
  // Set all TC cards to display value in uVolts
  for (card = 0; card < NUM_TC_CARDS; card++)
  {
    ddi_sdk_fusion_set_aout(fusion_instance, TC_OFFSET_NORMAL + (card * 8), CAL_COMMAND_DISPLAY_TC_uVOLTS);
    usleep(TEN_MS);    
  }


  // Cycle through all voltages on the k-type TC table
  while(getline(k_type_file, readback, ','))
  {
    command_voltage = stof(readback);
    set_DC_voltage((double)convert_mV_to_V(command_voltage));

    sleep(1);

    // Read value from DMM
    dmm_reading = 1000 * read_meter_volts();

    for(card = 0; card < NUM_TC_CARDS; card++)
    {
      for(int chan = 0; chan < num_channels; chan++)
      {
        int value;
        value = (ddi_sdk_fusion_get_ain(fusion_instance, TC_OFFSET_NORMAL + chan * 2)) & 0xFFFF;
        value += ((ddi_sdk_fusion_get_ain(fusion_instance, TC_OFFSET_NORMAL + (chan * 2) + 1)) & 0xFFFF) << 16;
        data = convert_uV_to_mV(value);
        residual_error =  dmm_reading - data;
        printf("channel = %d,  command_voltage = %5.3f mV, dmm_reading = %5.3f, data = %5.3f mV, residual error = %5.3f mV\n", chan, command_voltage, dmm_reading, data, residual_error);   
        fprintf(file_handles.at(card), "%d,%.3f,%.5f,%.5f,%.5f\n", chan, command_voltage, dmm_reading, data, residual_error);
        fflush(file_handles.at(card));
      }
    }
  }

  // Exiting
  for(card = 0; card < NUM_TC_CARDS; card++)
  {
    fclose(file_handles.at(card));
  }

  k_type_file.close();
}


#define   MAX_CJ_VOLTAGE    1.00
//#define   CJ_VOLTAGE_STEP   0.001
#define   CJ_VOLTAGE_STEP   0.01  // temp while I debug new algorithm
//-----------------------------------------------------------------------------
//  Dump the cold junction values to the screen and a file
//-----------------------------------------------------------------------------
void verify_cj(ddi_fusion_instance_t* fusion_instance)
{
  int     num_voltages = (float)MAX_CJ_VOLTAGE/CJ_VOLTAGE_STEP;
  double  voltage;
  float   dmm_reading, data, residual_error;
  int     i, value, card;

  printf("num voltages = %d\n", num_voltages);

  // Creating log files for all cards and initializing with header
  vector<FILE *> file_handles;
  for(card = 0; card < NUM_TC_CARDS; card++)
  {
    char buff[256];
    snprintf(buff, 256, "cj_verification_log_%d.csv", card);
    file_handles.push_back(fopen(buff, "w"));
    fprintf(file_handles.back(), "Commanded(mV),DMM(mV),Data(mV),RE(mV)\n");    
  }

  // Set all TC cards to display value in uVolts
  for (card = 0; card < NUM_TC_CARDS; card++)
  {
    ddi_sdk_fusion_set_aout(fusion_instance, TC_OFFSET_NORMAL + (card * 8), CAL_COMMAND_DISPLAY_CJ_uVOLTS);
    usleep(TEN_MS);    
  

    for(int num_volts = 0; num_volts < num_voltages; num_volts++)
    {
      // Set DC output to desired voltage
      voltage = num_volts * CJ_VOLTAGE_STEP;;
      set_DC_voltage(voltage);
      usleep(ONE_SECOND);

      // Read value from DMM
      dmm_reading = 1000 * read_meter_volts();
      printf("voltage = %5.2f mV\n", dmm_reading);

      for (int sam = 0; sam < NUM_SAMPLES ; sam++)
      {
        usleep(TEN_MS);   // New samples every 10 ms
        value =    ddi_sdk_fusion_get_ain(fusion_instance, TC_OFFSET_NORMAL) & 0xFFFF;
        value += ((ddi_sdk_fusion_get_ain(fusion_instance, TC_OFFSET_NORMAL + 1)) & 0xFFFF) << 16;
        data = convert_uV_to_mV(value);
        residual_error =  dmm_reading - data;
        printf("command_voltage = %5.3f mV, dmm_reading = %5.3f, data = %5.3f mV, residual error = %5.3f mV\n",  voltage * 1000, dmm_reading, data, residual_error);   
        fprintf(file_handles.at(card), "%.3f,%.5f,%.5f,%.5f\n", voltage, dmm_reading, data, residual_error);
        fflush(file_handles.at(card));
      }
    }
  }

  // Exiting
  for(card = 0; card < NUM_TC_CARDS; card++)
  {
    fclose(file_handles.at(card));
  }

}


//-----------------------------------------------------------------------------
//  Walk thru all possible voltages, read back thru ECAT, also thru DMM, record
//-----------------------------------------------------------------------------
void verify_rtd(ddi_fusion_instance_t* fusion_instance)
{
  double starting_voltage = -0.01;
  double increment = 0.01;
  double ending_voltage = 0.07;
  double voltage, meter_value;
  int i, num_channels = NUM_RTD_CHANNELS, value;
  
  // Initialize log file and fill out headers
  FILE *fp = fopen("rtd_verification_log.csv", "w");
  fprintf(fp, "Channel,DC,DMM,");
  for (i = 0; i < NUM_SAMPLES - 1; i++)
  {
    fprintf(fp, "Data%d,", i + 1);
  }
  fprintf(fp, "Data%d\n", i + 1);

  // Walking thru possible voltages
  for( voltage = starting_voltage; voltage < ending_voltage; voltage += increment )
  {
    printf("Driving to %.3f Volt(s)\n", voltage);

    // Set DC output to desired voltage
    set_DC_voltage(voltage);
    usleep(HUNDRED_MS);

    // Read value from DMM
    meter_value = read_meter_volts();

    // Collect NUM_SAMPLES amount of AIN readings and log them
    for(int chan = 0; chan < num_channels; chan++)
    {
      printf("In channel %d\n", chan + 1);
      // Log channel info, DC voltage, and DMM readback voltage
      fprintf(fp, "%d,%f,%f,", chan, voltage, meter_value);
      for (i = 0; i < NUM_SAMPLES - 1; i++)
      {
        usleep(TEN_MS);   // New samples every 10 ms
        value = (ddi_sdk_fusion_get_ain(fusion_instance, TC_OFFSET_NORMAL + chan)) & 0xFFFF;
        fprintf(fp, "%f,", value / (10^3));
        fflush(fp);
      }
        usleep(TEN_MS);   // New samples every 10 ms
        value = (ddi_sdk_fusion_get_ain(fusion_instance, TC_OFFSET_NORMAL + chan)) & 0xFFFF;
        fprintf(fp, "%f\n", value / (10^3));
        fflush(fp);
    }
  }
}


struct cal_params
{
  int   uVolt_offset[NUM_TC_CHANNELS] = {0, 0, 0, 0, 0, 0, 0, 0};
  float uVolt_gain[NUM_TC_CHANNELS] = {1.00, 1.00, 1.00, 1.00, 1.00, 1.00, 1.00, 1.00};
  int   tc_ohms[NUM_TC_CHANNELS] = {0};
  float uAmp_gain = {0.0};
  int   cj_offset = {0};
  float cj_gain = {1.0};
};

extern void  set_default_cal_parms(ddi_fusion_instance_t* fusion_instance, cal_params *params);
extern void  display_cal_parms(ddi_fusion_instance_t* fusion_instance);



//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
float convert_mV_to_V(float arg)
{
  return arg / (1000);
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int mV_to_temp(float arg)
{
  return (arg * 16);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
float convert_uV_to_mV(int arg)
{
  return (float)arg / (1000);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
float convert_V_to_mV(float arg)
{
  return arg * (1000);
}