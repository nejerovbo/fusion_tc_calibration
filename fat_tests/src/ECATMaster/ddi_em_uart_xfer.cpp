/**************************************************************************
(c) Copyright 2022 Digital Dynamics Inc. Scotts Valley CA USA.
Unpublished copyright. All rights reserved. Contains proprietary and
confidential trade secrets belonging to DDI. Disclosure or release without
prior written authorization of DDI is prohibited.
**************************************************************************/
#include <fcntl.h>
#include <termios.h>
#include <inttypes.h>
#include <stdio.h>
#include <iostream>
#include <inttypes.h>
#include <thread>
#include <sys/time.h>
#include "EnvironmentRegistry.h"
#include "DDIEMUtility.h"
#include "DDIEMEnvironment.h"
#include "ddi_em_fusion_uart_api.h"
#include "ddi_em_fusion.h"
#include "ddi_fusion_uart_test_fixture.h"


//-----------------------------------------------------------------------------
// This test transfers data to/from the host computer on a USB-serial dongle
//  to the fusion UART on RIM3.
// For RS232 mode
//  -Change UART_INTERFACE_MODE to UART_INTERFACE_RS232
//  -Recompile
//  -Use a logear model UC-232A and null-modem cable
//  -Mode switches on the RIM3 are to the right, with the RIM sitting upright
// For RS485 mode
//  -Change UART_INTERFACE_MODE to UART_INTERFACE_RS485
//  -Recompile
//  -Use a Dtech USB 2.0 to RS422/RS485 cable with a special connector Bob built
//  -Mode switches on the RIM3 are to the left, with the RIM sitting upright
//
//  Run the test using run_xfer.sh script
//-----------------------------------------------------------------------------


#define UART_START_INDEX    0x5095  // 5095 for RIM 3
#define UART_CHANNEL        0       // Of 0, 1, 2, or 3
#define THIS_NIC            DDI_EM_NIC_1
#define CONFIG_FILE         "config/platform_eni.xml"
#define TTY_FILE            "/dev/ttyUSB0"
//#define UART_INTERFACE_MODE UART_INTERFACE_RS232
#define UART_INTERFACE_MODE UART_INTERFACE_RS485




int init_serial_port(void);
void init_uart(ddi_fusion_uart_handle uart_handle);
void do_loopback(ddi_fusion_uart_handle uart_handle);

