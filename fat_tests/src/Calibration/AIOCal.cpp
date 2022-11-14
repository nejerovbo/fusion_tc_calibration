/**************************************************************************
(c) Copyright 2022 Digital Dynamics Inc. Scotts Valley CA USA.
Unpublished copyright. All rights reserved. Contains proprietary and
confidential trade secrets belonging to DDI. Disclosure or release without
prior written authorization of DDI is prohibited.
**************************************************************************/

#include "AcontisTestFixture.h"
#include "CRAMProcessData.h"
#include "sys/socket.h"
#include "arpa/inet.h"
#include "stdio.h"
#include "stdbool.h"
#include "math.h"
#include "CalibrationCommon.h"
#include "DAQUtil.h"
#include "ADCChip.h"
#include "DACChip.h"
#include "SlotCard.h"
#include "IOChannel.h"
#include <thread>
#include <vector>
#include "string.h"

using namespace std;

extern AcontisTestFixture *g_fixture;
extern FILE* ai_calibration_point_log_fd;

int g_8_ch_pd_offset_out_array[6] = {0, 8, 40, 16, 32, 24};
int g_16_ch_pd_offset_out_array[6] = {0, 16, 80, 32, 64, 48};
string g_card_name_array[6] = {"0", "1", "5", "2", "4", "3"};

// Callback class to support the EtherCAT Master callback registration
class MULTI_AIO_CALIBRATION8 : public CallbackTest
{
  public:
    MULTI_AIO_CALIBRATION8 (AcontisTestFixture *p): CallbackTest(p) {}
    virtual void cyclic_method (void*);
};

// Placeholder for cyclic method, not used currently
void MULTI_AIO_CALIBRATION8::cyclic_method (void *arg)
{
  uint16_t current_state, requested_state;
  ddi_fusion_instance_t* fusion_instance;
  //check parameters
  if (!arg)
    return;

  //set the fusion interface from the argument to the cyclic data function
  fusion_instance = (ddi_fusion_instance_t *)arg;

  ddi_sdk_ecat_get_slave_state(fusion_instance->slave, &current_state, &requested_state);
  //don't process data unless the slave is in OP mode
  if ( current_state != DDI_ECAT_OP)
    return;
}

// Callback class to support the EtherCAT Master callback registration
class MULTI_AIO_CALIBRATION16 : public CallbackTest
{
  public:
    MULTI_AIO_CALIBRATION16 (AcontisTestFixture *p): CallbackTest(p) {}
    virtual void cyclic_method (void*);
};

// Placeholder for cyclic method, not used currently
void MULTI_AIO_CALIBRATION16::cyclic_method (void *arg)
{
  uint16_t current_state, requested_state;
  ddi_fusion_instance_t* fusion_instance;
  //check parameters
  if (!arg)
    return;

  //set the fusion interface from the argument to the cyclic data function
  fusion_instance = (ddi_fusion_instance_t *)arg;

  ddi_sdk_ecat_get_slave_state(fusion_instance->slave, &current_state, &requested_state);
  //don't process data unless the slave is in OP mode
  if ( current_state != DDI_ECAT_OP)
    return;
}

/** aio_uncal_quanta_to_volts
 @brief Converts a quanta code to voltage in double form
 @param quanta Input code
 @return double voltage
 */
double aio_uncal_quanta_to_volts(uint16_t quanta)
{
  double volt;
  volt = (double) ((quanta - 32768) / 3200.00);
  return volt;
}

// capture NUM_AI_CAL_SAMPLES and output the captures into output_array
/** read_ain
 @brief collects AIN data from each ADC chip's designated channel
 @param fusion_instance The Master instance handle to update
 @param ain Pointer to a vector<ADCChip *>, this vector holds references to a number of ADC chip instances
 @param channels Target channel to read data from
 */
void read_ain(ddi_fusion_instance_t *fusion_instance, vector<ADCChip *> *ain, uint channels)
{
  int sample_count, vector_size;
  uint16_t ain_value;
  for(sample_count = 0; sample_count < NUM_AI_CAL_SAMPLES; sample_count++)
  {
    for(vector_size = 0; vector_size < ain->size(); vector_size++)
    {
      ain_value = ain->at(vector_size)->get_ioChannel(channels).get_input();
      ain->at(vector_size)->set_output_array(sample_count, ain_value);
    }
    usleep(1000); // Delay 1 millisecond for the next train arrival
  }
}

