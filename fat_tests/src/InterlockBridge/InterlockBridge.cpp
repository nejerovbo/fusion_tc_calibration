#define DEBUG
#include "gtest/gtest.h"
#include "../include/AcontisTestFixture.h"

#include "ddi_sdk_common.h"
#include "ddi_sdk_ecat_master.h"
#include "ddi_sdk_ecat_sdo.h"
#include "ddi_status.h"
#include "ddi_sdk_fusion_interface.h"
#include "ddi_status.h"

// For console output and gflag testing
#include <iostream>
#include <gflags/gflags.h>
#include "InterlockBridge.h"

using namespace std;

#define PROJECT_NAME "InterlockBridge_EtherCAT_test"
#define PROJECT_VERSION "v1.0.0"

#define ONE_SEC_DELAY_MS 1000
#define TOGGLE_CNT 1000
#define HOLDOFF_CNT 4

class InterlockBridgeIO : public CallbackTest
{
  public:
    InterlockBridgeIO (AcontisTestFixture *p): CallbackTest(p) {}
    virtual void cyclic_method (void*);
};

void InterlockBridgeIO::cyclic_method (void *arg)
{
  static int holdoff_cnt = HOLDOFF_CNT; // Holdoff compare for 5 frames after performing a toggle
  static int toggle_cnt = TOGGLE_CNT; // Hold the new pattern for this amount of trains after performing a toggle
  static uint16_t output_value = 0x7FFF;
  ddi_fusion_instance_t * fusion_instance;
  pd_out_t pd_out;
  pd_out.ib_dout = 0xFFFF;
  pd_out.rim1_dout = 0xFFFF;
  uint16_t current_state, requested_state;
    
  //check parameters
  if (!arg)
    return;

  //set the fusion interface from the argument to the cyclic data function
  fusion_instance = (ddi_fusion_instance_t *)arg;

  ddi_sdk_ecat_get_slave_state(fusion_instance->slave,&current_state,&requested_state);
  //don't process data unless the slave is in OP mode
  if ( current_state != DDI_ECAT_OP)
    return;

  if ( toggle_cnt-- == 0 )
  {
    output_value = rand();
    //printf("generated 0x%x \n", output_value);
    holdoff_cnt = HOLDOFF_CNT;
    toggle_cnt = TOGGLE_CNT;
  }

  ddi_sdk_fusion_set_dout16(fusion_instance, 0, output_value);
  ddi_sdk_fusion_set_dout16(fusion_instance, 1, output_value);

  if ( holdoff_cnt ) // Dont perform any compares until this is 0
  {
    holdoff_cnt--;
    return;
  }
  
  uint16_t size, input_value;
  input_value = ddi_sdk_fusion_get_din(fusion_instance, 2, &size);
  if ( input_value != output_value)
  {
    ELOG("Right side IB Error \n");
    m_AcontisTestFixture->m_test_result = ddi_status_err;
  }
  input_value = ddi_sdk_fusion_get_din(fusion_instance, 3, &size);
  if ( input_value != output_value)
  {
    ELOG("Right side IB Error \n");
    m_AcontisTestFixture->m_test_result = ddi_status_err;
  }

  return;
}


// Used for testing mutiple callbacks operating in mutiple test fixtures
void update_IO_skeleton(void *arg)
{
  static int count = ONE_SEC_DELAY_MS;
  count--;
  if ( count != 0 )
    return;
  count = ONE_SEC_DELAY_MS;
}

// this function gets called every process data cycle
// this function will set outputs and display inputs
void update_IO_InterlockBridge(void *arg)
{
  // Run the hello world update by default
  AcontisTestFixture::hello_world_demo(arg);
}

// Test using a different callback
TEST_F(AcontisTestFixture, SkeletonTest)
{
  // Open Fusion Instance
  AcontisTestFixture::open_fusion((cyclic_func_t *)update_IO_InterlockBridge);
  AcontisTestFixture::go_to_op_mode(PROJECT_VERSION);
  AcontisTestFixture::poll_EtherCAT(10,ONE_SEC_DELAY_MS);
}

// Tests from the original main.cpp under here
// These tests will not make fatal exit yet for testing purposes
TEST_F(AcontisTestFixture, InterlockBridge)
{
  InterlockBridgeIO callBack(this);
  printf("\n\n\n ***** InterlockBridge Test ***** \n\n\n");
  // Open Fusion Instance
  AcontisTestFixture::open_fusion(&callBack);
  ASSERT_EQ(fusion_instance==NULL, false) << "Returned Fusion interface is NULL, exiting\n";

  // Go to OP mode
  AcontisTestFixture::go_to_op_mode(PROJECT_VERSION);

  // Sleep for 60 seconds
  AcontisTestFixture::poll_EtherCAT(SEC_PER_MIN*10,ONE_SEC_DELAY_MS);
  //AcontisTestFixture::poll_EtherCAT(10,ONE_SEC_DELAY_MS);
  ASSERT_EQ(m_test_result, ddi_status_ok);
}

