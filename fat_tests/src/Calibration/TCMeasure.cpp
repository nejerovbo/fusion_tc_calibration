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
#include <thread>
#include <termios.h>
#include <fcntl.h>
#include <boost/math/statistics/linear_regression.hpp>
#include "MatlabDataArray.hpp"
#include "MatlabEngine.hpp"
#include <iostream>
#include <time.h>

using namespace std;

extern AcontisTestFixture *g_fixture;


union f_to_w
{
    float fl;
    uint16_t  wds[2];
};

//-----------------------------------------------------------------
// Define the collection parameters
//-----------------------------------------------------------------
#define NUM_TC_CHANNELS   8
#define NUM_SAMPLES       20

#define TC_FULL_SCALE_VOLTS  .070    // 70 milli-volts

#define TC_OFFSET_NORMAL  0
#define TC_PARM_REG       8

struct cal_params
{
  float uVolt_slope[NUM_TC_CHANNELS] = {1.00, 1.00, 1.00, 1.00, 1.00, 1.00, 1.00, 1.00};
  float uVolt_intercept[NUM_TC_CHANNELS] = {0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00};
  int   tc_ohms[NUM_TC_CHANNELS] = {0, 0, 0, 0, 0, 0, 0, 0};
  float uAmp_gain = {0.0};
  float cj_slope = {1.0};
  float cj_intercept = {0.00};
};


void  set_cal_parms(ddi_fusion_instance_t* fusion_instance, cal_params *params);
void  display_cal_parms(ddi_fusion_instance_t* fusion_instance);
void  send_cal_command(ddi_fusion_instance_t* fusion_instance, int command);
void  write_cal_to_flash(ddi_fusion_instance_t* fusion_instance);
void  calibrate_tc(ddi_fusion_instance_t* fusion_instance);
void  calculate_tc_params(ddi_fusion_instance_t* fusion_instance, float uVolt_gain[], int uVolt_offset[]);
void  calculate_tc_ohms(ddi_fusion_instance_t* fusion_instance, int *tc_ohms);
void  calibrate_tc_slope_and_intercept(ddi_fusion_instance_t* fusion_instance, float uVolt_gain[], float uVolt_offset[]);
float calculate_uAmp_gain(ddi_fusion_instance_t* fusion_instance, int tc_ohms);
void  calculate_cj_parms(ddi_fusion_instance_t *fusion_instance, float *slope, float *intercept);
void  temp_display(ddi_fusion_instance_t* fusion_instance);

void  callFevalgcd(void);

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
class TC_MEASURE : public CallbackTest
{
  public:
    TC_MEASURE (AcontisTestFixture *p): CallbackTest(p) {}
    virtual void cyclic_method (void*);
};


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void TC_MEASURE::cyclic_method (void *arg)
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


#define ONE_SECOND    1000000
#define HUNDRED_MS    100000
#define TEN_MS        10000
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
TEST_F(AcontisTestFixture, TCTest)
{
  char c;
  int status;

  TC_MEASURE callBack(this);
  // Open Fusion Instance
  open_fusion(&callBack);

  status = ddi_sdk_ecat_set_license_file((char *)"acontis_licenses.csv");
  ASSERT_EQ(status, 0) << "Bad or no acontis_licenses.csv\n";


  // Check for fusion instance != NULL after calling open_fusion()
  ASSERT_EQ(fusion_instance==NULL, false) << "Returned Fusion interface is NULL, exiting\n";

  this->go_to_op_mode(CALIBRATION_PROJECT_VERSION);

  status = DC_init();
  ASSERT_EQ(status, 1);

  set_DC_range(TEN_VOLTS);
  sleep(1);
  enable_DC_out();

  // Establish connection to DMM
  status = open_connection_to_meter();
  ASSERT_EQ(status, 1);

//  callFevalgcd();

  calibrate_tc(fusion_instance);

  disable_DC_out();
}



