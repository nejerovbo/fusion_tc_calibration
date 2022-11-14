/**************************************************************************
(c) Copyright 2022 Digital Dynamics Inc. Scotts Valley CA USA.
Unpublished copyright. All rights reserved. Contains proprietary and
confidential trade secrets belonging to DDI. Disclosure or release without
prior written authorization of DDI is prohibited.
**************************************************************************/

#ifndef CALIBRATION_COMMON_H
#define CALIBRATION_COMMON_H

#define CALIBRATION_PROJECT_VERSION "1.1.0"

#define HOST_QUANTA_MINUS_TEN 0x8000 // -10 V
#define HOST_QUANTA_POS_TEN 0x7FFF // 9.999695

#define DAC_QUANTA_MAX_NEGATIVE_CODE 0 // Estimate for 0 volts on an uncalibrated aout card
#define DAC_QUANTA_ZERO_EST_CODE 0x8000 // Estimate for 0 volts on an uncalibrated aout card

#define RESIDUAL_ERROR_COMPARE_BAND 0.000200  // In Volts
#define RESIDUAL_ERROR_COMPARE_BAND_SECONDARY 0.000450  // In Volts

#define VOLTS_TO_QUANTA  3276.8
#define MAX_NEG_QUANTA  -32768
#define MAX_POS_QUANTA   32767
#define MAX_NEG_VOLTS   -10.000
#define MAX_POS_VOLTS    9.999695
#define ZERO_VOLTS       0.0f
#define HOST_QUANTA_ZERO 0 
#define CALIBRATION_TYPE "-10v to 9.9996, volts == quanta / 3276.8"

#define TYPE_SIGNED                   1
#define TYPE_UNSIGNED                 0
#define DISABLE_METER_READING         0  // Disable a meter reading
#define PERFORM_METER_READING         1  // Trigger an immediate meter reading
#define PERFORM_AIN_READING           1  // Trigger an AIN reading
#define DISABLE_AIN_READING           0  // Disable an AIN reading
#define CAL_SUCESS_STATUS_FAILED      0
#define NUM_CARDS 6
#define NO_TIMEOUT       0
#define TIMEOUT_OCCURRED 1
#define DISABLE_LOG_CSV  0 // Disable logging of quanta values for initial sets to 0
#define LOG_CSV          1
#define ENABLE_CAL_LOG true
#define DAQ_8_CHANNEL_OFFSET 101
#define DAQ_16_CHANNEL_OFFSET 109
#define AIO_8_CHANNEL 8
#define AIO_16_CHANNEL 16
#define WAIT_FOR_WARM_UP 0
#define NUM_CARDS_PER_MODULE 2

#define AOUT_MAX_POS_VOLTAGE    (9.99969f)
#define AOUT_MAX_NEG_VOLTAGE    (-10.0f)
#define DAQ_AOUT_CHANNEL_OFFSET (8)
#define DAQ_MAX_CHANNELS        (24)


#define MAX_CAL_CHANNELS              16 // Maximum number of channels supported

#define WAIT_TIME_FOR_CARD_TO_POWERON (20 * 60)

#define NUM_AO_CAL_POINTS             100
#define NUM_AO_TEST_POINTS            100 
#define NUM_AO_SELF_TEST_POINTS       21

#define AO_CAL_POINT_SIZE             (20.0f/(NUM_AO_CAL_POINTS-1))
#define AO_TEST_POINT_SIZE            (20.0f/(NUM_AO_TEST_POINTS-1))

#define NUM_AI_CAL_POINTS             640 
#define NUM_AI_TEST_POINTS            512
#define NUM_AI_SELF_TEST_POINTS       21
#define NUM_AI_CAL_SAMPLES            60 
#define NUM_AI_TEST_SAMPLES           100
#define AIN_TEST_MODE                 1
#define AIN_CAL_MODE                  0

#define AI_CAL_POINT_SIZE             (20.0f/(NUM_AI_CAL_POINTS-1))
#define AI_TEST_POINT_SIZE            (20.0f/(NUM_AI_TEST_POINTS-1))

