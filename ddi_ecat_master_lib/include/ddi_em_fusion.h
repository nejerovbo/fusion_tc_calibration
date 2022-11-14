/**************************************************************************
(c) Copyright 2021-2022 Digital Dynamics Inc. Scotts Valley CA USA.
Unpublished copyright. All rights reserved. Contains proprietary and
confidential trade secrets belonging to DDI. Disclosure or release without
prior written authorization of DDI is prohibited.
**************************************************************************/

#ifndef DDI_EM_FUSION_H
#define DDI_EM_FUSION_H

// Support for Fusion-specific interfaces

/// @file ddi_em_fusion.h

/*! @var DDI_ETHERCAT_VENDOR_ID
    @brief This value represents the digital dynamics EtherCAT Vendor ID
*/
#define DDI_ETHERCAT_VENDOR_ID             0x76B
/*! @var DDI_FUSION_PRODUCT_CODE
    @brief This value represents the digital dynamics EtherCAT Product Code
*/
#define DDI_FUSION_PRODUCT_CODE            0x64

/*! @var DDI_FUSION_1096_REV
    @brief This value represents the revision code for Fusion firmware version 1.09.6
*/
#define DDI_FUSION_1096_REV                0x00041096

/*! @var DDI_FUSION_SLOT_INCREMENT
    @brief The index increment between Fusion EtherCAT MDP modules
*/
#define DDI_FUSION_SLOT_INCREMENT          0x10

/*! @var DDI_FUSION_MAX_ETHERCAT_MODULES
    @brief Maximum number of Fusion EtherCAT modules
*/
#define DDI_FUSION_MAX_ETHERCAT_MODULES    0xFF

/*! @var MAX_FUSION_INSTANCES
    @brief Maximum number of Fusion EtherCAT devices supported on the same network
*/
#define DDI_MAX_FUSION_INSTANCES           16

#endif // __DDI_EM_FUSION_H__
