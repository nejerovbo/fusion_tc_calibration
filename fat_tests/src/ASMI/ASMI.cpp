// For console output and gflag testing
#include "AcontisTestFixture.h"
#include "CRAMProcessData.h"
#include <fcntl.h>
#include <termios.h>
#include "ASMI.h"

using namespace std;
// Log file
#define LOGFILE_NAME "ASMI.csv"
// Project Name and Version
#define PROJECT_NAME "ASMI_test"
#define PROJECT_VERSION "v1.0.0"
#define ONE_SEC_DELAY_MS 1000

// Supports G-test callback from the Acontis SDK
class ASMI : public CallbackTest
{
  public:
    ASMI (AcontisTestFixture *p): CallbackTest(p) {}
    virtual void cyclic_method (void*);
};

// Cyclic callback for the ASMI test
void ASMI::cyclic_method (void *arg)
{
  uint16_t ain_position ;
  uint16_t din_position, relay_position, dout_position;
  uint16_t current_state, requested_state, input_value;
  uint16_t din_value, din_value1,din_value2;
  static uint16_t dout_value=0x1000;
  static uint16_t dout_value1=0x2000;
  static uint16_t dout_value2=0x3000;
  uint16_t din_value_J3;
  uint16_t din_value_J7;
  uint16_t din_value_J8;
  uint16_t din_value_J9;
  uint16_t din_value_J10;
  uint16_t din_value_J11;
  uint16_t din_value_J12;
  uint16_t din_value_J14;
  uint16_t din_value_J15;
  uint16_t din_value_J16;
  uint16_t din_value_J17;
  uint16_t din_value_J18;
  uint16_t din_value_J19;
  uint16_t din_value_J20;
  uint16_t din_value_J21;
  uint16_t din_value_J22;
  uint16_t din_value_J23;
  
  static uint8_t relay_value=0,relay_value4;
  uint8_t dinlsb,dinmsb,channel;
  uint16_t avalue;
  int16_t ain_value_c1;
  int16_t ain_value_c2;
  int16_t ain_value_c3;
  int16_t ain_value_c4;
  int16_t ain_value_c5;
  int16_t ain_value_c6;
  int16_t ain_value_c7;
  int16_t ain_value_c8;
  uint16_t i=0;
  uint16_t size;
  uint8_t size8;
  static int16_t count=0;
int x,y;
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

  // Read from the first channel of the ASMI
  din_position = ASMI_RIM_5_DIN_START;
  // Read from the first channel of the ASMI
  dout_position = ASMI_RIM_5_RELAY_START;
   
  // Set the first relay channel of the ASMI
  relay_position = ASMI_RIM_5_RELAY_START;
  // Read from the first channel of the ASMI
  ain_position = ASMI_RIM_5_AIN_START;

  // Read the DIN 
  //
  
  din_value_J3=ddi_sdk_fusion_get_din(local_fusion_instance, din_position,&size);       //force guided relay
  din_value_J8=ddi_sdk_fusion_get_din(local_fusion_instance, din_position+2,&size);     //Dout
  din_value_J10=ddi_sdk_fusion_get_din(local_fusion_instance, din_position+4,&size);    //Dout
  din_value_J14=ddi_sdk_fusion_get_din(local_fusion_instance, din_position+7,&size);    //Relay
  din_value_J15=ddi_sdk_fusion_get_din(local_fusion_instance, din_position+8,&size);    //Relay
  din_value_J16=ddi_sdk_fusion_get_din(local_fusion_instance, din_position+9,&size);    //Relay
  din_value_J17=ddi_sdk_fusion_get_din(local_fusion_instance, din_position+10,&size);    //Relay
  din_value_J18=ddi_sdk_fusion_get_din(local_fusion_instance, din_position+11,&size);    //Dout


     x= 5;
     y= 6;

            printf ("\033[%d;%dH   R      DO     DO     R      R      R      R      DO   \n",x,y);
     x=6;
            printf ("\033[%d;%dH   j3     j8     j10    j14    j15    j16    j17    j18  \n",x,y);
//	                          xxxx   xxxx   xxxx   xxxx   xxxx   xxxx   xxxx   xxxx
     x= 7;
     y= 6;
            printf ("\033[%d;%dH  %04X   %04X   %04X   %04X   %04X   %04X   %04X   %04X \n",x,y,din_value_J3,din_value_J8,din_value_J10,din_value_J14,din_value_J15,din_value_J16,din_value_J17,din_value_J18);


  din_value_J7=ddi_sdk_fusion_get_din(local_fusion_instance,  din_position + 1,&size);
  din_value_J9=ddi_sdk_fusion_get_din(local_fusion_instance,  din_position + 3,&size);
  din_value_J11=ddi_sdk_fusion_get_din(local_fusion_instance, din_position + 5,&size);
  din_value_J12=ddi_sdk_fusion_get_din(local_fusion_instance, din_position + 6,&size);
  din_value_J19=ddi_sdk_fusion_get_din(local_fusion_instance, din_position +12,&size);
  din_value_J20=ddi_sdk_fusion_get_din(local_fusion_instance, din_position +13,&size);
  din_value_J21=ddi_sdk_fusion_get_din(local_fusion_instance, din_position +14,&size);
  din_value_J22=ddi_sdk_fusion_get_din(local_fusion_instance, din_position +15,&size);
  din_value_J23=ddi_sdk_fusion_get_din(local_fusion_instance, din_position+16,&size);
  
  
  // Write the din data to the log file
  m_AcontisTestFixture->log_write(true, "0x%04x, 0x%04x \n", din_value, din_value);
  
     x= 10;
     y= 6;

            printf ("\033[%d;%dH   DI     DI     DI     DI     DI     DI     DI     DI   \n",x,y);
	    x=11;
            printf ("\033[%d;%dH   j7     j9     j11    j12    j19    j20    j21    j22    j23  \n",x,y);
//	                          xxxx   xxxx   xxxx   xxxx   xxxx   xxxx   xxxx   xxxx   xxxx
     x= 12;
     y= 6;
            printf ("\033[%d;%dH  %04X   %04X   %04X   %04X   %04X   %04X   %04X   %04X   %04X\n",x,y,din_value_J7,din_value_J9,din_value_J11,din_value_J12,din_value_J19,din_value_J20,din_value_J21,din_value_J22,din_value_J23);


 dout_value++; 
 if(dout_value == 0xFFF0)
	 dout_value = 0;
     // J8 -  16 channel sinking
  ddi_sdk_fusion_set_dout16(local_fusion_instance, dout_position+1, dout_value);

 dout_value1++; 
 if(dout_value1 == 0xFFF0)
	 dout_value1 = 0;
     // J10-  16 channel sinking
  ddi_sdk_fusion_set_dout16(local_fusion_instance, dout_position+2, dout_value1);

// Relays - controlled by Douts
  // Set the relay value determined by the 8 lsbs of the DIN
x=18;
y=6;
            printf ("\033[%d;%dH count %3d relay value %04X dout_value %04X dout_value1 %04X dout_value2 %04X\n",x,y,count ,relay_value,dout_value, dout_value1,dout_value2);
  count++;
  if(count==100)
  {
      relay_value = relay_value ^ 0xff;
      relay_value4 = relay_value4 ^ 0xf;
      count = 0;
  }
  //  Relays
     // J3 - Force Guided Relay
  ddi_sdk_fusion_set_dout8(local_fusion_instance, relay_position+0, relay_value4);

     // J14-  8 channel relay
  ddi_sdk_fusion_set_dout8(local_fusion_instance, relay_position + 3, relay_value);
  
     // J15-  8 channel relay
  ddi_sdk_fusion_set_dout8(local_fusion_instance, relay_position + 4, relay_value);
  
     // J16-  8 channel relay
  ddi_sdk_fusion_set_dout8(local_fusion_instance, relay_position + 5, relay_value);
  
     // J17-  8 channel relay
  ddi_sdk_fusion_set_dout8(local_fusion_instance, relay_position + 6, relay_value);
  
     // J18-  16 channel Dout 
 dout_value2++; 
 if(dout_value2== 0xFFF0)
	 dout_value2 = 0;
  ddi_sdk_fusion_set_dout16(local_fusion_instance, dout_position + 7, dout_value2);
  
  
  // Get the AIN value  j24
  ain_value_c1 = ddi_sdk_fusion_get_ain(local_fusion_instance, ain_position);
  ain_value_c2 = ddi_sdk_fusion_get_ain(local_fusion_instance, ain_position+1);
  ain_value_c3 = ddi_sdk_fusion_get_ain(local_fusion_instance, ain_position+2);
  ain_value_c4 = ddi_sdk_fusion_get_ain(local_fusion_instance, ain_position+3);
  ain_value_c5 = ddi_sdk_fusion_get_ain(local_fusion_instance, ain_position+4);
  ain_value_c6 = ddi_sdk_fusion_get_ain(local_fusion_instance, ain_position+5);
  ain_value_c7 = ddi_sdk_fusion_get_ain(local_fusion_instance, ain_position+6);
  ain_value_c8 = ddi_sdk_fusion_get_ain(local_fusion_instance, ain_position+7);

  x=14;
  y=6;
  printf ("\033[%d;%dH J24 ain 1-8  %04X %08X %08X %08X %08X %08X %08X %08X \n",x,y,ain_value_c1,ain_value_c2,ain_value_c3,ain_value_c4,ain_value_c5,ain_value_c6,ain_value_c7,ain_value_c8);
  // Write the ain data to the log file
  //
  float A_Value,C_Value;
/*
  A_Value = ((float)ain_value_c1/65536. * 20.) ;
     if ((din_value&0x0001) == 0x0001)	  
         C_Value = ((float)din_value/65536. * 20.) -10.;
     else
         C_Value = ((float)count/65536. * 20.)-10.;
  x=16;
  y=16;
     if ((din_value&0x0001) == 0x0001)	  
         printf ("\033[%d;%dH dinvalue voltage  %7.3f Volts",x,y,C_Value);
     else
         printf ("\033[%d;%dH Count    voltage  %7.3f Volts",x,y,C_Value);
  x=17;
  printf ("\033[%d;%dH J24 ain 1-8 voltage  %7.3f Volts",x,y,A_Value);
 */ 

  m_AcontisTestFixture->log_write(true, "0x%04x, 0x%04x \n", ain_value_c1, ain_value_c2);
}


// Tests from the original main.cpp under here
// These tests will not make fatal exit yet for testing purposes
TEST_F(AcontisTestFixture, ASMI_Test)
{
  // Callback mechanism
  ASMI callBack(this);

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
