/**************************************************************************
(c) Copyright 2022 Digital Dynamics Inc. Scotts Valley CA USA.
Unpublished copyright. All rights reserved. Contains proprietary and
confidential trade secrets belonging to DDI. Disclosure or release without
prior written authorization of DDI is prohibited.
**************************************************************************/

/// @file ddi_em_fusion_interface.h

#ifndef DDI_EM_FUSION_INTERFACE_H
#define DDI_EM_FUSION_INTERFACE_H

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include <signal.h>
#include <AtEthercat.h>
#include "ddi_debug.h"
#include "ddi_em_api.h"
#include "ddi_em_fusion.h"
#include "ddi_em_config.h"
#include "ddi_em_translate.h"
#include "ddi_em_fusion_uart_api.h"
#include "ddi_em_fusion_uart.h"
#include "ddi_debug.h"
#include "ddi_macros.h"
#include "ddi_em_pd.h"

/*! @var FUSION_MODULE_INDEX_MASK
    @brief The Fusion Module Index mask
*/
#define DDI_FUSION_MODULE_INDEX_MASK   0xF00F

/*! @var ddi_fusion_sdk_handle
  @brief Defines the Fusion SDK handle type
*/
typedef int32_t ddi_fusion_sdk_handle;

#define DDI_FUSION_SDK_HANDLE_INVALID  -1

typedef struct {
  uint8_t          is_allocated = 0; /**< Is this slave instance allocated? */
  uint8_t          is_active = 0;    /**< Is this slave active? */
  uint64_t         is_uart_event_registered;
} fusion_instance_type;


/** ddi_em_fusion_handle_process_data
 @brief Handle fusion-specific process data
 @param[in] em_handle The EtherCAT Master handle
 */
ddi_em_result ddi_em_setup_fusion_interface (ddi_em_handle em_handle, EC_T_BUS_SLAVE_INFO* slave_info);

/** ddi_em_fusion_handle_process_data
 @brief Handle fusion-specific process data
 @param[in] em_handle The EtherCAT Master handle
 */
void ddi_em_fusion_handle_process_data (ddi_em_handle em_handle);

/** ddi_em_open_fusion_interface
 @brief Open a Fusion interface and return a Fusion SDK handle
 @param[in] sdk_handle The Fusion instance handle
 @param[in] slave_info The information regarding an EtherCAT slave
 @param[out] fusion_sdk_handle The allocated Fusion SDK handle
 */
ddi_em_result ddi_em_open_fusion_interface (ddi_em_handle em_handle, EC_T_BUS_SLAVE_INFO* slave_info, ddi_fusion_sdk_handle *fusion_sdk_handle);

/** ddi_fusion_close_all_handles
 @brief Close any open Fusion SDK handles. Used when the EtherCAT Master instance is de-initialized
 */
ddi_em_result ddi_em_fusion_close_all_handles (ddi_em_handle em_handle);

/** ddi_em_fusion_set_registered_uart_events
 @brief Registers this UART event with the Fusion UART event handler system
 @param[in] em_handle The EtherCAT Master instance handle
 @param[in] fusion_sdk_handle The Fusion SDK handle
 @param[in] uart_channel The UART channel
 */
void ddi_em_fusion_set_registered_uart_events (ddi_em_handle em_handle, ddi_fusion_sdk_handle fusion_sdk_handle, uint64_t uart_channel);

#endif