/** aio_set_aout_channel_to_value
 @brief Set the same channel of each DAC chip to value designated by val. Read back voltage values with the DAQ and store into a vector<double> to be returned.
 If is_cal_point flag is set to true, then collect data for all chips and log to file
 @param instance The Master instance handle to update
 @param channel The target channel to be manipulated
 @param val A vector<uint16_t> that holds the specific code to be set for each AOUT card
 @param read_meter A flag to signal for DAQ operation. ( 0 for false, 1 for true )
 @param ain Pointer to a vector<ADCChip *>, this vector holds references to a number of ADC chip instances
 @param aout Pointer to a vector<DACChip *>, this vector holds references to a number of DAC chip instances
 @param daq Pointer to an instance of DAQ data acquisition system
 @param is_cal_point Flag ( defaults to false ) to signal for calibration logging
 @return vector<double> Voltage values for each chip are stored into a vector and returned
 */
vector<double> aio_set_aout_channel_to_value(ddi_fusion_instance_t *fusion_instance, uint channel, vector<uint16_t> val, uint read_meter, vector<ADCChip *> *ain, vector<DACChip *> *aout, DAQ *daq, bool is_cal_point = false)
{
  vector<double> voltage;
  thread *ai_collection;
  int sample_count, card_index;
  uint16_t output_array[NUM_AI_CAL_SAMPLES];
  vector<double> residual_error;
  int size = aout->size();
  // Drive channel of all aout cards to a voltage that val commands
  for( card_index = 0; card_index < size; card_index++ )
  {
    // Sets the AOUT
    aout->at(card_index)->get_ioChannel(channel).set_output(val.at(card_index));
    residual_error.push_back(0.0);  // For aout logging purpose only

    // size <= 6 means 8 channel cards
    if(size <= 6)
    {
      // Call on DAQ to prepare the necessary data to build a collection command string later
      int module_choice = (card_index) / NUM_CARDS_PER_MODULE;
      int bank_choice   = (card_index) % NUM_CARDS_PER_MODULE;
      daq->add_channel(module_choice, bank_choice, channel, 0);
    }
    else  // for 16 channel cards
    {
      int module_choice = (card_index) / (NUM_CARDS_PER_MODULE * 2);
      int bank_choice   = (card_index) % (NUM_CARDS_PER_MODULE * 2);
      daq->add_channel(module_choice, bank_choice, channel, 1);
    }
  }

  usleep (AO_PROPAGATION_DELAY_US);  // Propagation delay to drive value though EtherCAT

  // Only query the DAQ and AIN data when a read meter request is provided
  if ( read_meter )
  {
    // this flag triggers ain data collection and logging
    if ( is_cal_point )
    {
      // Read the Fusion AINs while the DAQ is being read
      ai_collection = new thread(read_ain, fusion_instance, ain, channel);
    }

    // Generates a data collection command to the DAQ and store the data into meter_data[]
    daq->daq_collect_data_multi(0, card_index);

    for(int daq_meter_data_index = 0; daq_meter_data_index < size; daq_meter_data_index++)
    {
      if(size > 6) // only perform the data swap for 16-channel calibration
      {
        if( channel % 2 == 1 )
        {
          int choice_flag = daq_meter_data_index % 4;
          switch (choice_flag)
          {
          case 0: case 1:
            voltage.push_back(daq->get_meter_data(daq_meter_data_index + 2));
            break;
          case 2: case 3:
            voltage.push_back(daq->get_meter_data(daq_meter_data_index - 2));
          default:
            break;
          }
        }
        else
        {
          // Extract from daq's meter_data[] and push it onto voltage
          voltage.push_back(daq->get_meter_data(daq_meter_data_index));
        }
      }
      else
      {
        // Extract from daq's meter_data[] and push it onto voltage
        voltage.push_back(daq->get_meter_data(daq_meter_data_index));
      }
    }

    // this flag triggers ain data collection and logging
    if ( is_cal_point )
    {
      // kick off the ai_collection thread
      ai_collection->join();

      // logging into .csv file
      for( card_index = 0; card_index < ain->size(); card_index++ )
      {
        ain->at(card_index)->write_to_log(channel, aio_uncal_quanta_to_volts(val.at(card_index)),voltage.at(card_index));
      }
      log_cal_point_result(channel, voltage, &val, residual_error, 0, aout);
    }
  }

  // Resets daq's internal records for this operation
  daq->clear_meter_data_and_requests();

  return voltage;
}

