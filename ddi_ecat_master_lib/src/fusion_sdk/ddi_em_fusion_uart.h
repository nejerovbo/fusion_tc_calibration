/**************************************************************************
(c) Copyright 2022 Digital Dynamics Inc. Scotts Valley CA USA.
Unpublished copyright. All rights reserved. Contains proprietary and
confidential trade secrets belonging to DDI. Disclosure or release without
prior written authorization of DDI is prohibited.
**************************************************************************/

/// @file ddi_em_fusion_uart.h

#ifndef DDI_EM_UART_H
#define DDI_EM_UART_H

#include "ddi_em_api.h"
#include "ddi_em_config.h"
#include "ddi_em_translate.h"
#include "ddi_em_fusion_uart_api.h"
#include "ddi_em_pd.h"

// There are 64 UART handles total in Version 1.3 across all EtherCAT Master handles
#define MAX_UART_INSTANCES 64

#define UART_DEFAULT_TIMEOUT_MS 100
// Each 5nn5 channel has 7 channels
#define SIZEOF_5005_CHANNEL 7
#define SIO_OFFSET          1

#define UART_COMPLETE_ACCESS 1
#define SIZEOF_SI0 (sizeof(uint16_t))

/** ddi_fusion_uart_get_pd_desc
 @brief Return the process data copy for the physical UART channel
 @param[in] The UART physical channel (0 to 63)
 @return The fusion process data pointer for this physical UART channel
 */
fusion_pd_desc_t* ddi_fusion_uart_get_pd_desc(uint16_t uart_physical_channel);

/** ddi_fusion_uart_map_slot_to_channel
 @brief Map the UART EtherCAT slot to a physical UART channel
 @param[in] index The UART EtherCAT index
 @param[in] uart_channel The UART channel
 */
void ddi_fusion_uart_map_slot_to_channel(uint16_t index, uint16_t uart_channel);

/** ddi_fusion_uart_check_for_events
 @brief Check for UART events on the given EtherCAT Master handle
 @param[in] em_handle The EtherCAT Master handle
 */
ddi_em_result ddi_fusion_uart_check_for_events (ddi_em_handle em_handle);

/** ddi_fusion_uart_close_all_handles
 @brief Close any open UART handles.  Used when the EtherCAT Master instance is de-initialized
 */
ddi_em_result ddi_fusion_uart_close_all_handles (void);

/*! @var VALIDATE_UART_INSTANCE
  @brief  Macro to validate the UART instance argument to a function
*/
#define VALIDATE_UART_INSTANCE(instance) do{ if ((instance > MAX_UART_INSTANCES) || (instance < 0))\
                                         { printf("DDI ECAT SDK UART: Invalid instance %d \n", instance); return DDI_EM_STATUS_INVALID_INSTANCE;\
                                         }\
                                      }while(0)

#endif
