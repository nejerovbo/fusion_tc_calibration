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
#include "DCUtil.h"


// Following block is used for DC manipulation
#define TTY_FILE "/dev/ttyUSB0"
int    ai_serial_port = -1;
char          cmd_buff[50];
int           len;  
// End of block

int DC_init()
{
  ai_serial_port = open(TTY_FILE, O_RDWR);
  if(ai_serial_port < 0)
  {
    return -1;
  }
  struct termios tty;
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
  return 1;
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void set_DC_voltage(double voltage)
{
  len = sprintf(cmd_buff, "VOLT %.5f;\r", voltage);
  len = write(ai_serial_port, cmd_buff, len);
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void set_DC_range(vr_e voltage_range)
{
  disable_DC_out();

  // Set the range based on the input
  if(ONE_VOLT == voltage_range)
  {
    len = sprintf(cmd_buff, "RNGE0\r");
    len = write(ai_serial_port, cmd_buff, len);
  }
  else if( TEN_VOLTS == voltage_range)
  {
    len = sprintf(cmd_buff, "RNGE1\r");
    len = write(ai_serial_port, cmd_buff, len);
  }
  else if( ONE_HUNDRED_VOLTS == voltage_range)
  {
    len = sprintf(cmd_buff, "RNGE2\r");
    len = write(ai_serial_port, cmd_buff, len);
  }

  enable_DC_out();
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void enable_DC_out()
{
  // Turn the output on
  len = sprintf(cmd_buff, "SOUT1\r");
  len = write(ai_serial_port, cmd_buff, len);
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void disable_DC_out()
{
  // Turn off output before setting the range
  len = sprintf(cmd_buff, "SOUT0\r");
  len = write(ai_serial_port, cmd_buff, len);
}