// Controls pattern generation
#define DISABLE_ALTERNATING_PATTERN   0
#define ENABLE_ALTERNATING_PATTERN    1

#define CALIBRATION_SUCCESS           0

#define MAX_AO_CODE_SCALE             65536
#define MAX_CODE_VALUES               65536

// Process data mapping for RIM5 and RIM1
#define FUSION_CRAM_RIM5_J1_J2_AIN         128
#define FUSION_CRAM_RIM5_J1_J2_AOUT        80 
#define FUSION_CRAM_RIM1_J1_AOUT           0       // for 8 channel
#define FUSION_CRAM_RIM1_J1_AIN            0

#define DAQ_8_CHANNEL_OFFSET               101
#define DAQ_16_CHANNEL_OFFSET              109
#define AIO_8_CHANNEL                      8
#define AIO_16_CHANNEL                     16

// Define the value for Keysight PLC value:
// https://www.keysight.com/us/en/lib/resources/training-materials/adjusting-nplc-and-aperture-to-make-high-speed-measurements.html
#define NUMBER_OF_PLCS                1.0

// Define the value for a single PLC
#define PLC_TIME_MS                  (1000.0f/60.0f)

// Convert the PLC values to microseconds, to get the most accurate DMM readings, wait this period before performing a DMM read
#define AO_PROPAGATION_DELAY_US      (70 * 1000) // 100 milliseconds

// IO position of the data aquistion to read inputs from
#define DAQ_IO_CHN_START     101

#include "AcontisEnvironment.h"
#include "AcontisTestFixture.h"
#include "DACChip.h"
#include <vector>
#include "IOChannel.h"
#include "SlotCard.h"
#include "ADCChip.h"
#include "DAQUtil.h"
#include "stdio.h"

class DACChip;
class ADCChip;

// Perform a linear interpolation to host code range from device code range (device_quanta_hi-device_quanta_lo)
void wait_for_card_to_warm_up(ddi_fusion_instance_t *fusion_instance, int warmup_time_in_seconds);

// Perform a linear interpolation
void perform_linear_interpolation(int chn, int dac_quanta_lo, int dac_quanta_hi, double volts_lo, double volts_hi);

// Initialize the calibration system
void cal_init (void);

// De-initialize the calibration system
void cal_finish (void);

// Write cal data to a file
void write_cal_data_to_file(int num_channels);

// Write new cal data to an internal array
void set_cal_data(uint chn, uint16_t index, uint16_t value);

// Retreive cal data from an internal array
uint16_t get_cal_data(uint16_t chn, uint16_t index);

// Log verification data to file
void aio_log_test_point_result(uint chn, double test_voltage, vector<double> daq_reading, vector<double> residual_error, vector<DACChip *> *aout, vector<ADCChip *> *ain);

// Log calibration results to a file
void log_cal_point_result(uint chn, vector<double> desired_voltage, vector<uint16_t> *dac_code_estimate, vector<double> residual_error, uint timeout_occurred, vector<DACChip *> *aout);

void log_cal_point_result(uint chn, double desired_voltage, vector<uint16_t> *dac_code_estimate, vector<double> residual_error, uint timeout_occurred, vector<DACChip *> *aout);

void log_cal_point_result_old(uint chn, double desired_voltage, uint dac_code_estimate, double residual_error, uint timeout_occurred);

// Convert from volts to quanta using the host encoding (-10 V to 9.999695 V)
int host_volts_to_quanta(double volts);

// Convert from quanta to volts using the host encoding (-10 V to 9.999695 V)
double host_quanta_to_volts(int16_t quanta);

// Get a calibration point in volts
double get_ao_cal_point (uint index);

// Get a calibration test point in volts
double get_ao_test_point (uint index);

// Load test pattern from CSV
int DAC_load_csv();

// Generate AO calibration data
void gen_ao_test_points (uint enabling_alternating_pattern);

// Display an error message and ex
void failmsg(const char *fmt, ...);

#endif
