/**************************************************************************
(c) Copyright 2022 Digital Dynamics Inc. Scotts Valley CA USA.
Unpublished copyright. All rights reserved. Contains proprietary and
confidential trade secrets belonging to DDI. Disclosure or release without
prior written authorization of DDI is prohibited.
**************************************************************************/

#ifndef DDI_EM_REMOTE_ACCESS
#define DDI_EM_REMOTE_ACCESS

#include "ddi_em_api.h"

/*! @struct ddi_em_remote_access_instance
  \brief Provides per-instance support of remote access configuration and status
*/
typedef struct {
  uint8_t  is_allocated;            /**< Is the remote access server allocated? */
  uint32_t remote_access_enabled;   /**< Remote access is enabled for this instance */
  void*    remote_access_handle;    /**< Remote access handle for this instance*/
} ddi_em_remote_access_instance;

/** ddi_em_remote_access_clear_instance_data
 @brief Initialize the the remote access instance structure. Called by ddi_sdk_init()
 */
void ddi_em_remote_access_clear_instance_data (void);

#endif
