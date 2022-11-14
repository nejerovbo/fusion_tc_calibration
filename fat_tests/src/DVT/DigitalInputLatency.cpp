// For console output and gflag testing
#include "AcontisTestFixture.h"
#include "CRAMProcessData.h"
#include "DDITestCommon.h"

using namespace std;

// Provides a test for digital input latency
// Reads an AIN through EtherCAT and routes it to an AOUT

#define PROJECT_NAME "Digital Input Latency EtherCAT Test"
#define LOGFILE_NAME "din_input_latency.csv"
#define PROJECT_VERSION "v1.0.0"
#define ONE_SEC_DELAY_MS 1000

// Callback definition that supports the Fusion as an output device
class DI_IO : public CallbackTest
{
  public:
    DI_IO (AcontisTestFixture *p): CallbackTest(p) {}
    virtual void cyclic_method (void*);
};

// Callback definition that supports the Beckhoff EL4123 as an output device
class DI_IO_BECKHOFF : public CallbackTest
{
  public:
    DI_IO_BECKHOFF (AcontisTestFixture *p): CallbackTest(p) {}
    virtual void cyclic_method (void*);
};

// Digital input latency test that uses the Fusion as an output device
void DI_IO::cyclic_method (void *arg)
{
  uint16_t din_position, dout_position;
  uint16_t current_state, requested_state, input_value;
  ddi_fusion_instance_t* fusion_instance;
  // Validate  parameters
  if (!arg)
    return;

  // set the fusion interface from the argument to the cyclic data function
  fusion_instance = (ddi_fusion_instance_t *)arg;

  ddi_sdk_ecat_get_slave_state(fusion_instance->slave,&current_state,&requested_state);
  // don't process data unless the slave is in OP mode
  if ( current_state != DDI_ECAT_OP)
    return;

  // Read from the first channel of the 12-slot AIN
  din_position = CRAM_RIM_5_DIN_START;
  // Write the first channel of the 12-slot AOUT
  dout_position = CRAM_RIM_5_DOUT_START;
  uint16_t din_value;
  uint16_t length;
  din_value=ddi_sdk_fusion_get_din(fusion_instance, din_position, &length);
  printf("din value 0x%04x \n", din_value);
  // Write the din data to the log file
  m_AcontisTestFixture->log_write(false, "0x%04x, 0x%04x \n", din_value, din_value);
  // Route the din back to an dout
  ddi_sdk_fusion_set_dout16(fusion_instance, dout_position, din_value);
}

// Digital input latency test that uses the EL4123 as an output device
#define DIN_CH1 1
// Test the DI latency by using the beckhoff AO as the output to the o-scope
void DI_IO_BECKHOFF::cyclic_method (void *arg)
{
  uint16_t din_position, aout_position;
  uint16_t current_state, requested_state, input_value;
  ddi_fusion_instance_t* local_fusion_instance;

  // Validate parameters
  if (!arg)
    return;

  // set the fusion interface from the argument to the cyclic data function
  local_fusion_instance = (ddi_fusion_instance_t*)arg;

  ddi_sdk_ecat_get_slave_state(local_fusion_instance->slave,&current_state,&requested_state);
  // don't process data unless the slave is in OP mode
  if ( current_state != DDI_ECAT_OP)
    return;

  uint8_t *pd_out_ptr = local_fusion_instance->slave->pd_output;
  memset(pd_out_ptr,0, 800);

  // Read from the first channel of the 12-slot AIN
  din_position = CRAM_RIM_5_DIN_START;
  uint16_t din_value;
  uint16_t din_length;
  din_value=ddi_sdk_fusion_get_din(local_fusion_instance, din_position, &din_length);

#ifdef USE_AOUT
  // Set Beckhoff AOUT
  uint16_t *aout = (uint16_t *)(uint8_t *)&pd_out_ptr[BECKHOFF_AOUT_OFFSET];
  int count;
  count = 0;
  uint8_t *ptr = &pd_out_ptr[BECKHOFF_AOUT_OFFSET];

  if ( din_value & DIN_CH1 )
  {
    *aout = 0x7FFF;
  }
  else
  {
    *aout = 0;
  }
#else
  // Set Beckhoff AOUT
  uint16_t *aout = (uint16_t *)(uint8_t *)&pd_out_ptr[847];
  int count;
  count = 0;
  uint8_t *ptr = &pd_out_ptr[847];

  if ( din_value & DIN_CH1 )
  {
    *aout = 0x7FFF;
  }
  else
  {
    *aout = 0;
  }
#endif

  static int log_count = 0;

  log_count++;
  if ( (log_count > 10) && (log_count < 1010))
  {
    // Add data to CSV
    m_AcontisTestFixture->log_write(true, "0x%04x, %0.4f \n", \
    din_value & DIN_CH1, m_AcontisTestFixture->quanta_to_volts(*aout));
  }
}

// Tests from the original main.cpp under here
// These tests will not make fatal exit yet for testing purposes
TEST_F(AcontisTestFixture, DVT_DINLatency)
{
  DI_IO callBack(this);
  // Open Fusion Instance
  open_fusion(&callBack);
  ASSERT_EQ(fusion_instance==NULL, false) << "Returned Fusion interface is NULL, exiting\n";

  set_sync_mode(fusion_instance, SYNC_MODE_FREE_RUN);
  //set_sync_mode(fusion_instance, SYNC_MODE_SM_SYNCHRON);

  // Open logfile for write
  AcontisTestFixture::log_open(LOGFILE_NAME, "w");
  AcontisTestFixture::log_write(true,"Digital Latency Test\n");
  AcontisTestFixture::log_write(true, "Sync Mode: Free-run at 0.5 kHz\n");
  AcontisTestFixture::log_write(true, "DIN_Value, DOUT_Value \n");

  // Go to OP mode
  go_to_op_mode(PROJECT_VERSION);

  // Clear the screen
  clrscr();

  printf("Welcome to %s version: %s\n", PROJECT_NAME, PROJECT_VERSION);

  // Sleep for 60 seconds
  AcontisTestFixture::poll_EtherCAT(20,ONE_SEC_DELAY_MS);
}

// Tests from the original main.cpp under here
// These tests will not make fatal exit yet for testing purposes
TEST_F(AcontisTestFixture, DVT_DINLatency_Beckhoff)
{
  DI_IO_BECKHOFF callBack(this);
  // Open Fusion Instance
  AcontisTestFixture::open_fusion(&callBack);
  ASSERT_EQ(fusion_instance==NULL, false) << "Returned Fusion interface is NULL, exiting\n";

  // Open logfile for write
  AcontisTestFixture::log_open(LOGFILE_NAME, "w");
  // Get sync and train mode, add them to the CSV
  AcontisTestFixture::log_csv_header();

  set_sync_mode(fusion_instance);

  // Go to OP mode
  AcontisTestFixture::go_to_op_mode(PROJECT_VERSION);

  // Clear the screen
  clrscr();

  printf("Welcome to %s version: %s\n", PROJECT_NAME, PROJECT_VERSION);

  // Sleep for 60 seconds
  AcontisTestFixture::poll_EtherCAT(SEC_PER_MIN*60,ONE_SEC_DELAY_MS);
}
