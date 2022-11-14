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
#include "inttypes.h"
#include "DCUtil.h"
#include "DMMUtil.h"
#include <thread>
#include "ADCChip.h"
#include "DACChip.h"
#include "SlotCard.h"
#include "IOChannel.h"
#include <vector>

int pd_offset_out_array[6] = {0, 8, 40, 16, 32, 24};
int ch_16_pd_offset_out_array[6] = {0, 16, 80, 32, 64, 48};
string card_name_array[6] = {"0", "1", "5", "2", "4", "3"};

// Callback class to support the EtherCAT Master callback registration
class Multi_AIO_Test8 : public CallbackTest
{
  public:
    Multi_AIO_Test8 (AcontisTestFixture *p): CallbackTest(p) {}
    virtual void cyclic_method (void*);
};

// Placeholder for cyclic method, not used currently
void Multi_AIO_Test8::cyclic_method (void *arg)
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
class Multi_AIO_Test16 : public CallbackTest
{
  public:
    Multi_AIO_Test16 (AcontisTestFixture *p): CallbackTest(p) {}
    virtual void cyclic_method (void*);
};

// Placeholder for cyclic method, not used currently
void Multi_AIO_Test16::cyclic_method (void *arg)
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

/** aio_verification_multi
 @brief Verifies the calibrated AIN and AOUT cards by setting one channel of each AOUT card at a time to a pre-generated test_point value, wait for a short delay
 and read back the actual voltage value using the DAQ instance that is passed in. We also read back the AIN values simultaneously since they are 
 driven by the AOUT cards in a loopback system. The data we gathered from these operations are then logged.
 test_point values should range from -10 V to + 10 V, divided by NUM_AO_TEST_POINTS
 @param instance The Master instance handle to update
 @param num_channels The number of channels to calibrate
 @param is_signed Flag for Signed vs. Unsigned cards ( 0 for unsigned, 1 for signed )
 @param ain Pointer to a vector<ADCChip *>, this vector holds references to a number of ADC chip instances
 @param aout Pointer to a vector<DACChip *>, this vector holds references to a number of DAC chip instances
 @param daq Pointer to an instance of DAQ data acquisition system
 @return void Results are logged to files in the directory. Nothing is returned here.
 */
