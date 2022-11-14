/******************************************************************************
 * (c) Copyright 2020 Digital Dynamics Inc. Scotts Valley CA USA.
 * Unpublished copyright. All rights reserved. Contains proprietary and
 * confidential trade secrets belonging to DDI. Disclosure or release without
 * prior written authorization of DDI is prohibited.
 *****************************************************************************/

#ifndef DDI_SDK_ECAT_SDO
#define DDI_SDK_ECAT_SDO

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "ddi_sdk_common.h"
#include "ddi_sdk_display.h"
#include <time.h>
#include <signal.h>
#include <AtEthercat.h>
#include "ddi_macros.h"
#include "ddi_status.h"
#include "ddi_sdk_processing.h"
#include "ddi_os.h"
#include "ddi_debug.h"
#include "ddi_sdk_ecat_master.h"

#ifdef __cplusplus
extern "C" {
#endif

//standard CDP ecat commands
#define DDI_COE_GET_DEVICE_VERSION                        1
#define DDI_COE_GET_DEVICE_NAME                           2
#define DDI_COE_GET_HW_VERSION                            3
#define DDI_COE_GET_SW_VERSION                            4
#define DDI_COE_GET_BL_VERSION                            5
#define DDI_COE_SET_DEVICE_RESET                          6
#define DDI_COE_SET_FACTORY_DEVICE_RESET                  7
#define DDI_COE_GET_NUMBER_OF_DETECTED_MODULES            8
#define DDI_COE_GET_DETECTED_MODULES                      9
#define DDI_COE_DETECT_MODULE_CMD                         10
#define DDI_COE_LOAD_PARAMETERS_CMD                       11
#define DDI_COE_STORE_PARAMETERS_CMD                      12
#define DDI_COE_CALCULATE_CHECKSUM                        13
#define DDI_COE_GET_NUMBER_OF_CONFIG_MODULES              14
#define DDI_COE_GET_CONFIGURED_MODULES                    15
#define DDI_COE_GET_MODULE_PROFILE_IDX_DIST               16
#define DDI_COE_GET_MODULE_MAX_MODULES                    17
#define DDI_COE_GET_MODULE_PROFILE_LIST                   18
#define DDI_COE_GET_ACTIVE_EXCEPTION_STATUS               19
#define DDI_COE_GET_ACTIVE_DEVICE_WARNING_DETAILS         20
#define DDI_COE_GET_ACTIVE_MANF_WARNING_DETAILS           21
#define DDI_COE_GET_ACTIVE_DEVICE_ERROR_DETAILS           22
#define DDI_COE_GET_ACTIVE_MANF_ERROR_DETAILS             23
#define DDI_COE_GET_ACTIVE_GLOBAL_DEV_WARNING_DETAILS     24
#define DDI_COE_GET_ACTIVE_GLOBAL_MANF_WARNING_DETAILS    25
#define DDI_COE_GET_ACTIVE_GLOBAL_DEV_ERROR_DETAILS       26
#define DDI_COE_GET_ACTIVE_GLOBAL_MANF_ERROR_DETAILS      27
#define DDI_COE_GET_LATCHED_EXCEPTION_STATUS              28
#define DDI_COE_GET_LATCHED_DEVICE_WARNING_DETAILS        29
#define DDI_COE_GET_LATCHED_MANF_WARNING_DETAILS          30
#define DDI_COE_GET_LATCHED_DEVICE_ERROR_DETAILS          31
#define DDI_COE_GET_LATCHED_MANF_ERROR_DETAILS            32
#define DDI_COE_GET_LATCHED_GLOBAL_DEV_WARNING_DETAILS    33
#define DDI_COE_GET_LATCHED_GLOBAL_MANF_WARNING_DETAILS   34
#define DDI_COE_GET_LATCHED_GLOBAL_DEV_ERROR_DETAILS      35
#define DDI_COE_GET_LATCHED_GLOBAL_MANF_ERROR_DETAILS     36
#define DDI_COE_SET_DEVICE_WARNING_MASK                   37
#define DDI_COE_SET_MANF_WARNING_MASK                     38
#define DDI_COE_SET_DEVICE_ERROR_MASK                     39
#define DDI_COE_SET_MANF_ERROR_MASK                       40
#define DDI_COE_SET_GLOBAL_DEVICE_WARNING_MASK            41
#define DDI_COE_SET_GLOBAL_MANF_WARNING_MASK              42
#define DDI_COE_SET_GLOBAL_DEVICE_ERROR_MASK              43
#define DDI_COE_SET_GLOBAL_MANF_ERROR_MASK                44
#define DDI_COE_SET_EXCEPTION_RESET                       45
#define DDI_COE_START_CALCULATE_CHECKSUM_MD5              46
#define DDI_COE_START_CALCULATE_CHECKSUM_DEFAULT          47
#define DDI_COE_GET_CALCULATE_CHECKSUM_RESULT             48
#define DDI_COE_GET_CDP_FGN                               49
#define DDI_COE_GET_MANF_SERIAL_NUMBER                    50
#define DDI_COE_GET_SDP_FGN                               51
#define DDI_COE_GET_VENDOR_NAME                           52
#define DDI_COE_GET_SDP_NAME                              53
#define DDI_COE_GET_OUTPUT_IDENTIFIER                     54
#define DDI_COE_GET_TIME_SINCE_POWERON                    55
#define DDI_COE_GET_FW_UDPATE_FGN                         56
#define DDI_COE_END                                       57

#define DDI_TX                 1
#define DDI_RX                 2

//max length of an SDO message
#define MAX_SDO_MSG_LENGTH 256*4

//sdo message structure
typedef struct {
  uint16_t id;                         //command id 
  uint16_t address;                    //sdo index address
  uint16_t subindex;                   //sdo subindex
  uint8_t  direction;                  //ddi tx or ddi rx
  uint16_t tx_amount;                  //amount for tx transfers
  uint16_t rx_amount;                  //amount for tx transfers
  uint8_t  buffer[MAX_SDO_MSG_LENGTH]; //buffer used for rx/tx messages
  uint32_t timeout_us;                 //timeout of the SDO in microseconds
  uint16_t sizeof_data_type;           //size of data type in bytes, 1, 2, 4
  uint16_t complete_access;            //complete access
} ddi_sdo_msg_t;

void display_alstatus (slave_t *slave);

/** ddi_sdk_ecat_coe_upload
 * Upload a message from SDO to the master over COE
 * The direction of this operation from slave -> master
 *
 * @param slave the slave instance
 * @param sdo_address the sdo address to read from
 * @param subindex the subindex to start reading from
 * @param msg   the returned message
 * @param msg_length the maximimum return message length
 * @param return_length the actual return message length
 * @param timeout_us the timeout value in microseconds
 * @param complete_access 0 = non-complete access, 1 = complete access
 * @return ddi_status_t
 * @see ddi_status_t
 */
ddi_status_t ddi_sdk_ecat_coe_upload(slave_t *slave, uint32_t sdo_address, uint32_t subindex, char *msg,
                                       uint32_t msg_length, uint32_t *return_length, uint32_t timeout_us, uint32_t complete_access);

/** ddi_sdk_ecat_coe_download
 * Upload a message from SDO to the master over COE
 * Is from master -> slave
 *
 * @param slave the slave instance
 * @param sdo_address the sdo index to write to
 * @param msg the message contents to write to
 * @param complete_access 0 = non-complete access, 1 = complete access
 * @return ddi_status_t
 * @see ddi_status_t
 */
ddi_status_t ddi_sdk_ecat_coe_download(slave_t *slave, uint32_t sdo_address, char *msg, uint32_t length, uint32_t complete_access);

/** ddi_sdk_ecat_sdo_msg
 * Transfer an SDO message to/from the slave using predefined messages defined in ddi_sdk_ecat_sdo.h
 * Is from slave -> master or master->slave
 *
 * @param slave the slave instance
 * @param sdo_cmd the sdo command to operate on
 * @param msg the message contents to write from or read to
 * @param msg_length the length of the message read or written
 * @return ddi_status_t
 * @see ddi_status_t
 */
ddi_status_t ddi_sdk_ecat_sdo_msg(slave_t *slave, uint32_t sdo_cmd, char *msg, uint32_t *msg_length );

/** ddi_sdk_ecat_read_obj_dictionary
 * Read COE object dictionary for the slave
 *
 * @param slave the slave instance
 * @param output_filename the logfile name to store the COE object dictionary
 * @param live_update query the SDO values of the objects, this will perform an extra slave->master transaction
 * @param timeout_us the timeout value in microseconds for each mailbox and SDO transacation
 * @return ddi_status_t
 * @see ddi_status_t
 */
ddi_status_t ddi_sdk_ecat_read_obj_dictionary( slave_t *slave, char *output_filename, uint8_t  live_update,uint32_t timeout_us);

#ifdef __cplusplus
}
#endif

#endif // DDI_SDK_ECAT_SDO

