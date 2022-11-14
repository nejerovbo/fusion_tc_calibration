/**************************************************************************
(c) Copyright 2021-2022 Digital Dynamics Inc. Scotts Valley CA USA.
Unpublished copyright. All rights reserved. Contains proprietary and
confidential trade secrets belonging to DDI. Disclosure or release without
prior written authorization of DDI is prohibited.
**************************************************************************/

#ifndef DDI_EM_PLATFORM_H
#define DDI_EM_PLATFORM_H

/// @file ddi_em_supermicro_e300-8d.h

// Contains hardware definitions for the Supermicro E300 8D platform
// This file should be soft-linked as the <ddi_ecat_sdk>/include/ddi_em_platform.h file
// The ddi_em_platform.h will be linked by ddi_em_api.h

/*! @enum ddi_em_interface_select
//  @brief This enumeration supports identification of EtherCAT NICs for the Supermicro E300-8D platform
//
*/

// From the rear view of the Supermicro E300-8D where the six network interface ports are located:
//
//     _________                       _________                       _________
//    |         |                     |         |                     |         |
//    |  NIC4   |                     |  NIC2   |                     |   IP    |
//    |         |                     |         |                     |         |
//    \_________/                     \_________/                     \_________/
//
//     _________                       _________                       _________
//    |         |                     |         |                     |         |
//    |  NIC3   |                     |  NIC1   |                     |   IP    |
//    |         |                     |         |                     |         |
//    \_________/                     \_________/                     \_________/
//
// The ports marked as IP are reserved for network protocol communication, such as TCP or UDP are not available
// as EtherCAT master ports
//
typedef enum {
  DDI_EM_NIC_1 = 1,  /**< @brief 1st supported NIC */
  DDI_EM_NIC_2 = 2,  /**< @brief 2nd supported NIC */
  DDI_EM_NIC_3 = 3,  /**< @brief 3rd supported NIC */
  DDI_EM_NIC_4 = 4,  /**< @brief 4th supported NIC */
} ddi_em_interface_select;

/*! @enum ddi_em_cpu_select
  @brief This enumeration supports selection of the CPU affinity of the cyclic thread

  If enabled, the cyclic thread can be optionally be placed on a specific CPU.
*/
typedef enum {
  DDI_EM_CPU_MIN = 0,  /**< @brief Minimum CPU selection value */
  DDI_EM_CPU_1   = 0,  /**< @brief 1st supported CPU */
  DDI_EM_CPU_2   = 1,  /**< @brief 2nd supported CPU */
  DDI_EM_CPU_3   = 2,  /**< @brief 3rd supported CPU */
  DDI_EM_CPU_4   = 3,  /**< @brief 4th supported CPU */
  DDI_EM_CPU_MAX = 3,  /**< @brief Maximum CPU selection value */
} ddi_em_cpu_select;

#endif