void aio_verification_multi(ddi_fusion_instance_t *instance, int num_channels, int is_signed, vector<ADCChip *> *ain, vector<DACChip *> *aout, DAQ *daq)
{
  int ch, j, chn_count, vector_index, volt_count =0, size = aout->size();
  double vindex;
  vector<double> daq_readings;
  vector<double> residual_errors;
  double hold_voltage;
  double verr;
  double current_test_voltage = AOUT_MAX_NEG_VOLTAGE;
  uint16_t ain_value;

  int test_repeat_count = 0;
  // Add repated tests for more data
  for ( test_repeat_count = 0 ; test_repeat_count < 1; test_repeat_count++ )
  {
    // Verify for the number of channels
    for (ch = 0; ch < num_channels; ch++)
    {
      uint last_test_iteration = 0;
      printf(RED "Verifying channel #%d " CLEAR " \n", ch);
      // Start at negative -10
      current_test_voltage = -10;

      uint channel_test_done = 0;
      uint toggle_off = 0;
      // check a range of voltages from <low> to <high>
      for ( volt_count = 0; volt_count < NUM_AO_TEST_POINTS; volt_count++ )
      {
        uint16_t aout_value;
        // set all channels to 0 at the start
        for (vector_index = 0; vector_index < aout->size(); vector_index++)
        {
          if( aout->at(vector_index)->get_cal_success_status() ) // if things are still good for this chip, then continue
          {
            for (chn_count = 0; chn_count < num_channels; chn_count++) {
              aout->at(vector_index)->get_ioChannel(chn_count).set_output(HOST_QUANTA_ZERO);
            }
          }
        }

        hold_voltage = get_ao_test_point(volt_count);

        // set the current channel of each AOUT card to its test voltage
        for (vector_index = 0; vector_index < aout->size(); vector_index++)
        {
          aout->at(vector_index)->get_ioChannel(ch).set_output(host_volts_to_quanta(hold_voltage));

          if( aout->size() <= 6 ) // less than or equal to 6 chips implies 8-channel
          {
            // Call on DAQ to prepare the necessary data to build a collection command string later
            int module_choice = vector_index / NUM_CARDS_PER_MODULE;
            int bank_choice = vector_index % NUM_CARDS_PER_MODULE;
            daq->add_channel(module_choice, bank_choice, ch, 0);
          }
          else
          {
            int module_choice = vector_index / (NUM_CARDS_PER_MODULE * 2);
            int bank_choice = vector_index % (NUM_CARDS_PER_MODULE * 2);
            daq->add_channel(module_choice, bank_choice, ch, 1);
          }
        }

        // Get the DAQ data
        daq->daq_collect_data_multi(0, vector_index);

        for(int daq_meter_data_index = 0; daq_meter_data_index < aout->size(); daq_meter_data_index++)
        {
          if(size > 6)
          {
            // Extract from daq's meter_data[] and push it onto daq_readings
            if( ch % 2 == 1 )
            {
              int choice_flag = daq_meter_data_index % 4;
              switch (choice_flag)
              {
              case 0: case 1:
                daq_readings.push_back(daq->get_meter_data(daq_meter_data_index + 2));
                break;
              case 2: case 3:
                daq_readings.push_back(daq->get_meter_data(daq_meter_data_index - 2));
              default:
                break;
              }
            }
            else
            {
              // Extract from daq's meter_data[] and push it onto daq_readings
              daq_readings.push_back(daq->get_meter_data(daq_meter_data_index));
            }
          }
          else
          {
            // Extract from daq's meter_data[] and push it onto daq_readings
            daq_readings.push_back(daq->get_meter_data(daq_meter_data_index));
          }

          double difference = daq_readings.at(daq_meter_data_index) - hold_voltage;
          if (fabs(difference) > RESIDUAL_ERROR_COMPARE_BAND)
          {
            aout->at(daq_meter_data_index)->set_cal_success_status(CAL_SUCESS_STATUS_FAILED);
          }
          
          residual_errors.push_back(difference);
        }
        
        // Log the channel, commanded voltage, DMM reading, residual error
        aio_log_test_point_result(ch, hold_voltage, daq_readings, residual_errors, aout, ain);
        daq->clear_meter_data_and_requests();
        residual_errors.clear();
        daq_readings.clear();
      }
    }
    printf("The following cards have failed calibration:\n");
    for(vector_index = 0; vector_index < size; vector_index++)
    {
      if(aout->at(vector_index)->get_cal_success_status() == CAL_SUCESS_STATUS_FAILED)
      {
        printf("Card #%d\n", vector_index / 2);
      }
    }
  }

  // clean up - set ao low.
  for(vector_index = 0; vector_index < aout->size(); vector_index++)
  {
    for (ch = 0; ch < num_channels; ch++)
    {
      aout->at(vector_index)->get_ioChannel(ch).set_output(HOST_QUANTA_ZERO);
    }
  }
}