//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void calibrate_tc(ddi_fusion_instance_t* fusion_instance)
{
  char c;
  cal_params m_params;
  
  // Send the default calibration parameters to the card.
  // This is necessary because the algorithm counts on "ideal" values
  //  being applied to the readings for calibration
  set_cal_parms(fusion_instance, &m_params);
  display_cal_parms(fusion_instance);
  
  // Put the switches on the calibration field connect in thermocouple slope intercept
  //  measure mode. This switches the DC205 voltage source to drive the thermocouple inputs
  system("ssh ubuntu@192.168.9.20 \"sudo ~/.local/bin/switch_util SI >/dev/null\"");

  // Calibrate the slope and intercept for the 8 thermocopule channels
  enable_DC_out();
  printf("Thermocouple slope and intercept measurement.\n");
  calibrate_tc_slope_and_intercept(fusion_instance, m_params.uVolt_slope, m_params.uVolt_intercept);
  disable_DC_out();

  printf("\n");
  // Display the results
  for(int i = 0; i < 8; i++)
  {
    printf("channel %2d, tc slope  = %7.5f,  tc intercept   = %7.5f\n", i, m_params.uVolt_slope[i], m_params.uVolt_intercept[i]);
  }

  // Put the switches on the calibration field connect in the resistance measuement
  //  mode. This shorts each pair of thermocouple inputs together to measure the
  //  loop resistance
  system("ssh ubuntu@192.168.9.20 \"sudo ~/.local/bin/switch_util RES >/dev/null\"");
  // Calculate resistance
  printf("\nResistance measurement.\n");
  calculate_tc_ohms(fusion_instance, m_params.tc_ohms);
  for(int i = 0; i < 8; i++)
  {
    printf("    tc ohms ch %d = %8d\n", i, m_params.tc_ohms[i]);
  }

  system("ssh ubuntu@192.168.9.20 \"sudo ~/.local/bin/switch_util CJ >/dev/null\"");
  enable_DC_out();
  printf("\nCold Junction slope and intercept measurement\n");
  calculate_cj_parms(fusion_instance, &m_params.cj_slope, &m_params.cj_intercept);
  disable_DC_out();

  printf("  cj offset = %5.2f\n", m_params.cj_intercept);
  printf("  cj_gain = %8.4f\n", m_params.cj_slope);


  // Send the real parameters to the card
  set_cal_parms(fusion_instance, &m_params);

  printf("Enter p to program the cal parameters to flash, any other key to continue\n");
  scanf("%c", &c);
  if( ('p' == c) || ('P' == c) )
  {
    write_cal_to_flash(fusion_instance);
  }
}


#define NUM_TC_DATA_POINTS  5   // Numer of data points to be collected to
                                //  generate the linear regression
