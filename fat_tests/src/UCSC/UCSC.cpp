/**************************************************************************
(c) Copyright 2022 Digital Dynamics Inc. Scotts Valley CA USA.
Unpublished copyright. All rights reserved. Contains proprietary and
confidential trade secrets belonging to DDI. Disclosure or release without
prior written authorization of DDI is prohibited.
**************************************************************************/
#include "AcontisTestFixture.h"
#include "CRAMProcessData.h"
#include <fcntl.h>
#include <termios.h>
#include <math.h>

using namespace std;
// Log file
#define LOGFILE_NAME "UCSC.csv"
// Project Name and Version
#define PROJECT_NAME "UCSC"
#define PROJECT_VERSION "v1.0.0"
#define ONE_SEC_DELAY_MS 1000

#define UCSC_SOURCE_DOUT_START 0
#define UCSC_SINK_DOUT_START 1

class UCSC : public CallbackTest
{
  public:
    UCSC (AcontisTestFixture *p) : CallbackTest(p) {}
    virtual void cyclic_method (void*);
};

// Cyclic callback for IO demo
void UCSC::cyclic_method(void *arg)
{
  uint16_t source_dout_position, sink_dout_position;
  uint16_t current_state, requested_state, input_value;
  static uint16_t toggle_on = 0;
  static uint16_t count = 0;
  static uint16_t light_cycle = 0;
  
  static uint16_t dout_value = 1;
  int x, y;
  ddi_fusion_instance_t* local_fusion_instance;

  // Validate parameters
  if ( !arg )
    return;
  
  // Set the fusion interface from the argument to the cyclic data function
  local_fusion_instance = (ddi_fusion_instance_t*) arg;

  ddi_sdk_ecat_get_slave_state(local_fusion_instance->slave,&current_state,&requested_state);

  // Don't process data unless the slave is in OP mode
  if ( current_state != DDI_ECAT_OP )
    return;

  source_dout_position = UCSC_SOURCE_DOUT_START;  // 0
  sink_dout_position   = UCSC_SINK_DOUT_START; // 1

// ********* This block, when un-commented, will toggle one LED per second*****
// printf ("count %d\n", count);
  if( count == 100)
  {

    // ddi_sdk_fusion_set_dout16(local_fusion_instance, source_dout_position, dout_value);// Actuator
    light_cycle++;
    dout_value = light_cycle == 0 ? 0 : (1 << (light_cycle - 1));
    


    // Guess (fusion_instance, which_field_connect, output )
    ddi_sdk_fusion_set_dout16(local_fusion_instance, sink_dout_position, dout_value);// LED
    
    if(light_cycle == 17){ // On the seventeenth bit shift reset to zero
      light_cycle = 0;
    }

    toggle_on++;
    if(toggle_on == 6)
    {
      toggle_on = 0;
    }

  }

  if(count == 100){
    count = 0;
  }else{
    count++;
  }
//********* End of the individual LED toggle block*************


//********This is a simple LED toggler at a 5-second interval
  // Check for toggle status, then set values
  // if ( !toggle_on && count == 1000 )
  // {
  //   ddi_sdk_fusion_set_dout16(local_fusion_instance, source_dout_position, 0XFFFF);

  //   ddi_sdk_fusion_set_dout16(local_fusion_instance, sink_dout_position, 0xFFFF);

  //   toggle_on = 1;
  //   count = 0;
  // }
  // else if ( toggle_on && count == 1000 )
  // {
  //   ddi_sdk_fusion_set_dout16(local_fusion_instance, source_dout_position, 0X0000);

  //   ddi_sdk_fusion_set_dout16(local_fusion_instance, sink_dout_position, 0x0000);

  //   toggle_on = 0;   
  //   count = 0;   
  // }
  // count++;
//*************End of the 5-second interval toggler
}

TEST_F(AcontisTestFixture, UCSC_IO_TEST)
{
  // Callback mechanism
  UCSC callBack(this);
  
  // Open Fusion Instance and register callback
  AcontisTestFixture::open_fusion(&callBack);
  ASSERT_EQ(fusion_instance==NULL, false) << "Returned fusion interface is NULL, exiting.\n";

  ddi_sdk_ecat_set_license_file((char *)"./acontis_licenses.csv");

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

