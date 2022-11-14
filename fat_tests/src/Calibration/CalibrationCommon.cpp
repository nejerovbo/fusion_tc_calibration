/*****************************************************************************
 * (c) Copyright 2022 Digital Dynamics Inc. Scotts Valley CA USA.
 * Unpublished copyright. All rights reserved. Contains proprietary and
 * confidential trade secrets belonging to DDI. Disclosure or release without
 * prior written authorization of DDI is prohibited.
 *****************************************************************************/

#include "AcontisTestFixture.h"
#include "CRAMProcessData.h"
#include <vector>
#include "DMMUtil.h"
#include "DACChip.h"
#include "ADCChip.h"
#include "DAQUtil.h"
#include "sys/socket.h"
#include "arpa/inet.h"
#include "stdio.h"
#include "stdbool.h"
#include "math.h"
#include "CalibrationCommon.h"

using namespace std;
FILE *ao_calibration_point_log_fd = NULL;
FILE *ai_calibration_point_log_fd = NULL;
FILE *ai_verification_log_fd = NULL;

static uint16_t g_xor_cal_data[MAX_CAL_CHANNELS][MAX_CODE_VALUES];
static uint16_t g_debug_cal_data[MAX_CAL_CHANNELS][MAX_CODE_VALUES];

static uint16_t g_ao_self_test_data[NUM_AO_CAL_POINTS] = { 0 };
static double g_ao_cal_point[NUM_AO_CAL_POINTS]  = { 0 };
static double g_ao_test_point[NUM_AO_TEST_POINTS + 1]  = { 0 };

static uint16_t g_ai_self_test_data[NUM_AI_CAL_POINTS] = { 0 };
static double g_ai_cal_point[NUM_AI_CAL_POINTS]  = { 0 };
static double g_ai_test_point[NUM_AI_TEST_POINTS]  = { 0 };

// Log a calibration point result
void log_cal_point_result(uint chn, vector<double> desired_voltage, vector<uint16_t> *dac_code_estimate, vector<double> residual_error, uint timeout_occurred, vector<DACChip *> *aout)
{
  int index;
  for(index = 0; index < dac_code_estimate->size(); index++)
  {
    FILE *fp = aout->at(index)->get_file_pointer();
    if( fp != NULL )
    {
      fprintf(fp, "%d, %f, %d, %f, %d\n", chn + (aout->at(index)->get_chip_id() * 8), desired_voltage.at(index), dac_code_estimate->at(index), residual_error.at(index), timeout_occurred);
      fflush(fp);
    }
  }
}

void log_cal_point_result(uint chn, double desired_voltage, vector<uint16_t> *dac_code_estimate, vector<double> residual_error, uint timeout_occurred, vector<DACChip *> *aout)
{
  int index;
  for(index = 0; index < dac_code_estimate->size(); index++)
  {
    FILE *fp = aout->at(index)->get_file_pointer();
    if( fp != NULL )
    {
      fprintf(fp, "%d, %f, %d, %f, %d\n", chn + (aout->at(index)->get_chip_id() * 8), desired_voltage, dac_code_estimate->at(index), residual_error.at(index), timeout_occurred);
      fflush(fp);
    }
  }
}

// Log a calibration point result
void aio_log_test_point_result(uint chn, double test_voltage, vector<double> daq_reading, vector<double> residual_error, vector<DACChip *> *aout, vector<ADCChip *> *ain)
{
  int index;
  int count;
  vector<vector<uint16_t> *> data;

  
  for(index = 0; index < aout->size(); index++)
  {
    // AOUT Logging
    FILE *aout_fp = aout->at(index)->get_file_pointer();
    if( aout_fp != NULL )
    {
      fprintf(aout_fp, "%d, %f, %f, %f\n", chn + (aout->at(index)->get_chip_id() * 8),  daq_reading.at(index), test_voltage, residual_error.at(index));
      fflush(aout_fp);
    }
    data.push_back(new vector<uint16_t>);
  }

  for(count = 0; count < NUM_AI_TEST_SAMPLES; count++)
  {
      for(index = 0; index < ain->size(); index++)
      {
        data.at(index)->push_back(ain->at(index)->get_ioChannel(chn).get_input());
      }
      usleep(1000);
  }

  for(index = 0; index < ain->size(); index++)
  {
    // AIN Logging
    FILE *ain_fp = ain->at(index)->get_file_pointer();
    if( ain_fp != NULL )
    {
      // "Channel, CommandedVoltage, MultiMeter, Data1, Data 2... Residual Error"
      fprintf(ain_fp, "%d,%f,%f,", chn + (ain->at(index)->get_chip_id() * 8), test_voltage, daq_reading.at(index));

      // Dump out all the data stored in the nested vectors
      for( count = 0; count < NUM_AI_TEST_SAMPLES; count++)
      {
      fprintf(ain_fp, "%d,", data.at(index)->at(count));
      }

      fprintf(ain_fp, "%f\n", residual_error.at(index));          
    }
  }
}

