/******************************************************************************
 * (c) Copyright 2020-2021 Digital Dynamics Inc. Scotts Valley CA USA.
 * Unpublished copyright. All rights reserved. Contains proprietary and
 * confidential trade secrets belonging to DDI. Disclosure or release without
 * prior written authorization of DDI is prohibited.
 *****************************************************************************/
#ifndef DDI_SDK_COMMON_H
#define DDI_SDK_COMMON_H
#include "ddi_status.h"
#include "AtEthercat.h"

#ifdef __cplusplus
extern "C" {
#endif

#define UART_FIFO_DEPTH         22
//fifo control structure
typedef struct
{
  uint16_t control_status;
  uint8_t fifo[UART_FIFO_DEPTH];
} uart_t;

typedef struct {
  char network_interface[256];
  char eni_file[256];
  uint32_t scan_rate_us;
  uint32_t display_update_rate;
  uint8_t optimized_set;           //is the optimized link layer set in the cmd line?
  uint8_t optimized_instance;      //which optmized link layer to open
  char *io_logfile_name;           //rawio logfile name
  uint32_t io_display_update_rate; //rawio display rate
  char *coe_od_logfile;            //logfile for the COE object dictionary
} cmdline_args_t;

/** ddi_control_c_handler
 * Handle a control c event
 *
 * @param signal The signal received
 * @return void
 */
void ddi_sdk_control_c_handler(int signal);

/** ddi_sdk_log_data
 * Log data to a statistics file.  Currently this is assumed to be "stats.txt"
 *
 * @message The message to be logged in variable argument format
 * @return ddi_status_t
 * @see ddi_status_t
 */
ddi_status_t ddi_sdk_log_data(const char *message, ...);

/** ddi_log_error
 * Log data to a error file.  Currently this is assumed to be "iolog.txt"
 *
 * @param message The message to be logged in variable argument format
 * @return ddi_status_t
 * @see ddi_status_t
 */
ddi_status_t ddi_sdk_log_error(const char *message, ...);

/** ddi_sdk_init
 * Provides an entry point for DDI applications.  Currently the EtherCAT
 * master consumes the main argument.
 *
 * @param scan_rate_us the bus cycle rate in microsecond
 * @param eni_filename the ENI filename
 * @param network_interface the network interface to attach to
 * @return void
 */
ddi_status_t ddi_sdk_init(uint32_t scan_rate_us, char *eni_filename, char *network_interface);

/** ddi_sdk_exit
 * Provides an exit point for DDI applications.  This is called when
 * the EtherCAT master stops sending cyclic data
 *
 * @return void
 */
void ddi_sdk_exit(void);

/** calculate_pd_byte
 * Calculate the byte offset into the process data
 *
 * @message The process data entry structure
 * @return ddi_status_t
 */
uint16_t calculate_pd_byte (EC_T_PROCESS_VAR_INFO_EX *entry);

/** ddi_get_next_test_value
 * This function generates the next test value to send out. It
 * currently uses a fixed pattern that can be expanded as
 * more tests cases are added.
 *
 * @return The next test value
 */
int ddi_sdk_get_next_test_value ();

/** ddi_sdk_handle_cmdline
 * Handles command line-processing for standard ddi_acontis_sdk arguments.
 * Here are the current arguments supported:
 * -i network interface to attach to
 * -e eni.xml to use for parsing
 * -d process data output display rate in cyclic data cylces
 * -s cyclic data rate in microseconds
 * @return ddi_status_t
 * @see ddi_status_t
 */
int ddi_sdk_handle_cmdline (int argc, char **argv, cmdline_args_t *args);

/** ddi_sdk_get_cmdline_args
 * Returns the current command line arguments parsed by ddi_sdk_handle_cmdline()
 * @return ddi_status_t
 * @see ddi_status_t
 */
int ddi_sdk_get_cmdline_args (cmdline_args_t *args);

#ifdef __cplusplus
}
#endif

#endif // DDI_SDK_COMMON_H

