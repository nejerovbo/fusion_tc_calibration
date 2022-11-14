/**************************************************************************
(c) Copyright 2021 Digital Dynamics Inc. Scotts Valley CA USA.
Unpublished copyright. All rights reserved. Contains proprietary and
confidential trade secrets belonging to DDI. Disclosure or release without
prior written authorization of DDI is prohibited.
**************************************************************************/

#ifndef DDI_EC_LINK_LAYER_H
#define DDI_EC_LINK_LAYER_H

#include <stdint.h>
#include <AtEthercat.h>
#include "ddi_em_api.h"

/** link_layer_i8254_init
 @brief Create the link layer for the i8254 interface
 @param instance The Master instance handle
 @param plink_primary_params The link layer parameters used in initialization
 @param interface Which NIC to execute from
 @return ddi_em_result The result code of the operation, see ddi_em_result for details
 @see ddi_em_result
 */
ddi_em_result link_layer_i8254_init(ddi_em_handle instance, EC_T_LINK_PARMS** plink_primary_params, ddi_em_interface_select interface);

/** link_layer_adapter_allocated
 @brief Determine if the network adapater is already allocated
 @param nic_interface The network interface to determine if it's already been bound to another instance
 @return ddi_em_result The result code of the operation, see ddi_em_result for details
 @see ddi_em_result
 */
ddi_em_result link_layer_adapter_allocated (ddi_em_interface_select nic_interface);

/** link_layer_deinit
 @brief De-initialize the link layer instance
 @param instance The master instance handle
 @return ddi_em_result The result code of the operation, see ddi_em_result for details
 @see ddi_em_result
 */
void link_layer_deinit (ddi_em_handle instance);

#endif // DDI_EC_LINK_LAYER_H
