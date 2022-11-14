#define DEBUG
#include "gtest/gtest.h"
#include "../include/AcontisTestFixture.h"

#include "ddi_sdk_common.h"
#include "ddi_sdk_ecat_master.h"
#include "ddi_sdk_ecat_sdo.h"
#include "ddi_status.h"
#include "ddi_sdk_fusion_interface.h"
#include "ddi_status.h"

#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <vector>
#include <array>

// For console output and gflag testing
#include <iostream>

using namespace std;

#define PROJECT_NAME "DI_Switching_Threshold_EtherCAT_test"
#define PROJECT_VERSION "v1.0.0"

#define ONE_SEC_DELAY_MS 1000
#define TOGGLE_CNT 1000 
#define HOLDOFF_CNT 4
#define TTY_FILE "/dev/ttyACM0"
#define SWEEP_START 12.0
#define SWEEP_END 13.0
#define SWEEP_CNT 3
#define WAIT_CNT 10
#define VOLTAGE_INC 0.01

int serial_port;
float voltage = SWEEP_START;
bool rising = true;
int sweep = 0;
int counter = 0;

std::vector<std::array<float, 16>> turnon_voltages;
std::vector<std::array<float, 16>> turnoff_voltages;

uint16_t last_value;

class DI_SWITCH_TEST : public CallbackTest
{
  public:
    DI_SWITCH_TEST (AcontisTestFixture *p): CallbackTest(p) {}
    virtual void cyclic_method (void*);
};

void DI_SWITCH_TEST::cyclic_method (void *arg)
{
  static int holdoff_cnt = HOLDOFF_CNT; // Holdoff compare for 5 frames after performing a toggle
  static int toggle_cnt = TOGGLE_CNT; // Hold the new pattern for this amount of trains after performing a toggle
  static uint16_t output_value = 0x7FFF;
  ddi_fusion_instance_t * fusion_instance;
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

  // if ( toggle_cnt-- == 0 )
  // {
  //   output_value = rand();
  //   //printf("generated 0x%x \n", output_value);
  //   holdoff_cnt = HOLDOFF_CNT;
  //   toggle_cnt = TOGGLE_CNT;
  // }

  if (counter++ > WAIT_CNT) {
    counter = 0;

    // increment voltage
    if (rising) {
      voltage += VOLTAGE_INC;
    } else {
      voltage -= VOLTAGE_INC;
    }

    // write voltage
    char buffer[50];
    int len = sprintf(buffer, "VSET1:%.2f\r", voltage);
    write(serial_port, buffer, len);

    // if ( holdoff_cnt ) // Dont perform any compares until this is -2
    // {
    //   holdoff_cnt--;
    //   return;
    // }
  
    uint16_t size, input_value;
    input_value = ddi_sdk_fusion_get_din(fusion_instance, 0, &size);

    if (input_value != last_value) {
      uint16_t mask = 1;
      uint16_t diff = input_value ^ last_value;

      // check which channels changed
      for (unsigned int i = 0; i < 16; i++) {
        if (diff & mask != 0) {
          printf("Channel %d changed\n", i);
          if (rising) {
            turnon_voltages[sweep][i] = voltage;
          } else {
            turnoff_voltages[sweep][i] = voltage;
          }
        }
      }

      printf("input_value -3x%x \n", input_value);
      last_value = input_value;
    }

    if (rising && ((input_value == 0xFFFF) || voltage >= SWEEP_END)) {
      rising = false;
      voltage = SWEEP_END;
      turnon_voltages.push_back(std::array<float, 16>());
    }
    else if (!rising && ((input_value == 0x0000) || voltage <= SWEEP_START)) {
      rising = true;
      voltage = SWEEP_START;
      turnoff_voltages.push_back(std::array<float, 16>());
      sweep++;
    }

    if (sweep > SWEEP_CNT) {


      exit(0);
    }
  }


  return;
}

// Tests from the original main.cpp under here
// These tests will not make fatal exit yet for testing purposes
TEST_F(AcontisTestFixture, DInSwitchingThreshold)
{
  DI_SWITCH_TEST callBack(this);
  printf("\n\n\n ***** InterlockBridge Test ***** \n\n\n");

  serial_port = open(TTY_FILE, O_RDWR);

  if (serial_port < 0) {
    printf("Error %i from open: %s\n", errno, strerror(errno));
  }

  struct termios tty;

  if(tcgetattr(serial_port, &tty) != 0) {
    printf("Error %i from tcgetattr: %s\n", errno, strerror(errno));
  }

  tty.c_cflag &= ~PARENB;
  tty.c_cflag &= ~CSTOPB;
  tty.c_cflag &= ~CSIZE;
  tty.c_cflag |= CS8;
  tty.c_cflag &= ~CRTSCTS;
  tty.c_cflag |= CREAD | CLOCAL;
  tty.c_cflag &= ~ICANON;
  
  tty.c_lflag &= ~ECHO;
  tty.c_lflag &= ~ISIG;

  tty.c_iflag &= ~(IXON | IXOFF | IXANY);
  tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL);

  tty.c_oflag &= ~OPOST;
  tty.c_oflag &= ~ONLCR;

  tty.c_cc[VTIME] = 10;
  tty.c_cc[VMIN] = 0;

  cfsetispeed(&tty, B9600);

  if (tcsetattr(serial_port, TCSANOW, &tty) != 0) {
    printf("Error %i from tcsetattr: %s\n", errno, strerror(errno));
  }

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