/** aio_find_best_quanta_for_voltage
 @brief Finds the best matching quanta value for desired_voltage for all chips
 Sets cal_chn to dac_code_estimate on each DAC chip and read back the voltage values with DAQ
 For each chip:
    Calculate the residual error with desired_voltage and compare against RESIDUAL_ERROR_COMPARE_BAND
    Calculate a new/better quanta value based on the residual error
    Set to new quanta value and read back voltage with DAQ
    Repeat until the residual error is less than RESIDUAL_ERROR_COMPARE_BAND
 @param instance The Master instance handle to update
 @param cal_chn The target channel to be manipulated
 @param desired_voltage Target voltage to match with quanta values
 @param dac_code_estimate Quanta value calculated from desired_voltage
 @param dac_code_found Pointer to a vector<uint16_t> that stores the best matching quanta value for each chip
 @param log_to_csv A flag that triggers AIN data gathering and AIO data logging
 @param ain Pointer to a vector<ADCChip *>, this vector holds references to a number of ADC chip instances
 @param aout Pointer to a vector<DACChip *>, this vector holds references to a number of DAC chip instances
 @param daq Pointer to an instance of DAQ data acquisition system
 @return int A function success signal
 */
int aio_find_best_quanta_for_voltage(ddi_fusion_instance_t *fusion_instance, int cal_chn, double desired_voltage, uint16_t dac_code_estimate,
                                   vector<uint16_t> *dac_code_found, uint log_to_csv, vector<ADCChip *> *ain, vector<DACChip *> *aout, DAQ *daq)
{
  int residual_band_compare_success = 0;
  int timeout_count = 10000, fine_adjustment_req = 0;
  vector<double> actual_voltage;
  vector<double> previous_actual_voltage;
  double compare_band;
  vector<double> residual_error;
  double best_residual_error = -100000;

  vector<uint16_t> current_dac_code_estimate;
  uint16_t previous_dac_code_estimate, best_dac_code_estimate;

#ifdef SELF_TEST 
  int count = 0;
  find_ao_self_test_data(desired_voltage, dac_code_found);
  return 0;
#endif
  for( int dac_count = 0; dac_count < aout->size(); dac_count++ )
  {
    current_dac_code_estimate.push_back(dac_code_estimate);
  }

  bool one_channel_out_of_band;
  while (timeout_count-- > 0) // If timeout count execeeds threshold then exit the search
  {
    one_channel_out_of_band = false;
    // Set the value and get a new reading

    actual_voltage = aio_set_aout_channel_to_value(fusion_instance, cal_chn, current_dac_code_estimate, PERFORM_METER_READING, ain, aout, daq);

    // Calculate the residual error (DMM - actual) for all readbacks
    int index;
    for( index = 0; index < actual_voltage.size(); index++ )
    {
      residual_error.push_back(actual_voltage.at(index) - desired_voltage);
    }
    // Compare the current residual error versus the best residual error found so far
    if ( fabs(residual_error.back()) <  fabs(best_residual_error) ) 
    {
      // Save off the best found dac_code_estimate in case there's a timeout comparing against the RESIDUAL_ERROR_COMPARE_BAND
      best_residual_error = residual_error.back();
      // current_dac_code_estimate.push_back(best_dac_code_estimate);
    }

    // See if the residual error is within the tolerance
    for( index = 0; index < residual_error.size(); index++ )
    {
      if(aout->at(index)->get_ioChannel(cal_chn).get_quanta_found() == 0)
      {
        if (fabs(residual_error.at(index)) < RESIDUAL_ERROR_COMPARE_BAND)
        {
          // Mark this AOUT card's designated channel as good to go, it will be ignored when we call aio_set_aout_channel_to_value() next time
          aout->at(index)->get_ioChannel(cal_chn).toggle_quanta_found();
          dac_code_found->at(index) = current_dac_code_estimate.at(index);
          // estimate was within band
        }
        else // Make rough adjustment
        {
          one_channel_out_of_band = true;
          uint16_t adjust = current_dac_code_estimate.at(index) + (uint16_t)nearbyint((desired_voltage - actual_voltage.at(index)) * 3276.8);
          current_dac_code_estimate.at(index) = adjust;
        }
      }
    }

    // If a quanta could not be found within timeout_count tries, return the best estimate
    if ( timeout_count <= 0 ) 
    {
      // quanta_found= 1;
      // Return the best dac code estimate found so far as the calibration point corresponding to the input voltage
      log_cal_point_result_old(cal_chn,desired_voltage,best_dac_code_estimate, best_residual_error, TIMEOUT_OCCURRED);

      // Compare the final residual error vs the secondary residual error band
      if ( fabs(residual_error.back()) > RESIDUAL_ERROR_COMPARE_BAND_SECONDARY )
      {
        failmsg("Timeout error occurred while searching for %f (V) on channel %d: residual error: %f \n", desired_voltage, cal_chn, residual_error);
      }
      break;
    }
    // if all channels are in band, then exit the while loop
    if( !one_channel_out_of_band )
    {
      break;
    }
    residual_error.clear();
  }

  if ( log_to_csv ) // Selectively enable logging to a CSV file
  {
      log_cal_point_result(cal_chn, desired_voltage, dac_code_found, residual_error, 0, aout);

      // spawn a thread to gather ain data, ain data is stored in output_array
      thread ai_collection(read_ain, fusion_instance, ain, cal_chn);
      // make it happen bb
      ai_collection.join();

      // loggin ain data
      for(int i = 0; i < ain->size(); i++)
      {
        ain->at(i)->write_to_log(cal_chn, desired_voltage, actual_voltage.at(i));
      }
  }
  return 0;
}