// Log a calibration point result
void log_cal_point_result_old(uint chn, double desired_voltage, uint dac_code_estimate, double residual_error, uint timeout_occurred)
{
  if ( ao_calibration_point_log_fd != NULL )
  {
    fprintf(ao_calibration_point_log_fd, "%d, %f, %d, %f, %d\n", chn, desired_voltage, dac_code_estimate, residual_error, timeout_occurred);
    fflush(ao_calibration_point_log_fd);
  }
}

int volts_to_quanta(double volts) {
  int quanta;

  quanta = nearbyint(volts * VOLTS_TO_QUANTA);
  return quanta;
}

double quanta_to_volts(int quanta) {
  return quanta / VOLTS_TO_QUANTA;
}

// Display a failure message and exit with an error code
void failmsg(const char *fmt, ...)
{
  va_list ap;

  printf("***** Error Message:\n");
  va_start(ap, fmt);
  vprintf(fmt, ap);
  va_end(ap);
  printf("Exiting the application");
  exit(EXIT_FAILURE);
}

// Generate Knot points with an optional alternating pattern
void gen_knot_points (double *point_array,  uint number_of_points, double point_size, int alternating_pattern)
{
  int count = 0;

  double current_val_neg = host_quanta_to_volts(HOST_QUANTA_MINUS_TEN);
  double current_val_pos = host_quanta_to_volts(HOST_QUANTA_POS_TEN);
  
  if ( alternating_pattern ) // Generate alternating positive and negative voltages
  {
    // Generate knot points from negative to positive
    for ( count = 0; count < number_of_points - 1; count += 2)
    {
      // Assign positive value
      point_array[count] = current_val_pos;
      current_val_pos -= point_size;
      // Assign negative value
      point_array[count+1] = current_val_neg;
      current_val_neg += point_size;
    }

  }
  else // Generate knot points from -10 to +9.99965 V
  {
    for ( count = 0; count < number_of_points; count++)
    {
      // Assign value
      point_array[count] = current_val_neg;
      // Increment from negative to positive
      current_val_neg += point_size;
    }    
  }

  // Insert a test point for 0 volts
  point_array[number_of_points] = HOST_QUANTA_ZERO;
}

// Generate analog output voltages
void gen_ao_test_points (uint enabling_alternating_pattern)
{
  int count;
  gen_knot_points(g_ao_test_point, NUM_AO_TEST_POINTS, AO_TEST_POINT_SIZE, enabling_alternating_pattern);
}


// Get an AO cal point based on array index
double get_ao_test_point (uint index)
{
  return g_ao_test_point[index];
}

// Wait for the card to warmup
void wait_for_n_seconds(ddi_fusion_instance_t *fusion_instance, int nsecs)
{
  int j;
  nsecs = nsecs;
}

void wait_for_card_to_warm_up(ddi_fusion_instance_t *fusion_instance, int warmup_time_in_seconds) 
{
  if (warmup_time_in_seconds > 0)
  {
    wait_for_n_seconds(fusion_instance, warmup_time_in_seconds);
  }
}

// Set Calibration data using DDI's XOR algorithm
void set_cal_data(uint chn, uint16_t index, uint16_t value)
{ 
  g_xor_cal_data[chn][(index)]=((value)^(index)^0xffff);
  g_debug_cal_data[chn][index]=value;
}

// Get Calibration data using DDI's XOR algorithm
uint16_t get_cal_data(uint16_t chn, uint16_t index)
{
  return g_xor_cal_data[chn][(index)]^(index)^0xffff;
}

// Initialize the data structures and open log files
void cal_init (void)
{
  int ch_count = 0;
  int index_count = 0;
  for ( ch_count = 0; ch_count < MAX_CAL_CHANNELS; ch_count++)
  {
    for ( index_count = 0; index_count < 65536; index_count++ )
    {
      g_xor_cal_data[ch_count][index_count] = 0xbeef;
      g_debug_cal_data[ch_count][index_count] = 0xbeef;
    }
  }
}

// Convert from volts to quanta using the host encoding (-10 V to 9.999695 V)
int host_volts_to_quanta(double volts)
{
  int quanta;
  
  quanta = nearbyint(volts * VOLTS_TO_QUANTA);
  return quanta;
}

// Convert from quanta to volts using the host encoding (-10 V to 9.999695 V)
double host_quanta_to_volts(int16_t quanta)
{
  return quanta / VOLTS_TO_QUANTA;
}
