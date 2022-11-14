/**************************************************************************
(c) Copyright 2021 Digital Dynamics Inc. Scotts Valley CA USA.
Unpublished copyright. All rights reserved. Contains proprietary and
confidential trade secrets belonging to DDI. Disclosure or release without
prior written authorization of DDI is prohibited.
**************************************************************************/

#ifndef __DDI_EM_UART_H__
#define __DDI_EM_UART_H__

#include "ddi_em_api.h"

void UART_SDO_impedance_match (ddi_em_handle master_handle, ddi_em_handle slave_handle);
ddi_em_result UART_select_baud (ddi_em_handle master_handle, ddi_em_handle slave_handle, const char *baud);
ddi_em_result UART_channel_flush (ddi_em_handle master_handle, ddi_em_handle slave_handle);
ddi_em_result UART_channel_select (ddi_em_handle master_handle, ddi_em_handle slave_handle, uint8_t channel);
void UART_cyclic_method (void *arg);

#endif