// Perform a volts to quanta in the DAC code domain
/**
 @brief Perform a volts to quanta conversion in the DAC code domain
 @param volts Voltage ( double ) to be converted
 @return uint16_t quanta code that represents the voltage in the DAC code domain 
*/
uint16_t aio_uncal_volts_to_quanta(double volts) {
  int quanta;
  quanta = round(volts * 3200) + 32768;
  return quanta;
}

/** aio_calibrate_single_ao_channel
 @brief Finds the true + 10 V and - 10 V value for all DAC chips.
 Calculates the size of each step of DAC code from - 10 V to + 10 V, walks through each step starting from - 10 V
 sets all DAC chip's targeted channel to code
 reads back voltage with DAQ, and log to calibration file
 @param instance The Master instance handle to update
 @param channel The target channel to be calibrated
 @param is_signed Flag for Signed vs. Unsigned cards ( 0 for unsigned, 1 for signed )
 @param ain Pointer to a vector<ADCChip *>, this vector holds references to a number of ADC chip instances
 @param aout Pointer to a vector<DACChip *>, this vector holds references to a number of DAC chip instances
 @param daq Pointer to an instance of DAQ data acquisition system
 @return void Nada
 */
void aio_calibrate_single_ao_channel(ddi_fusion_instance_t *instance, int channel, int is_signed, vector<ADCChip *> *ain, vector<DACChip *> *aout, DAQ *daq) 
{
  int size = aout->size();
  vector<uint16_t> dac_quanta_range(size);
  vector<uint16_t> dac_code_found(size);
  vector<uint16_t> neg_dac_code(size);
  vector<uint16_t> pos_dac_code(size);
  vector<uint16_t> pos_ten_quanta(size);
  vector<uint16_t> neg_ten_quanta(size);
  vector<uint16_t> zero_quanta(size);
  int              cal_point_count, negative_cal_count = 0, positive_cal_count = NUM_AO_CAL_POINTS - 1, ret, ai_cal_count;
  double           quanta_step;
  vector<double>   current_neg_quanta(size);
  vector<double>   current_pos_quanta(size);
  vector<double>   actual_voltage;

  printf(RED "Calibrating channel #%d " CLEAR " \n", channel);

  // Find 0 V, the return code will be in zero_quanta
  ret = aio_find_best_quanta_for_voltage(instance, channel, 0.0, aio_uncal_volts_to_quanta(0), &zero_quanta, LOG_CSV, ain, aout, daq);

  if ( ret != CALIBRATION_SUCCESS)
  {
    printf("An error occurred while finding %f \n", MAX_NEG_VOLTS);
    log_cal_point_result_old(channel,MAX_NEG_VOLTS,-1, -1, -1);
  }

  // Find the DAC code for -10 V, the return code will be in neg_ten_quanta
  ret = aio_find_best_quanta_for_voltage(instance, channel, MAX_NEG_VOLTS, aio_uncal_volts_to_quanta(MAX_NEG_VOLTS), &neg_ten_quanta, LOG_CSV, ain, aout, daq);
  if ( ret != CALIBRATION_SUCCESS)
  {
    printf("An error occurred while finding %f \n", MAX_NEG_VOLTS);
    log_cal_point_result_old(channel,MAX_NEG_VOLTS,-1, -1, -1);
  }

  // Find the DAC code for 9.999695 V, the return code will be in pos_ten_quanta
  ret = aio_find_best_quanta_for_voltage(instance, channel, MAX_POS_VOLTS, aio_uncal_volts_to_quanta(MAX_POS_VOLTS), &pos_ten_quanta, LOG_CSV, ain, aout, daq);
  if ( ret != CALIBRATION_SUCCESS)
  {
    printf("An error occurred while finding %f \n", MAX_NEG_VOLTS);
    log_cal_point_result_old(channel, MAX_NEG_VOLTS, -1, -1, -1);
  }
 
  // Set the DAC quanta range as the positive - negative DAC quanta
  int vector_index;
  for(vector_index = 0; vector_index < size; vector_index++)
  {
    dac_quanta_range.at(vector_index) = pos_ten_quanta.at(vector_index) - neg_ten_quanta.at(vector_index);
  }

  // Determine how many DAC quanta steps to step into
  quanta_step = dac_quanta_range.front()/(double)NUM_AO_CAL_POINTS;

  // Set the initial current negative quanta to -10 V
  for(vector_index = 0; vector_index < size; vector_index++)
  {
    current_neg_quanta.at(vector_index) = (double) neg_ten_quanta.at(vector_index);
    // Set the initial current negative quanta to 9.999695 V
    current_pos_quanta.at(vector_index) = (double) pos_ten_quanta.at(vector_index);
  }

  // Derive the quanta for each positive and negative point
  // (NUM_AO_CAL_POINTS-3) adjusts for -10 , +9.999965 V and 0 V being found with the meter
  for ( cal_point_count = 0 ; cal_point_count < (NUM_AO_CAL_POINTS-3)/2; cal_point_count++)
  {
    for(vector_index = 0; vector_index < size; vector_index++)
    {
      current_neg_quanta.at(vector_index) += quanta_step;  // Increment to the next more positive negative step
      current_pos_quanta.at(vector_index) -= quanta_step;  // Decrement to the next more negative positive step
      
      // Round to the nearest DAC code
      neg_dac_code.at(vector_index) = nearbyint(current_neg_quanta.at(vector_index));
      // Round to the nearest DAC code
      pos_dac_code.at(vector_index) = nearbyint(current_pos_quanta.at(vector_index));
    }

    // Set the negative value and get a new reading
    actual_voltage = aio_set_aout_channel_to_value(instance, channel, neg_dac_code, PERFORM_METER_READING, ain, aout, daq, ENABLE_CAL_LOG);

    // Set the positive value and get a new reading
    actual_voltage = aio_set_aout_channel_to_value(instance, channel, pos_dac_code, PERFORM_METER_READING, ain, aout, daq, ENABLE_CAL_LOG);

  }
}

