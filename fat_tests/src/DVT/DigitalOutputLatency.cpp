// For console output and gflag testing
#include "AcontisTestFixture.h"
#include "CRAMProcessData.h"

using namespace std;

/*
 * Objective:
 * Provide a test for Digital output latency measured using the o-scope.
 * Requirements:
 * 1) CRAM unit
 * Test Procecure:
 * 1) Toggle DOUTs between 0 and 0xFFFF every 1kHz.
 * 2) Measure Response on o-scope
 */

#define PROJECT_NAME "Digital Output Latency EtherCAT_test"
#define PROJECT_VERSION "v1.0.0"
#define ONE_SEC_DELAY_MS 1000

class DO_IO : public CallbackTest
{
  public:
    DO_IO (AcontisTestFixture *p): CallbackTest(p) {}
    virtual void cyclic_method (void*);
};

void DO_IO::cyclic_method (void *arg)
{
  static int pattern_count = 0, toggle_on = 1, output_value = 0, count;
  uint16_t current_state, requested_state, input_value;
  ddi_fusion_instance_t* fusion_instance;
  // Validate parameters
  if (!arg)
    return;

  // set the fusion interface from the argument to the cyclic data function
  fusion_instance = (ddi_fusion_instance_t *)arg;

  ddi_sdk_ecat_get_slave_state(fusion_instance->slave,&current_state,&requested_state);
  // don't process data unless the slave is in OP mode
  if ( current_state != DDI_ECAT_OP)
    return;

  if ( toggle_on == 1)
  {
    toggle_on = 0;
    output_value = 0xFFFF;
  }
  else
  {
    toggle_on = 1;
    output_value = 0x0000;
  }

  // Set each available DOUT
  if (fusion_instance->dout_count)
  {
    // set the dout values according the test pattern
    for(count=0; count < 10; count++)
      ddi_sdk_fusion_set_dout16(fusion_instance, count, output_value);
  }
}

// Tests from the original main.cpp under here
// These tests will not make fatal exit yet for testing purposes
TEST_F(AcontisTestFixture, DVT_DOUTLatency)
{
  DO_IO callBack(this);
  // Open Fusion Instance
  open_fusion(&callBack);
  ASSERT_EQ(fusion_instance==NULL, false) << "Returned Fusion interface is NULL, exiting\n";

  // Set free-run
  set_sync_mode(fusion_instance, SYNC_MODE_FREE_RUN);
  //set_sync_mode(fusion_instance, SYNC_MODE_SM_SYNCHRON);

  // Go to OP mode
  go_to_op_mode(PROJECT_VERSION);

  // Clear the screen
  clrscr();

  printf("Welcome to %s version: %s\n", PROJECT_NAME, PROJECT_VERSION);

  // Sleep for 60 seconds
  AcontisTestFixture::poll_EtherCAT(SEC_PER_MIN*60,ONE_SEC_DELAY_MS);
}