//TEST(UART_XFER_SUITE, DDIEM_UART_Xfer_test)
TEST(UART_XFER_SUITE, DDIEM_UART_Xfer)
{
  ddi_em_handle em_handle;
  ddi_es_handle es_handle;
  ddi_em_slave_config es_cfg;    
  ddi_fusion_uart_handle uart_handle;
  ddi_em_result result;
  ddi_em_init_params init_params;
  uint32_t flags = 0;  

  uint8_t  tx1_data[DDI_FUSION_UART_SDO_DATA_SIZE_MAX];
  uint8_t  tx2_data[DDI_FUSION_UART_SDO_DATA_SIZE_MAX];
  uint8_t  rx1_data[DDI_FUSION_UART_SDO_DATA_SIZE_MAX];
  uint8_t  rx2_data[DDI_FUSION_UART_SDO_DATA_SIZE_MAX];
  uint16_t rx1_len, rx2_len;
  int fd_serial;

  memset(&init_params, 0, sizeof(ddi_em_init_params));

  // EtherCAT network adapter to use
  init_params.network_adapter         = THIS_NIC;
  // Enable direct call-in from an external thread
  init_params.enable_cyclic_thread    = DDI_EM_TRUE;
  init_params.scan_rate_us            = DDI_EM_DEFAULT_CYCLIC_RATE;
  // Use the default thread priority for the EtherCAT Cyclic Thread
  init_params.polling_thread_priority = DDI_EM_CYCLIC_THREAD_PRI_DEFAULT;

  result = ddi_em_sdk_init();
  ASSERT_EQ(DDI_EM_STATUS_OK, result) << "ddi_em_sdk_init failed: " << ddi_em_get_error_string(result) << "\n";

  //initialize master instance
  result = ddi_em_init(&init_params, &em_handle);
  ASSERT_EQ(DDI_EM_STATUS_OK, result) << "ddi_em_init failed \n";

  // configure master
  result = ddi_em_configure_master(em_handle, CONFIG_FILE);
  ASSERT_EQ(DDI_EM_STATUS_OK, result);

  // open the fusion with vendor and product ID
  result = ddi_em_open_slave_by_id(em_handle, DDI_ETHERCAT_VENDOR_ID, DDI_FUSION_PRODUCT_CODE, 0, 0, &es_handle);
  ASSERT_EQ(DDI_EM_STATUS_OK, result);

  // retrieve the information about the slave
  result = ddi_em_get_slave_config(em_handle, es_handle, &es_cfg);
  ASSERT_EQ(DDI_EM_STATUS_OK, result);

  result = ddi_fusion_uart_open(em_handle, es_handle, UART_START_INDEX, (uart_channel) UART_CHANNEL, flags, &uart_handle);
  ASSERT_EQ(DDI_EM_STATUS_OK, result);

  result = ddi_em_set_master_state(em_handle, DDI_EM_STATE_PREOP, TEST_DEFAULT_TIMEOUT);
  ASSERT_EQ(DDI_EM_STATUS_OK, result) << "ddi_em_set_master_state failed.\n";

  fd_serial = init_serial_port();
  ASSERT_GT(fd_serial, 0) << "failed to open serial port\n";

  init_uart(uart_handle);

  srand(time(NULL)); // Seed the RNG
  sleep(1);

  // Clear the read buffer, this was necessary because we were getting a leading 0 on the first read
  rx2_len = read(fd_serial, rx2_data, sizeof(rx2_data));

  //-----------------------------------------------------------------------------
  // TransferLoop
  //  -Create random data in the tx buffers
  //  -send data host to UART
  //  -send data UART to host
  //  -Clear receive buffers
  //  -Receive and compare data from the UART
  //  -Receive and compare data from the host
  //-----------------------------------------------------------------------------
  for( int loop = 0; loop < 256; loop++)
  {
    printf("loop = %3d\r", loop);  fflush(stdout);  // Print the loop count on the same line

    // Create random data in the 2 tx buffers
    for (int count = 0; count < sizeof(tx1_data); count++)
    {
      tx1_data[count] = rand();
      tx2_data[count] = rand();
    }

    // Transmist data from the host, USB RS232/RS485 dongle,  to the fusion
    // Delay, don't remove the delay or we have bus contention in RS485 mode
    // Transmit data from the fusion to the host
    rx1_len = write(fd_serial, (char *)tx1_data, sizeof(tx1_data));
    usleep(20000);
    result = ddi_fusion_uart_tx_data(uart_handle, tx2_data, sizeof(tx2_data));
    EXPECT_EQ(DDI_EM_STATUS_OK, result) << "ddi_fusion_uart_tx_data failed\n";

    // Clear receive buffers
    memset(rx1_data, 0, sizeof(rx1_data));
    memset(rx2_data, 0x00, sizeof(rx2_data));

    usleep(60000);  // Wait for the transfer to complete

    //-------------------------------------
    // Receive and compare the data sent to the UART
    //-------------------------------------
    rx1_len = 0;
    while ( rx1_len < sizeof(tx1_data))
    {
      result = ddi_fusion_uart_get_rx_bytes_avail(uart_handle, &rx1_len);
      ASSERT_EQ(DDI_EM_STATUS_OK, result) << "ddi_fusion_uart_get_rx_bytes_avail failed\n";
    }

    uint32_t len;
    result = ddi_fusion_uart_rx_data(uart_handle, rx1_data, &len);
    ASSERT_EQ(DDI_EM_STATUS_OK, result) << "ddi_fusion_uart_rx_data failed \n";
    ASSERT_EQ( memcmp(rx1_data, tx1_data, sizeof(tx1_data)), 0 );

    //-------------------------------------
    // Receive and compare the data sent to the host 
    //-------------------------------------
    rx2_len = read(fd_serial, rx2_data, sizeof(rx2_data));
    ASSERT_EQ( memcmp(rx2_data, tx2_data, sizeof(tx2_data)), 0 );
  }

  // close fusion instance
  result = ddi_fusion_uart_close(uart_handle);
  EXPECT_EQ(DDI_EM_STATUS_OK, result);

  // de-init the master instance
  result = ddi_em_deinit(em_handle);
  EXPECT_EQ(DDI_EM_STATUS_OK, result) << "ddi_em_deinit failed \n";

  //de-init the SDK
  result = ddi_em_sdk_deinit();
  EXPECT_EQ(DDI_EM_STATUS_OK, result) << "ddi_em_sdk_deinit failed: " << ddi_em_get_error_string(result) << "\n";
  printf("\n");
}


