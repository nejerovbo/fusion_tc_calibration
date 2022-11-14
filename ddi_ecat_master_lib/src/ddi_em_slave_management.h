/**************************************************************************
(c) Copyright 2022 Digital Dynamics Inc. Scotts Valley CA USA.
Unpublished copyright. All rights reserved. Contains proprietary and
confidential trade secrets belonging to DDI. Disclosure or release without
prior written authorization of DDI is prohibited.
**************************************************************************/

#ifndef DDI_EM_SLAVE_MANAGEMENT
#define DDI_EM_SLAVE_MANAGEMENT

#include "ddi_em_api.h"

/** ddi_em_close_all_slave_handles
 @brief Closes all open slave handles
 @param[in] handle The UART handle opened by ddi_fusion_uart_open
 @return ddi_em_result The result code of the operation @see ddi_em_result
 */
ddi_em_result ddi_em_close_all_slave_handles (ddi_em_handle em_handle);

#endif // DDI_EM_SLAVE_MANAGEMENT