/** calibrate
 @brief Warms up the cards. Finds the true 0 V value for every channel of every DAC chip.
 Calibrate one channel at a time and set other channels to their respective true 0 V value to reduce DC coupling
 @param instance The Master instance handle to update
 @param channels_to_calibrate The number of channels to calibrate
 @param is_signed Flag for Signed vs. Unsigned cards ( 0 for unsigned, 1 for signed )
 @param ain Pointer to a vector<ADCChip *>, this vector holds references to a number of ADC chip instances
 @param aout Pointer to a vector<DACChip *>, this vector holds references to a number of DAC chip instances
 @param daq Pointer to an instance of DAQ data acquisition system
 @return void Nada
 */
void calibrate(ddi_fusion_instance_t* instance, int channels_to_calibrate, int is_signed, vector<ADCChip *> *ain, vector<DACChip *> *aout, DAQ *daq)
{
  int ch_count, vector_size = aout->size();
  int zeroize_ch_count;
  int ret;
  uint16_t found_hex_code = 0;
  vector<uint16_t> zero_quanta(aout->size(), DAC_QUANTA_ZERO_EST_CODE);
  vector<uint16_t> dac_quanta_max_negative_code(vector_size, DAC_QUANTA_MAX_NEGATIVE_CODE);

  // Set all channels to -10 volts on startup and don't wait for a new reading
  for (ch_count = 0; ch_count < channels_to_calibrate; ch_count++)
  {
    aio_set_aout_channel_to_value(instance, ch_count, dac_quanta_max_negative_code, DISABLE_METER_READING, ain, aout, daq);
  }

  // Wait for the card to warmup, characterization shows a wait time of ~20 minutes is required for proper warmup
  if(WAIT_FOR_WARM_UP)
  {
    wait_for_card_to_warm_up(instance, WAIT_TIME_FOR_CARD_TO_POWERON);
  }

  // Set all channels to ~0 volts before calibration
  for (ch_count = 0; ch_count < channels_to_calibrate; ch_count++)
  {
    aio_set_aout_channel_to_value(instance, ch_count, zero_quanta, DISABLE_METER_READING, ain, aout, daq);
  }

  // Find the true zero quanta for each channel - in an effort to set all other channels to 0 to reduce DC coupling
  for (zeroize_ch_count = 0; zeroize_ch_count < channels_to_calibrate; zeroize_ch_count++)
  {
    // Find the DAC code for 0 V, the return code will be in zero_quanta
    // This will be used each time a channel is calibrated to set the value to the DAC code for 0 V
    ret = aio_find_best_quanta_for_voltage(instance, zeroize_ch_count, ZERO_VOLTS,
      aio_uncal_volts_to_quanta(0.0), &zero_quanta, DISABLE_LOG_CSV, ain, aout, daq);
    if ( ret != CALIBRATION_SUCCESS)
    {
      printf("An error occurred while finding %f \n", ZERO_VOLTS);
    }
  }

  if (is_signed) // Calibrate a signed card
  {
    for (ch_count = 0; ch_count < channels_to_calibrate; ch_count++) // For each channel to calibrate
    {
      for (zeroize_ch_count = 0; zeroize_ch_count < channels_to_calibrate; zeroize_ch_count++) // Set all channels to 0
      {
        // Set all the channels to an estimate around ~0 Volts
        aio_set_aout_channel_to_value(instance, zeroize_ch_count, zero_quanta, DISABLE_METER_READING, ain, aout, daq);
      }
      // Calibrate a single channel of a card with signed representation
      aio_calibrate_single_ao_channel(instance, ch_count, TYPE_SIGNED, ain, aout, daq);
    }
  }
  else
  {
    // Not supported yet for unsigned cards
    aio_calibrate_single_ao_channel(instance, ch_count, TYPE_UNSIGNED, ain, aout, daq);
  }
}

