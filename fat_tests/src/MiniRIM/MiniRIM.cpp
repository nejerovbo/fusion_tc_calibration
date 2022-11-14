// For console output and gflag testing
#include "AcontisTestFixture.h"
#include "CRAMProcessData.h"
#include <fcntl.h>
#include <termios.h>
#include "MiniRIM.h"

using namespace std;
// Log file
#define LOGFILE_NAME "MiniRIM.csv"
// Project Name and Version
#define PROJECT_NAME "MiniRIM_test"
#define PROJECT_VERSION "v1.0.0"
#define ONE_SEC_DELAY_MS 1000

// Supports G-test callback from the Acontis SDK
class MiniRIM : public CallbackTest
{
  public:
    MiniRIM (AcontisTestFixture *p): CallbackTest(p) {}
    virtual void cyclic_method (void*);
};

// Cyclic callback for the MiniRIM test
void MiniRIM::cyclic_method (void *arg)
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


// Tests from the original main.cpp under here
// These tests will not make fatal exit yet for testing purposes
TEST_F(AcontisTestFixture, MiniRIM_Test)
{
  // Callback mechanism
  MiniRIM callBack(this);

  // Open Fusion Instance and register callback
  AcontisTestFixture::open_fusion(&callBack);
  ASSERT_EQ(fusion_instance==NULL, false) << "Returned Fusion interface is NULL, exiting\n";

  // Open logfile for write
  AcontisTestFixture::log_open(LOGFILE_NAME, "w");
  AcontisTestFixture::log_write(true, "MiniRIM Test\n");

  // Use the sync mode specified by the start script
  AcontisTestFixture::set_sync_mode(fusion_instance);

  // Go to OP mode
  AcontisTestFixture::go_to_op_mode(PROJECT_VERSION);

  // Clear the screen
  clrscr();

  printf("Welcome to %s version: %s\n", PROJECT_NAME, PROJECT_VERSION);

  // Wait for 50 minutes
  AcontisTestFixture::poll_EtherCAT(50 * SEC_PER_MIN,ONE_SEC_DELAY_MS);
}