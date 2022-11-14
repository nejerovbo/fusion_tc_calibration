// For console output and gflag testing
#include "AcontisTestFixture.h"
#include "CRAMProcessData.h"
#include "DDITestCommon.h"

using namespace std;

// Provides a test for analog input latency
// Reads an AIN through EtherCAT and routes it to an AOUT

#define PROJECT_NAME "Analog Input Latency EtherCAT Test"
#define LOGFILE_NAME "ain_input_latency.csv"
#define PROJECT_VERSION "v1.0.0"
#define ONE_SEC_DELAY_MS 1000

// Callback definition that supports the Fusion as an output device
class AI_IO : public CallbackTest
{
  public:
    AI_IO (AcontisTestFixture *p): CallbackTest(p) {}
    virtual void cyclic_method (void*);
};

// Callback definition that supports the EL4123 as an output device
class AI_IO_BECKHOFF : public CallbackTest
{
  public:
    AI_IO_BECKHOFF (AcontisTestFixture *p): CallbackTest(p) {}
    virtual void cyclic_method (void*);
};

// Callback definition that supports the Fusion as an output device
void AI_IO::cyclic_method (void *arg)
{
  uint16_t ain_position, aout_position;
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

  // Read from the first channel of the 12-slot AIN
  ain_position = CRAM_6_SLOT_AIN_START;
  // Write the first channel of the 12-slot AOUT
  aout_position = CRAM_6_SLOT_AOUT_START;
  uint16_t ain_value;
  ain_value=ddi_sdk_fusion_get_ain(local_fusion_instance, ain_position);
  // Write the ain data to the log file
  m_AcontisTestFixture->log_write(true, "0x%04x, 0x%04x \n", ain_value, ain_value);
  // Route the aout back to an out
  ddi_sdk_fusion_set_aout(local_fusion_instance, aout_position, ain_value);
}

// Callback definition that supports the EL4123 as an output device
void AI_IO_BECKHOFF::cyclic_method (void *arg)
{
  uint16_t ain_position, aout_position;
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
  ain_position = CRAM_6_SLOT_AIN_START;
  uint16_t ain_value;
  ain_value=ddi_sdk_fusion_get_ain(local_fusion_instance, ain_position);

  // Set Beckhoff AOUT
  uint16_t *aout = (uint16_t *)(uint8_t *)&pd_out_ptr[BECKHOFF_AOUT_OFFSET];
  int count;
  count = 0;
  uint8_t *ptr = &pd_out_ptr[BECKHOFF_AOUT_OFFSET];

  *aout = ain_value;

  static int log_count = 0;

  log_count++;
  if ( (log_count > 10) && (log_count < 1010))
  {
    // Add data to CSV
    m_AcontisTestFixture->log_write(true, "%.04f, %0.4f \n", \
    m_AcontisTestFixture->quanta_to_volts(ain_value), m_AcontisTestFixture->quanta_to_volts(*aout));
  }
}

// Tests from the original main.cpp under here
// These tests will not make fatal exit yet for testing purposes
TEST_F(AcontisTestFixture, DVT_AINLatency)
{
  AI_IO callBack(this);
  // Open Fusion Instance
  AcontisTestFixture::open_fusion(&callBack);
  ASSERT_EQ(fusion_instance==NULL, false) << "Returned Fusion interface is NULL, exiting\n";

  // Open logfile for write
  AcontisTestFixture::log_open(LOGFILE_NAME, "w");
  // Get sync and train mode, add them to the CSV
  string sync_mode;
  sync_mode = get_sync_mode(fusion_instance);
  string train_rate;
  train_rate = get_train_rate();
  if ( sync_mode == "sync" )
  {
    AcontisTestFixture::log_write(true, "Sync Mode: SM-Synchron\n");
  }
  else
  {
    AcontisTestFixture::log_write(true, "Sync Mode: Free-Run\n");
  }

  if ( train_rate == "1000" )
  {
    AcontisTestFixture::log_write(true, "Cyclic Data Rate: 1kHz\n");
  }
  else
  {
    AcontisTestFixture::log_write(true, "Cyclic Data Rate: 500Hz\n");
  }

  //AcontisTestFixture::set_sync_mode(fusion_instance, SYNC_MODE_FREE_RUN);
  //set_sync_mode(fusion_instance, SYNC_MODE_SM_SYNCHRON);

  // Go to OP mode
  AcontisTestFixture::go_to_op_mode(PROJECT_VERSION);

  // Clear the screen
  clrscr();

  printf("Welcome to %s version: %s\n", PROJECT_NAME, PROJECT_VERSION);

  // Sleep for 60 seconds
  AcontisTestFixture::poll_EtherCAT(SEC_PER_MIN*60,ONE_SEC_DELAY_MS);
}

// Tests from the original main.cpp under here
// These tests will not make fatal exit yet for testing purposes
TEST_F(AcontisTestFixture, DVT_AINLatency_Beckhoff)
{
  AI_IO_BECKHOFF callBack(this);
  // Open Fusion Instance
  AcontisTestFixture::open_fusion(&callBack);
  ASSERT_EQ(fusion_instance==NULL, false) << "Returned Fusion interface is NULL, exiting\n";

  // Open logfile for write
  AcontisTestFixture::log_open(LOGFILE_NAME, "w");
  // Get sync and train mode, add them to the CSV
  AcontisTestFixture::log_csv_header();

  //AcontisTestFixture::set_sync_mode(fusion_instance, SYNC_MODE_FREE_RUN);
  set_sync_mode(fusion_instance, SYNC_MODE_SM_SYNCHRON);

  // Go to OP mode
  AcontisTestFixture::go_to_op_mode(PROJECT_VERSION);

  // Clear the screen
  clrscr();

  printf("Welcome to %s version: %s\n", PROJECT_NAME, PROJECT_VERSION);

  // Sleep for 60 seconds
  AcontisTestFixture::poll_EtherCAT(SEC_PER_MIN*60,ONE_SEC_DELAY_MS);
}