/** aio_init_cal_objects
 @brief Sets the Acontis license file (if available) to avoid time out after 45 minutes
 Establishes connection to the DAQ data acquisition system
 Brings fusion to OP mode
 @param tf The Acontis test fixture instance used to manipulate the fusion unit
 @param daq Pointer to an instance of DAQ data acquisition system
 @return void Nada
 */
void aio_init_cal_objects(AcontisTestFixture *tf, DAQ *daq)
{
  char meter_cmd[SEND_BUFF_SIZE], meter_reply[RECV_BUFF_SIZE];
  struct  sockaddr_in this_saddr;
  int  len, count, test_iteration;

  // Initialize calibration routines
  cal_init();

  int ret = ddi_sdk_ecat_set_license_file((char *)"acontis_licenses.csv");

  // Connect to the DAQ
  daq->daq_connect("169.254.9.70");

  // Go to OP mode
  tf->go_to_op_mode(CALIBRATION_PROJECT_VERSION);
}

TEST_F(AcontisTestFixture, Multi_AIOCal8)
{
  MULTI_AIO_CALIBRATION8 callBack(this);
  // Open Fusion Instance
  open_fusion(&callBack);
  // Check for fusion instance != NULL after calling open_fusion()
  ASSERT_EQ(fusion_instance==NULL, false) << "Returned Fusion interface is NULL, exiting\n";

  this->go_to_op_mode(CALIBRATION_PROJECT_VERSION);

  DAQ daq;

  // Initialize calibration objects
  aio_init_cal_objects(this, &daq);

  std::vector<ADCChip *> adc_chips;
  std::vector<DACChip *> dac_chips;
  std::vector<FILE *> adc_file_pointers;
  std::vector<FILE *> dac_file_pointers;

  // creating time info and putting into a string
  std::time_t t = std::time(nullptr);
  std::tm m_tm = *std::localtime(&t);

  ostringstream oss;
  oss << put_time(&m_tm, "%T %D");
  auto str = oss.str();
  
  for(int i = 0; i < NUM_CARDS; i++)
  {
    string ain_name = "8_ch_ain_log_";
    ain_name.append(g_card_name_array[i]);
    ain_name.append(".csv");

    string aout_name = "8_ch_aout_log_";
    aout_name.append(g_card_name_array[i]);
    aout_name.append(".csv");

    // Creating AIN log file and storing in vector<FILE *>
    FILE *fp = fopen(ain_name.c_str(), "w");
    if(fp == NULL)
    {
      printf("Error creating log file\n");
      exit(0);
    }

    // Printing timestamp and calibration project version
    fprintf(fp, "#Time: %s\n", str.c_str());
    fprintf(fp, "#Calibration Project Version: %s\n", CALIBRATION_PROJECT_VERSION);

    fprintf(fp, "Channel,Commanded Voltage,MultiMeter,");
    int sample_count;
    for(sample_count = 0; sample_count < NUM_AI_CAL_SAMPLES -1; sample_count++)
    {
      fprintf(fp, "Data%d,", sample_count);
    }
    // Terminate the calibration log file header
    fprintf(fp, "Data%d\n", sample_count);

    adc_file_pointers.push_back(fp);

    // Creating AOUT log file and storing in vector<FILE *>
    FILE *fp2 = fopen(aout_name.c_str(), "w");
    if(fp2 == NULL)
    {
      printf("Error creating log file\n");
      exit(0);
    }

    // Printing timestamp and calibration project version
    fprintf(fp, "#Time: %s\n", str.c_str());
    fprintf(fp, "#Calibration Project Version: %s\n", CALIBRATION_PROJECT_VERSION);

    fprintf(fp2,"Channel, MultiMeter, Data1, ResidualError, Timeout\n");
    dac_file_pointers.push_back(fp2);

    adc_chips.push_back(new ADCChip(fusion_instance, g_8_ch_pd_offset_out_array[i], g_8_ch_pd_offset_out_array[i], adc_file_pointers.at(i), 0));
    dac_chips.push_back(new DACChip(fusion_instance, g_8_ch_pd_offset_out_array[i], g_8_ch_pd_offset_out_array[i], dac_file_pointers.at(i), 0));
  }

  calibrate(fusion_instance, AIO_8_CHANNEL, TYPE_SIGNED, &adc_chips, &dac_chips, &daq);

  // Clean out the vectors
  for(auto i : adc_chips)
  {
    delete i;
  }

  for(auto j : dac_chips)
  {
    delete j;
  }
  adc_chips.clear();
  dac_chips.clear();
}