// See aio_verification_multi for details
TEST_F(AcontisTestFixture, MultiAIOTest8)
{
  Multi_AIO_Test8 callBack(this);
  // Open Fusion Instance
  open_fusion(&callBack);
  // Check for fusion instance != NULL after calling open_fusion()
  ASSERT_EQ(fusion_instance==NULL, false) << "Returned Fusion interface is NULL, exiting\n";

  DAQ daq;

  // Enable Acontis license
  int ret = ddi_sdk_ecat_set_license_file((char *)"acontis_licenses.csv");

  // Connect to the DAQ
  daq.daq_connect("169.254.9.70");
  ASSERT_EQ(daq.get_daq_init_status(), 1) << "Failed to connect to DAQ\n";

  this->go_to_op_mode(CALIBRATION_PROJECT_VERSION);

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

  for( int i = 0; i < NUM_CARDS; i++ )
  {
    string ain_name = "ain_verification_";
    ain_name.append(card_name_array[i]);
    ain_name.append(".csv");

    string aout_name = "aout_verification_";
    aout_name.append(card_name_array[i]);
    aout_name.append(".csv");

    // Initializing AIN log file and setting header
    FILE *ain_fp = fopen(ain_name.c_str(), "w");
    fprintf(ain_fp, "#ain_verification_%d\n", i);
    fprintf(ain_fp, "#Time: %s\n", str.c_str());
    fprintf(ain_fp, "#Calibration Project Version: %s\n", CALIBRATION_PROJECT_VERSION);
    fprintf(ain_fp, "Channel,CommandedVoltage,MultiMeter,");
    int sample_count;
    for(sample_count = 0; sample_count < NUM_AI_TEST_SAMPLES -1; sample_count++)
    {
      fprintf(ain_fp, "Data%d,", sample_count);
    }
    // Terminate the calibration log file header
    fprintf(ain_fp, "Data%d\n", sample_count);
    adc_file_pointers.push_back(ain_fp);

    // Initializing AOUT log file and setting header
    FILE *aout_fp = fopen(aout_name.c_str(), "w");
    fprintf(aout_fp, "#aout_verification_%d\n", i);
    fprintf(aout_fp, "#Time: %s\n", str.c_str());
    fprintf(aout_fp, "#Calibration Project Version: %s\n", CALIBRATION_PROJECT_VERSION);
    fprintf(aout_fp, "Channel,CommandedVoltage,MultiMeter,ResidualError\n");
    dac_file_pointers.push_back(aout_fp);


    adc_chips.push_back(new ADCChip(fusion_instance, pd_offset_out_array[i], pd_offset_out_array[i], adc_file_pointers.at(i), 0));
    dac_chips.push_back(new DACChip(fusion_instance, pd_offset_out_array[i], pd_offset_out_array[i], dac_file_pointers.at(i), 0));
  }

  // Populate the g_ao_test_point array
  gen_ao_test_points(ENABLE_ALTERNATING_PATTERN);
  aio_verification_multi(fusion_instance, AIO_8_CHANNEL, TYPE_SIGNED, &adc_chips, &dac_chips, &daq);

  // Clear out the vectors
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

// See aio_verification_multi for details
TEST_F(AcontisTestFixture, MultiAIOTest16)
{
  Multi_AIO_Test16 callBack(this);
  // Open Fusion Instance
  open_fusion(&callBack);
  // Check for fusion instance != NULL after calling open_fusion()
  ASSERT_EQ(fusion_instance==NULL, false) << "Returned Fusion interface is NULL, exiting\n";

  DAQ daq;

  // Enable Acontis license
  int ret = ddi_sdk_ecat_set_license_file((char *)"acontis_licenses.csv");

  // Connect to the DAQ
  daq.daq_connect("169.254.9.70");
  ASSERT_EQ(daq.get_daq_init_status(), 1) << "Failed to connect to DAQ\n";

  this->go_to_op_mode(CALIBRATION_PROJECT_VERSION);

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

  for( int i = 0; i < NUM_CARDS; i++ )
  {
    string ain_name = "16_ch_ain_verification_";
    ain_name.append(card_name_array[i]);
    ain_name.append(".csv");

    string aout_name = "16_ch_aout_verification_";
    aout_name.append(card_name_array[i]);
    aout_name.append(".csv");
    
    // Initializing AIN log file and setting header
    FILE *ain_fp = fopen(ain_name.c_str(), "w");
    fprintf(ain_fp, "#Time: %s\n", str.c_str());
    fprintf(ain_fp, "#Calibration Project Version: %s\n", CALIBRATION_PROJECT_VERSION);
    fprintf(ain_fp, "Channel,CommandedVoltage,MultiMeter,");
    int sample_count;
    for(sample_count = 1; sample_count < NUM_AI_TEST_SAMPLES; sample_count++)
    {
      fprintf(ain_fp, "Data%d,", sample_count);
    }
    // Terminate the calibration log file header
    fprintf(ain_fp, "Data%d,ResidualError\n", sample_count);
    adc_file_pointers.push_back(ain_fp);

    // Initializing AOUT log file and setting header
    FILE *aout_fp = fopen(aout_name.c_str(), "w");
    fprintf(aout_fp, "#Time: %s\n", str.c_str());
    fprintf(aout_fp, "#Calibration Project Version: %s\n", CALIBRATION_PROJECT_VERSION);
    fprintf(aout_fp, "Channel,CommandedVoltage,MultiMeter,ResidualError\n");
    dac_file_pointers.push_back(aout_fp);

    for( int chip_count = 0; chip_count < 2; chip_count++)
    {
      adc_chips.push_back(new ADCChip(fusion_instance, ch_16_pd_offset_out_array[i], ch_16_pd_offset_out_array[i], adc_file_pointers.at(i), chip_count));
      dac_chips.push_back(new DACChip(fusion_instance, ch_16_pd_offset_out_array[i], ch_16_pd_offset_out_array[i], dac_file_pointers.at(i), chip_count));
    }

  }

  // Populate the g_ao_test_point array
  gen_ao_test_points(ENABLE_ALTERNATING_PATTERN);
  aio_verification_multi(fusion_instance, AIO_8_CHANNEL, TYPE_SIGNED, &adc_chips, &dac_chips, &daq);

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
    // Closing AIN file pointers and deleting them
    fclose( adc_file_pointers.at(index) );

    // Closing AOUT file pointers and deleting them
    fclose( dac_file_pointers.at(index) );
  }

  adc_file_pointers.clear();
  dac_file_pointers.clear();
  
  adc_chips.clear();
  dac_chips.clear();
}
