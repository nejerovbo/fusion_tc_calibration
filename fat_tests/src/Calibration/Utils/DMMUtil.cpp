#include "AcontisTestFixture.h"
#include "CRAMProcessData.h"
#include "sys/socket.h"
#include "arpa/inet.h"
#include "stdio.h"
#include "stdbool.h"
#include "math.h"
#include "CalibrationCommon.h"
#include "DAQUtil.h"
#include <termios.h>
#include <fcntl.h>
#include "telnet_open.h"
#include "DMMUtil.h"

using namespace std;

int connected_to_meter = 0;

int open_connection_to_meter(void) {
  //open a connection to the keysight meter...
  // this needs a lot more error checking, and friendlier error messages.
  //printf("DEBUG Opening connection to meter.\n");
  if (telnet_open("192.168.9.29", 5024) < 0) {
    //error connecting to keysight meter.
    printf("***** Something with the DMM is wrong\n");
    printf(" Unable to connect to Keysight meter\n");
    exit(1);
  }
  //printf("DEBUG - connection opened\n");
  telnet_verbose(0);
  get_input(0); //get (and don't show) input/banner message.
  connected_to_meter = 1;
  return connected_to_meter;
}

//Return meter to local mode
void set_meter_to_local_mode(void) {
  char buffer[160];

  telnet_send_str("SYST:LOC\r\n");
  telnet_gets(buffer, sizeof(buffer));
}

int close_connection_to_meter(void) {

  if (connected_to_meter == 0) {
    return 0;
  }
  set_meter_to_local_mode();
  telnet_close();
  connected_to_meter = 0;
  return connected_to_meter;
}

double read_meter_volts(void) {
  double volts;
  char buffer[160];

  //printf("Sending read command\n");
  telnet_send_str("MEASURE:VOLTAGE:DC?\r\n");
  wait_for_echo("MEASURE:VOLTAGE:DC?\r\n");
  // now read the voltage
  telnet_gets(buffer, sizeof(buffer));
  //printf("Debug '%s'\n", buffer);
  volts = atof(buffer);
  //printf("DEBUG volts read from meter =            %0.3f\n", volts);
  return volts;
}

double read_meter_ohms(void) {
  double ohms;
  char buffer[160];

  //printf("Sending read command\n");
  //telnet_send_str("MEASURE:FRESISTANCE:DC?\r\n");  //four-wire resistance measurement...
  telnet_send_str("MEASURE:RESISTANCE?\r\n");
  wait_for_echo("MEASURE:RESISTANCE?\r\n");
  // now read the resistance
  telnet_gets(buffer, sizeof(buffer));
  //printf("Debug '%s'\n", buffer);

  ohms = atof(buffer);
  printf("DEBUG ohms read from meter =            %0.3f\n", ohms);
  //
  read_meter_volts();  //Switch meter back to voltage, because ohms test sources current.
  return ohms;
}