TEST_F(AcontisTestFixture, ReturnPartNumber)
{
  char serial[256];

  MULTI_AIO_CALIBRATION16 callBack(this);
  // Open Fusion Instance
  open_fusion(&callBack);

  get_PN(serial);
  printf("PartNumber=%s \n", serial);
}

TEST_F(AcontisTestFixture, Multi_AIOCal16)
{
  MULTI_AIO_CALIBRATION16 callBack(this);
  // Open Fusion Instance
  open_fusion(&callBack);
  // Check for fusion instance != NULL after calling open_fusion()
  ASSERT_EQ(fusion_instance==NULL, false) << "Returned Fusion interface is NULL, exiting\n";

  this->go_to_op_mode(CALIBRATION_PROJECT_VERSION);

  DAQ daq;

  // Initialize calibration objects
  aio_init_cal_objects(this, &daq);

  std::vector<ADCChip *> adc_chips;
  std::vector<DACChip *> dac_chips;
  std::vector<FILE *> adc_file_pointers;
  std::vector<FILE *> dac_file_pointers;

  // creating time info and putting into a string
  std::time_t t = std::time(nullptr);
  std::tm m_tm = *std::localtime(&t);

  ostringstream oss;
  oss << put_time(&m_tm, "%T %D");
  auto str = oss.str();

  int card_index;
  for(card_index = 0; card_index < NUM_CARDS; card_index++)
  {
    string ain_name = "16_ch_ain_log_";
    ain_name.append(g_card_name_array[card_index]);
    ain_name.append(".csv");

    string aout_name = "16_ch_aout_log_";
    aout_name.append(g_card_name_array[card_index]);
    aout_name.append(".csv");

    // Creating AIN log file and storing in vector<FILE *>
    FILE *fp = fopen(ain_name.c_str(), "w");
    if(fp == NULL)
    {
      printf("Error creating log file\n");
      exit(0);
    }

    // Printing timestamp and calibration project version
    fprintf(fp, "#Time: %s\n", str.c_str());
    fprintf(fp, "#Calibration Project Version: %s\n", CALIBRATION_PROJECT_VERSION);

    fprintf(fp, "Channel,Commanded Voltage,MultiMeter,");
    int sample_count;
    for(sample_count = 0; sample_count < NUM_AI_CAL_SAMPLES -1; sample_count++)
    {
      fprintf(fp, "Data%d,", sample_count);
    }
    // Terminate the calibration log file header
    fprintf(fp, "Data%d\n", sample_count);

    adc_file_pointers.push_back(fp);

    // Creating AOUT log file and storing in vector<FILE *>
    FILE *fp2 = fopen(aout_name.c_str(), "w");
    if(fp2 == NULL)
    {
      printf("Error creating log file\n");
      exit(0);
    }

    // Printing timestamp and calibration project version
    fprintf(fp, "#Time: %s\n", str.c_str());
    fprintf(fp, "#Calibration Project Version: %s\n", CALIBRATION_PROJECT_VERSION);

    fprintf(fp2,"Channel, MultiMeter, Data1, ResidualError, Timeout\n");
    dac_file_pointers.push_back(fp2);

    // Populate each card with 2 chips
    for(int chip_index = 0; chip_index < 2; chip_index++)
    {
      adc_chips.push_back(new ADCChip(fusion_instance, g_16_ch_pd_offset_out_array[card_index], g_16_ch_pd_offset_out_array[card_index], adc_file_pointers.at(card_index), chip_index));
      dac_chips.push_back(new DACChip(fusion_instance, g_16_ch_pd_offset_out_array[card_index], g_16_ch_pd_offset_out_array[card_index], dac_file_pointers.at(card_index), chip_index));
    }
  }
  
  calibrate(fusion_instance, AIO_8_CHANNEL, TYPE_SIGNED, &adc_chips, &dac_chips, &daq);

  // Clear out the vectors
  for(auto i : adc_chips)
  {
    delete i;
  }

  for(auto j : dac_chips)
  {
    delete j;
  }

  for(int index = 0; index < adc_file_pointers.size(); index++)
  {
    // Closing AIN file pointers
    fclose( adc_file_pointers.at(index) );

    // Closing AOUT file pointers
    fclose( dac_file_pointers.at(index) );
  }

  adc_file_pointers.clear();
  dac_file_pointers.clear();
  
  adc_chips.clear();
  dac_chips.clear();
}
