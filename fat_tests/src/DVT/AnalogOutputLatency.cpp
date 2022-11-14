// For console output and gflag testing
#include "AcontisTestFixture.h"
#include "CRAMProcessData.h"

using namespace std;

// Provides a test for measuring analog output latency

#define PROJECT_NAME "Analog Output Latency EtherCAT_test"
#define PROJECT_VERSION "v1.0.0"
#define ONE_SEC_DELAY_MS 1000

class AO_IO : public CallbackTest
{
  public:
    AO_IO (AcontisTestFixture *p): CallbackTest(p) {}
    virtual void cyclic_method (void*);
};

void AO_IO::cyclic_method (void *arg)
{
  static int pattern_count = 0, output_value = 0, count;
  uint16_t current_state, requested_state, input_value;
  static int toggle_on = 0;
  ddi_fusion_instance_t* fusion_instance;
  // validate parameters
  if (!arg)
    return;

  // get the fusion interface from the argument to the cyclic data function
  fusion_instance = (ddi_fusion_instance_t *)arg;

  ddi_sdk_ecat_get_slave_state(fusion_instance->slave,&current_state,&requested_state);
  // don't process data unless the slave is in OP mode
  if ( current_state != DDI_ECAT_OP)
    return;

  if ( toggle_on == 1)
  {
    toggle_on = 0;
    output_value = 0x7FFF;
  }
  else
  {
    toggle_on = 1;
    output_value = 0x8000;
  }
 
  // If there's aouts present then set them all
  if (fusion_instance->aout_count)
  {
    // set the aout values according the test pattern
    for(count=0; count < fusion_instance->aout_count; count++)
      // fusion_instance->aouts[count] = output_value;
      ddi_sdk_fusion_set_aout(fusion_instance, count, output_value);

    uint16_t input_value = ddi_sdk_fusion_get_ain(fusion_instance, 0);
    printf("input_value 0x%04x\n", input_value);
  }
}

// Tests from the original main.cpp under here
// These tests will not make fatal exit yet for testing purposes
TEST_F(AcontisTestFixture, DVT_AOUTLatency)
{
  AO_IO callBack(this);
  // Open Fusion Instance
  open_fusion(&callBack);
  ASSERT_EQ(fusion_instance==NULL, false) << "Returned Fusion interface is NULL, exiting\n";

  set_sync_mode(fusion_instance, SYNC_MODE_FREE_RUN);
  //set_sync_mode(fusion_instance, SYNC_MODE_SM_SYNCHRON);

  // Go to OP mode
  go_to_op_mode(PROJECT_VERSION);

  // Sleep for 60 seconds
  AcontisTestFixture::poll_EtherCAT(SEC_PER_MIN*60,ONE_SEC_DELAY_MS);
}