//-----------------------------------------------------------------------------
// Init the UART to RS232 mode, 115,200 baud, 8-N-1
//-----------------------------------------------------------------------------
void init_uart(ddi_fusion_uart_handle uart_handle)
{
  ddi_em_result result;

  result = ddi_fusion_uart_set_interface(uart_handle, UART_INTERFACE_MODE);
  ASSERT_EQ(DDI_EM_STATUS_OK, result);

  // Initialize UART to 115200 baud, 8-N-1
  result = ddi_fusion_uart_set_baud(uart_handle, UART_BAUD_115200);
  ASSERT_EQ(DDI_EM_STATUS_OK, result);

  result = ddi_fusion_uart_set_data_bits(uart_handle, UART_DATA_BITS_8);
  ASSERT_EQ(DDI_EM_STATUS_OK, result);

  result = ddi_fusion_uart_set_parity_mode(uart_handle, UART_PARITY_NONE);
  ASSERT_EQ(DDI_EM_STATUS_OK, result);

  result = ddi_fusion_uart_set_stop_bits(uart_handle, UART_STOP_BITS_1);
  ASSERT_EQ(DDI_EM_STATUS_OK, result);

  result = ddi_fusion_uart_channel_flush (uart_handle);
  ASSERT_EQ(DDI_EM_STATUS_OK, result);
}


//-----------------------------------------------------------------------------
// Init the USB serial port 115,200 baud, 8-N-1
// Return the serial port file descriptor
//-----------------------------------------------------------------------------
int init_serial_port()
{
  int fd;
  struct termios tty;

  if( (fd = open(TTY_FILE, O_RDWR)) < 0)
  {
    printf("Error %i from open: %s\n", errno, strerror(errno));
  }

  // Get existing termios attributes
  if(tcgetattr(fd, &tty) != 0) {
    printf("Error %i from tcgetattr: %s\n", errno, strerror(errno));
  }

  cfsetispeed(&tty, B115200);

  // 8-N-1, No RTS/CTS, Enable Receiver, Ignore modem control lines 
  tty.c_cflag &= ~(PARENB | CSTOPB | CSIZE | CRTSCTS);
  tty.c_cflag |= (CREAD | CLOCAL | CS8 );

  // No echo, no signals, not canonical mode
  tty.c_lflag &= ~(ECHO | ISIG | ICANON);

  tty.c_iflag &= ~(IXON | IXOFF | IXANY | IGNCR);
  tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | ICRNL);

  tty.c_oflag &= ~(OPOST |ONLCR);

  tty.c_cc[VTIME] = 10;
  tty.c_cc[VMIN] = 0;

  // Set new termios attributes
  if (tcsetattr(fd, TCSANOW, &tty) != 0) {
    printf("Error %i from tcsetattr: %s\n", errno, strerror(errno));
  }
  return fd;
}