#undef NUM_SAMPLES
#define NUM_SAMPLES 5   // debug only

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void calibrate_tc_slope_and_intercept(ddi_fusion_instance_t* fusion_instance, float tc_slope[], float tc_intercept[])
{
  using boost::math::statistics::simple_ordinary_least_squares;
  using boost::math::statistics::simple_ordinary_least_squares_with_R_squared;
  vector<double> x[NUM_TC_CHANNELS];
  vector<double> y;

  int sum, value;
  float voltage;

  ddi_sdk_fusion_set_aout(fusion_instance, TC_OFFSET_NORMAL, CAL_COMMAND_DISPLAY_TC_uVOLTS);
  usleep(TEN_MS);

  for(int num_points = 0; num_points < NUM_TC_DATA_POINTS; num_points++)
  {
    voltage = num_points * ((float)TC_FULL_SCALE_VOLTS/(NUM_TC_DATA_POINTS - 1));
    printf("\nSet voltage to  %8.2f uVolts,    ", voltage * 1000000);
    set_DC_voltage(voltage);
    usleep(ONE_SECOND);

    // Read value from DMM
    y.push_back(1000000 * read_meter_volts());
    printf("meter reading = %8.2f uVolts\n", y[num_points]);

    for(int chan = 0; chan < NUM_TC_CHANNELS; chan++)
    {
      sum = 0;
      for(int sam = 0; sam < NUM_SAMPLES; sam++)
      {
        usleep(HUNDRED_MS);
        usleep(HUNDRED_MS);
        usleep(HUNDRED_MS);
        usleep(HUNDRED_MS);
        value =   ddi_sdk_fusion_get_ain(fusion_instance, TC_OFFSET_NORMAL + (chan * 2) + 0) & 0xFFFF;
        value += (ddi_sdk_fusion_get_ain(fusion_instance, TC_OFFSET_NORMAL + (chan * 2) + 1) & 0xFFFF) << 16;
        sum += value;
      }
      x[chan].push_back((float)sum/NUM_SAMPLES);
      printf("    ch %d ave  = %8.2f uVolts\n", chan, x[chan][num_points]);
    }
  }

  for(int chan = 0; chan < NUM_TC_CHANNELS; chan++)
  {
//    auto [intercept, slope] = simple_ordinary_least_squares(x[chan], y);
    auto [intercept, slope, R] = simple_ordinary_least_squares_with_R_squared(x[chan], y);
    tc_slope[chan] = slope;
    tc_intercept[chan] = intercept;
  }
  
  ddi_sdk_fusion_set_aout(fusion_instance, TC_OFFSET_NORMAL, CAL_COMMAND_DISPLAY_NORMAL);
  usleep(TEN_MS);
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void callFevalgcd()
{
    printf("Feval\n");
    printf("Feval\n");
    // Pass vector containing MATLAB data array scalar
    using namespace matlab::engine;

    // Start MATLAB engine synchronously
    std::unique_ptr<MATLABEngine> matlabPtr = startMATLAB();

    // Create MATLAB data array factory
    matlab::data::ArrayFactory factory;

    // Pass vector containing 2 scalar args in vector    
    std::vector<matlab::data::Array> args({
        factory.createScalar<int16_t>(40),
        factory.createScalar<int16_t>(56) });

    // Call MATLAB function and return result
    matlab::data::TypedArray<int16_t> result = matlabPtr->feval(u"gcd", args);
    int16_t v = result[0];
    std::cout << "Result: " << v << std::endl;
    printf("Result: %d\n", v);
    printf("Feval\n");
}



#define FOUR_SECONDS  4000000
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void calculate_tc_ohms(ddi_fusion_instance_t* fusion_instance, int *tc_ohms)
{
  int   sum[NUM_TC_CHANNELS] = {0, 0, 0, 0, 0, 0, 0, 0};
  int   chan;

  ddi_sdk_fusion_set_aout(fusion_instance, TC_OFFSET_NORMAL, CAL_COMMAND_DISPLAY_TC_OHMS);
  usleep(TEN_MS);

  for(int samples = 0; samples < NUM_SAMPLES; samples++)
  {
    usleep(FOUR_SECONDS);
    for(chan = 0; chan < NUM_TC_CHANNELS; chan++)
    {
      int value;
      value = (ddi_sdk_fusion_get_ain(fusion_instance, TC_OFFSET_NORMAL + chan * 2)) & 0xFFFF;
      value += ((ddi_sdk_fusion_get_ain(fusion_instance, TC_OFFSET_NORMAL + (chan * 2) + 1)) & 0xFFFF) << 16;      
      sum[chan] += value;
    }
  }

  for(chan = 0; chan < NUM_TC_CHANNELS; chan++)
  {
    tc_ohms[chan] = round(sum[chan]/NUM_SAMPLES);
  }

  ddi_sdk_fusion_set_aout(fusion_instance, TC_OFFSET_NORMAL, CAL_COMMAND_DISPLAY_NORMAL);
  usleep(TEN_MS);

}



#define NUM_CJ_DATA_POINTS    5
#define CJ_FULL_SCALE_VOLTS   1.00
//-----------------------------------------------------------------------------
// Calculate slope and intercept
//   slope = (y2 - y1)/( x2 - x1)
//     where y is meter readings and x is adc readings 
//-----------------------------------------------------------------------------
void  calculate_cj_parms(ddi_fusion_instance_t *fusion_instance, float *slope_p, float *intercept_p)
{
  using boost::math::statistics::simple_ordinary_least_squares;
  using boost::math::statistics::simple_ordinary_least_squares_with_R_squared;
  vector<double> x;
  vector<double> y;

  int sum, value;
  float voltage;

  ddi_sdk_fusion_set_aout(fusion_instance, TC_OFFSET_NORMAL, CAL_COMMAND_DISPLAY_CJ_uVOLTS);
  usleep(TEN_MS);

  for(int num_points = 0; num_points < NUM_CJ_DATA_POINTS; num_points++)
  {
    voltage = num_points * ((float)CJ_FULL_SCALE_VOLTS/(NUM_CJ_DATA_POINTS - 1));
    printf("Set voltage to %8.2f uVolts,    ", voltage * 1000000);
    set_DC_voltage(voltage);
    usleep(ONE_SECOND);

    // Read value from DMM
    y.push_back(1000000 * read_meter_volts());
    printf("meter reading = %8.2f uVolts\n", y[num_points]);

    sum = 0;
    for(int sam = 0; sam < NUM_SAMPLES; sam++)
    {
      usleep(HUNDRED_MS);
      usleep(HUNDRED_MS);
      usleep(HUNDRED_MS);
      usleep(HUNDRED_MS);
      value =   ddi_sdk_fusion_get_ain(fusion_instance, TC_OFFSET_NORMAL + 0) & 0xFFFF;
      value += (ddi_sdk_fusion_get_ain(fusion_instance, TC_OFFSET_NORMAL + 1) & 0xFFFF) << 16;
      sum += value;
    }
    x.push_back((float)sum/NUM_SAMPLES);
    printf("        ave  = %8.2f uVolts\n", x[num_points]);
  }

//  auto [intercept, slope] = simple_ordinary_least_squares(x[chan], y);
  auto [intercept, slope, R] = simple_ordinary_least_squares_with_R_squared(x, y);

  *slope_p = slope;
  *intercept_p = intercept;
  
  ddi_sdk_fusion_set_aout(fusion_instance, TC_OFFSET_NORMAL, CAL_COMMAND_DISPLAY_NORMAL);
  usleep(TEN_MS);
}




//-----------------------------------------------------------------------------
// Send a calibration command to the TC card then return display to "normal"
//  Delay before sending the command in case the parameters were just loaded.
//-----------------------------------------------------------------------------
void send_cal_command(ddi_fusion_instance_t* fusion_instance, int command)
{
    usleep(TEN_MS);
    ddi_sdk_fusion_set_aout(fusion_instance, TC_OFFSET_NORMAL, command);
    usleep(TEN_MS);
    ddi_sdk_fusion_set_aout(fusion_instance, TC_OFFSET_NORMAL, CAL_COMMAND_DISPLAY_NORMAL);
    usleep(TEN_MS);
}


//-----------------------------------------------------------------------------
// Program the current values of gcal to the flash
//-----------------------------------------------------------------------------
void write_cal_to_flash(ddi_fusion_instance_t* fusion_instance)
{
  ddi_sdk_fusion_set_aout(fusion_instance, TC_OFFSET_NORMAL, CAL_COMMAND_PREP_WRITE);
  usleep(TEN_MS);
  usleep(TEN_MS);
  usleep(TEN_MS);
  usleep(TEN_MS);
  usleep(TEN_MS);
  ddi_sdk_fusion_set_aout(fusion_instance, TC_OFFSET_NORMAL, CAL_COMMAND_WRITE_CAL_TO_FLASH);
  usleep(TEN_MS);
  usleep(TEN_MS);
  usleep(TEN_MS);
  usleep(TEN_MS);
  usleep(TEN_MS);
}


//-----------------------------------------------------------------------------
// To set parameters, we need to put the new data in the parameter register,
//  then send the command, then send the "normal" command to keep the 
//  data from being overwritten.
//-----------------------------------------------------------------------------
void set_cal_parms(ddi_fusion_instance_t* fusion_instance, cal_params *m_params)
{
  int date[] = {2000, 1, 1, 0, 0, 0, 0, 0};

  time_t t = time(NULL);
  struct tm tm = *localtime(&t);
  date[0] = tm.tm_year + 1900;
  date[1] = tm.tm_mon + 1;
  date[2] = tm.tm_mday;
  date[3] = tm.tm_hour;
  date[4] = tm.tm_min;


  ddi_sdk_fusion_set_aout(fusion_instance, TC_OFFSET_NORMAL + TC_PARM_REG, 1);
  send_cal_command(fusion_instance, CAL_COMMAND_SET_VERSION);

  // Setting date info
  for(int i = 0; i < sizeof(date)/sizeof(int); i++)
  {
    ddi_sdk_fusion_set_aout(fusion_instance, TC_OFFSET_NORMAL + TC_PARM_REG + i, date[i]);
  }
  send_cal_command(fusion_instance, CAL_COMMAND_SET_DATE);

  // Setting uVolt offset
  for(int chan = 0; chan < NUM_TC_CHANNELS/2; chan++)
  {
    f_to_w intercept;
    intercept.fl = m_params->uVolt_intercept[chan];
    ddi_sdk_fusion_set_aout(fusion_instance, TC_OFFSET_NORMAL + TC_PARM_REG + (chan * 2), intercept.wds[0]);
    ddi_sdk_fusion_set_aout(fusion_instance, TC_OFFSET_NORMAL + TC_PARM_REG + (chan * 2) + 1 , intercept.wds[1]);
  }
  send_cal_command(fusion_instance, CAL_COMMAND_SET_UV_INTERCEPT_LO);

  // Setting uVolt offset
  for(int chan = 0; chan < NUM_TC_CHANNELS/2; chan++)
  {
    f_to_w intercept;
    intercept.fl = m_params->uVolt_intercept[chan + NUM_TC_CHANNELS/2];
    ddi_sdk_fusion_set_aout(fusion_instance, TC_OFFSET_NORMAL + TC_PARM_REG + (chan * 2), intercept.wds[0]);
    ddi_sdk_fusion_set_aout(fusion_instance, TC_OFFSET_NORMAL + TC_PARM_REG + (chan * 2) + 1 , intercept.wds[1]);  
  }
  send_cal_command(fusion_instance, CAL_COMMAND_SET_UV_INTERCEPT_HI);
  printf("\n");

  // Setting uVolt gain
  for(int chan = 0; chan < NUM_TC_CHANNELS/2; chan++)
  {
    f_to_w gain;
    gain.fl = m_params->uVolt_slope[chan];
    ddi_sdk_fusion_set_aout(fusion_instance, TC_OFFSET_NORMAL + TC_PARM_REG + (chan * 2), gain.wds[0]);
    ddi_sdk_fusion_set_aout(fusion_instance, TC_OFFSET_NORMAL + TC_PARM_REG + (chan * 2) + 1, gain.wds[1]);
  }
  send_cal_command(fusion_instance, CAL_COMMAND_SET_UV_SLOPE_LO);

  for(int chan = 0; chan < NUM_TC_CHANNELS/2; chan++)
  {
    f_to_w gain;
    gain.fl = m_params->uVolt_slope[chan + NUM_TC_CHANNELS/2];
    ddi_sdk_fusion_set_aout(fusion_instance, TC_OFFSET_NORMAL + TC_PARM_REG + (chan * 2), gain.wds[0]);
    ddi_sdk_fusion_set_aout(fusion_instance, TC_OFFSET_NORMAL + TC_PARM_REG + (chan * 2) + 1, gain.wds[1]);
  }
  send_cal_command(fusion_instance, CAL_COMMAND_SET_UV_SLOPE_HI);

  // Setting uAmp_gain value
  ddi_sdk_fusion_set_aout(fusion_instance, TC_OFFSET_NORMAL + TC_PARM_REG, m_params->uAmp_gain);
  send_cal_command(fusion_instance, CAL_COMMAND_SET_UA_SLOPE);

  // Setting ohms value for resistance
  for(int chan = 0; chan < NUM_TC_CHANNELS; chan++)
  {
    ddi_sdk_fusion_set_aout(fusion_instance, TC_OFFSET_NORMAL + TC_PARM_REG + chan, m_params->tc_ohms[chan]);    
  }
  send_cal_command(fusion_instance, CAL_COMMAND_SET_TC_OHMS);

  // Setting cold junction offset
  f_to_w intercept;
  intercept.fl = m_params->cj_intercept;
  ddi_sdk_fusion_set_aout(fusion_instance, TC_OFFSET_NORMAL + TC_PARM_REG + 0, intercept.wds[0]);
  ddi_sdk_fusion_set_aout(fusion_instance, TC_OFFSET_NORMAL + TC_PARM_REG + 1, intercept.wds[1]);
  send_cal_command(fusion_instance, CAL_COMMAND_SET_CJ_INTERCEPT);

  // Setting cold junction gain
  f_to_w gain;
  gain.fl = m_params->cj_slope;
  ddi_sdk_fusion_set_aout(fusion_instance, TC_OFFSET_NORMAL + TC_PARM_REG + 0, gain.wds[0]);
  ddi_sdk_fusion_set_aout(fusion_instance, TC_OFFSET_NORMAL + TC_PARM_REG + 1, gain.wds[1]);
  send_cal_command(fusion_instance, CAL_COMMAND_SET_CJ_SLOPE);  

}


//-----------------------------------------------------------------------------
// To read parameters, send the appropriate "get" command, delay, then read
//  the parameter registers.
//-----------------------------------------------------------------------------
void display_cal_parms(ddi_fusion_instance_t* fusion_instance)
{
  int value;

  printf("\n");
  ddi_sdk_fusion_set_aout(fusion_instance, TC_OFFSET_NORMAL ,CAL_COMMAND_GET_VERSION);
  usleep(TEN_MS);
  printf("Version       =    %5d\n", ddi_sdk_fusion_get_ain(fusion_instance, TC_OFFSET_NORMAL + TC_PARM_REG));

  ddi_sdk_fusion_set_aout(fusion_instance, TC_OFFSET_NORMAL , CAL_COMMAND_GET_DATE);
  usleep(TEN_MS);
  printf("Date/Time     =  ");
  printf("%4d/",   ddi_sdk_fusion_get_ain(fusion_instance, TC_OFFSET_NORMAL + TC_PARM_REG + 0));
  printf("%02d/",  ddi_sdk_fusion_get_ain(fusion_instance, TC_OFFSET_NORMAL + TC_PARM_REG + 1));
  printf("%02d  ", ddi_sdk_fusion_get_ain(fusion_instance, TC_OFFSET_NORMAL + TC_PARM_REG + 2));
  printf("%02d:",  ddi_sdk_fusion_get_ain(fusion_instance, TC_OFFSET_NORMAL + TC_PARM_REG + 3));
  printf("%02d",   ddi_sdk_fusion_get_ain(fusion_instance, TC_OFFSET_NORMAL + TC_PARM_REG + 4));
  printf("\n");


  ddi_sdk_fusion_set_aout(fusion_instance, TC_OFFSET_NORMAL , CAL_COMMAND_GET_UV_INTERCEPT_LO);
  usleep(TEN_MS);
  printf("uVolt offsets =  ");
  for(int chan = 0; chan < NUM_TC_CHANNELS/2; chan++)
  {
    f_to_w intercept;
    intercept.wds[0] = ddi_sdk_fusion_get_ain(fusion_instance, TC_OFFSET_NORMAL + TC_PARM_REG + (chan * 2) );
    intercept.wds[1] = ddi_sdk_fusion_get_ain(fusion_instance, TC_OFFSET_NORMAL + TC_PARM_REG + (chan * 2) + 1);
    printf("%5.5f    ", intercept.fl);
  }

  ddi_sdk_fusion_set_aout(fusion_instance, TC_OFFSET_NORMAL , CAL_COMMAND_GET_UV_INTERCEPT_HI);
  usleep(TEN_MS);
  for(int chan = 0; chan < NUM_TC_CHANNELS/2; chan++)
  {
    f_to_w intercept;
    intercept.wds[0] = ddi_sdk_fusion_get_ain(fusion_instance, TC_OFFSET_NORMAL + TC_PARM_REG + (chan * 2) );
    intercept.wds[1] = ddi_sdk_fusion_get_ain(fusion_instance, TC_OFFSET_NORMAL + TC_PARM_REG + (chan * 2) + 1);
    printf("%5.5f    ", intercept.fl);
  }
  printf("\n");


  ddi_sdk_fusion_set_aout(fusion_instance, TC_OFFSET_NORMAL , CAL_COMMAND_GET_UV_SLOPE_LO);
  usleep(TEN_MS);
  printf("uVolt gains   =  ");
  for(int chan = 0; chan < NUM_TC_CHANNELS/2; chan++)
  {
    f_to_w gain;
    gain.wds[0] = ddi_sdk_fusion_get_ain(fusion_instance, TC_OFFSET_NORMAL + TC_PARM_REG + (chan * 2));
    gain.wds[1] = ddi_sdk_fusion_get_ain(fusion_instance, TC_OFFSET_NORMAL + TC_PARM_REG + (chan * 2) + 1);
    printf("%5.5f    ",  gain.fl);
  }

  ddi_sdk_fusion_set_aout(fusion_instance, TC_OFFSET_NORMAL , CAL_COMMAND_GET_UV_SLOPE_HI);
  usleep(TEN_MS);
  for(int chan = 0; chan < NUM_TC_CHANNELS/2; chan++)
  {
    f_to_w gain;
    gain.wds[0] = ddi_sdk_fusion_get_ain(fusion_instance, TC_OFFSET_NORMAL + TC_PARM_REG + (chan * 2));
    gain.wds[1] = ddi_sdk_fusion_get_ain(fusion_instance, TC_OFFSET_NORMAL + TC_PARM_REG + (chan * 2) + 1);
    printf("%5.5f    ",  gain.fl);
  }
  printf("\n");


  ddi_sdk_fusion_set_aout(fusion_instance, TC_OFFSET_NORMAL , CAL_COMMAND_GET_UA_SLOPE);
  usleep(TEN_MS);
  printf("uAmp gain     =  ");
  printf("%5.5f\n",  (float)(1000 + (int16_t)(ddi_sdk_fusion_get_ain(fusion_instance, TC_OFFSET_NORMAL + TC_PARM_REG)))/1000);


  ddi_sdk_fusion_set_aout(fusion_instance, TC_OFFSET_NORMAL , CAL_COMMAND_GET_TC_OHMS);
  usleep(TEN_MS);
  printf("TC ohms       = ");
  for(int chan = 0; chan < NUM_TC_CHANNELS; chan++)
  {
    printf("%8d   ", ddi_sdk_fusion_get_ain(fusion_instance, TC_OFFSET_NORMAL + TC_PARM_REG + chan));
  }
  printf("\n");

  ddi_sdk_fusion_set_aout(fusion_instance, TC_OFFSET_NORMAL , CAL_COMMAND_GET_CJ_INTERCEPT);
  usleep(TEN_MS);
  f_to_w intercept;
  intercept.wds[0] = ddi_sdk_fusion_get_ain(fusion_instance, TC_OFFSET_NORMAL + TC_PARM_REG + 0);
  intercept.wds[1] = ddi_sdk_fusion_get_ain(fusion_instance, TC_OFFSET_NORMAL + TC_PARM_REG + 1);
  printf("CJ offset     =  %5.5f\n", intercept.fl);


  ddi_sdk_fusion_set_aout(fusion_instance, TC_OFFSET_NORMAL , CAL_COMMAND_GET_CJ_SLOPE);
  usleep(TEN_MS);
  f_to_w gain;
  gain.wds[0] = ddi_sdk_fusion_get_ain(fusion_instance, TC_OFFSET_NORMAL + TC_PARM_REG + 0);
  gain.wds[1] = ddi_sdk_fusion_get_ain(fusion_instance, TC_OFFSET_NORMAL + TC_PARM_REG + 1);
  printf("CJ gain       =  %5.5f   ",  gain.fl);
  printf("\n");

  ddi_sdk_fusion_set_aout(fusion_instance, TC_OFFSET_NORMAL ,CAL_COMMAND_DISPLAY_NORMAL);
  printf("\n\n");

}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void temp_display(ddi_fusion_instance_t* fusion_instance)
{
  int value;
  ddi_sdk_fusion_set_aout(fusion_instance, TC_OFFSET_NORMAL, CAL_COMMAND_DISPLAY_TC_uVOLTS);
  usleep(TEN_MS);

  for(int num_points = 0; num_points < 100; num_points++)
  {
    for(int chan = 0; chan < NUM_TC_CHANNELS; chan++)
    {

      usleep(HUNDRED_MS);
      usleep(HUNDRED_MS);
      usleep(HUNDRED_MS);
      usleep(HUNDRED_MS);
      value =   ddi_sdk_fusion_get_ain(fusion_instance, TC_OFFSET_NORMAL + (chan * 2) + 0) & 0xFFFF;
      value += (ddi_sdk_fusion_get_ain(fusion_instance, TC_OFFSET_NORMAL + (chan * 2) + 1) & 0xFFFF) << 16;
      printf("    %8d", value);
    }
    printf("\n");
  }
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

#define RES_VALUE   200
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
float   calculate_uAmp_gain(ddi_fusion_instance_t* fusion_instance, int tc_ohms)
{
    int   sum, average;
    float uAmp_gain;

  printf("Change switches to resistors, hit any key to continue\n");
  getchar();

  ddi_sdk_fusion_set_aout(fusion_instance, TC_OFFSET_NORMAL, CAL_COMMAND_DISPLAY_TC_OHMS);
  usleep(TEN_MS);

  sum = 0;
  for(int samples = 0; samples < NUM_SAMPLES; samples++)
  {
    int value;
    usleep(HUNDRED_MS);   // New samples every 400 ms
    usleep(HUNDRED_MS);   // New samples every 400 ms
    usleep(HUNDRED_MS);   // New samples every 400 ms
    usleep(HUNDRED_MS);   // New samples every 400 ms
    value = (ddi_sdk_fusion_get_ain(fusion_instance, TC_OFFSET_NORMAL )) & 0xFFFF;
    value += ((ddi_sdk_fusion_get_ain(fusion_instance, TC_OFFSET_NORMAL + 1)) & 0xFFFF) << 16;      
    sum += value;
  }
  average = round(sum/NUM_SAMPLES);
  average -= tc_ohms;
  uAmp_gain = (float)RES_VALUE/average;
  printf("uAmp_gain = %f\n", uAmp_gain);

  printf("\n");
  ddi_sdk_fusion_set_aout(fusion_instance, TC_OFFSET_NORMAL, CAL_COMMAND_DISPLAY_NORMAL);
  usleep(TEN_MS);
  return uAmp_gain;
}