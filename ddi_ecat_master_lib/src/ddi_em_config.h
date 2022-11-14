/**************************************************************************
(c) Copyright 2021 Digital Dynamics Inc. Scotts Valley CA USA.
Unpublished copyright. All rights reserved. Contains proprietary and
confidential trade secrets belonging to DDI. Disclosure or release without
prior written authorization of DDI is prohibited.
**************************************************************************/

#ifndef DDI_EM_CONFIG_H
#define DDI_EM_CONFIG_H

#include "ddi_em_api.h"

/*! @var DDI_EM_MAX_BUS_SLAVES
  @brief Max number of bus slave supported
*/
#define DDI_EM_MAX_BUS_SLAVES             256

/*! @var DDI_EM_MAX_ACYC_FRAMES_QUEUED
  @brief Max number of acyc frames queued, 127 = the absolute maximum number
*/
#define DDI_EM_MAX_ACYC_FRAMES_QUEUED     32

/*! @var DDI_EM_MAX_ACYC_BYTES_PER_CYC_1MS
  @brief Max number of bytes sent during eUsrJob_SendAcycFrames within one cycle, for 1ms and above train rates
*/
#define DDI_EM_MAX_ACYC_BYTES_PER_CYC_1MS 4096

/*! @var DDI_EM_MAX_ACYC_BYTES_PER_CYC
  @brief Max number of bytes sent during eUsrJob_SendAcycFrames within one cycle
*/
#define DDI_EM_MAX_ACYC_BYTES_PER_CYC     1500

/*! @var DDI_EM_CFG_MAX_ACYC_CMD_RETRIES
  @brief Max retry count for acyclic frames
*/
#define DDI_EM_CFG_MAX_ACYC_CMD_RETRIES   3

/*! @var DDI_EM_MAX_ACYC_FRAMES_PER_CYCLE
  @brief This defines how many acyclic frames to send per cycle
*/
#define DDI_EM_MAX_ACYC_FRAMES_PER_CYCLE  1

/*! @var DDI_EM_MAX_ACYC_CMD_PER_CYCLE
  @brief This defines how many acyclic commands to send per cycle
*/
#define DDI_EM_MAX_ACYC_CMD_PER_CYCLE     20

/*! @var DDI_EM_LICENSE_FILE
  @brief Acontis License file location
*/
#define DDI_EM_LICENSE_FILE               "/home/ddi/ddi_em/config/license.txt"

/*! @var DDI_EM_LOG_DIR
  @brief Log file location
*/
#define DDI_EM_LOG_DIR                    "/home/ddi/ddi_em/ddi_em_log_files"

/*! @var DDI_EM_LOG_FILE_PREFIX
  @brief Log files start with this prefix
*/
#define DDI_EM_LOG_FILE_PREFIX            "ddi_em_log"

/*! @var DDI_EM_LOST_FRAME_COUNT_MAX
  @brief Log an additional error when this threshold is reached
*/
#define DDI_EM_LOST_FRAME_COUNT_MAX       15

/*! @var DDI_EM_SCAN_NETWORK_TIMEOUT
  @brief Defines the timeout for the inital bus scan, in microseconds
*/
#define DDI_EM_SCAN_NETWORK_TIMEOUT       10000

/*! @var DDI_CYCLIC_THREAD_STACK_SIZE
  @brief Defines the cyclic thread stack size in bytes
*/
#define DDI_CYCLIC_THREAD_STACK_SIZE      2048

/*! @var ACONTIS_SUCCESS
  @brief Defines success for the Acontis API, replaces EC_E_NO_ERROR
*/
#define ACONTIS_SUCCESS                   0

/*! @var REMOTE_ACCESS_PORT_DEFAULT
  @brief Defines the remote access TCP port default
*/
#define REMOTE_ACCESS_PORT_DEFAULT        6000

/*! @var REMOTE_ACCESS_DEFAULT_CYCLE
  @brief Defines the update rate of the Remote access server in microseconds
*/
#define REMOTE_ACCESS_DEFAULT_CYCLE       2000 // The network commands will be checked every 2000 microseconds

/*! @var REMOTE_ACCESS_SHUTDOWN_TIMEOUT_MS
  @brief Defines the update rate of the Remote access server in microseconds
*/
#define REMOTE_ACCESS_SHUTDOWN_TIMEOUT_MS 1000 // This is the timeout value during a remote access server shutdown

/*! @var VALIDATE_INSTANCE
  @brief  Macro to validate the instance argument to a function
*/
#define VALIDATE_INSTANCE(instance) do{ if ((instance >DDI_EM_MAX_MASTER_INSTANCES) || (instance < 0))\
                                         { printf("DDI ECAT SDK: Invalid instance %d \n", instance); return DDI_EM_STATUS_INVALID_INSTANCE;\
                                         }\
                                      }while(0)
/*! @var EM_API
  @brief Indicated API call supported in ddi_em_api.h
*/
#define EM_API
#endif
