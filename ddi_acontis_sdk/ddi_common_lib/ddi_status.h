/*****************************************************************************
 * (c) Copyright 2018 Digital Dynamics Inc. Scotts Valley CA USA.
 * Unpublished copyright. All rights reserved. Contains proprietary and
 * confidential trade secrets belonging to DDI. Disclosure or release without
 * prior written authorization of DDI is prohibited.
 *****************************************************************************/
/*  ddi_status.h
 *  Created by Johana Lehrer on 2018-10-02
 */

#ifndef DDI_STATUS_H
#define DDI_STATUS_H

#include <stdint.h>

/** Return status codes
 *
 */
#define  ddi_status_ok               0  /**< operation was successful */
#define  ddi_status_no_resources    -1  /**< there were no resources available */
#define  ddi_status_param_err       -2  /**< one or more parameters were invalid */
#define  ddi_status_timeout         -3  /**< the operation timed out before it completed */
#define  ddi_status_not_ready       -4  /**< resources were not ready to be used */
#define  ddi_status_queue_full      -5  /**< the queue was full */
#define  ddi_status_not_allowed     -6  /**< operation not allowed: e.g. can't call from ISR context */
#define  ddi_status_device_busy     -7  /**< hardware device busy */
#define  ddi_status_device_err      -8  /**< hardware device error */
#define  ddi_status_system_err      -9  /**< platform system software error */
#define  ddi_status_config_err      -10 /**< invalid configuration or not configured error */
#define  ddi_status_err             -11 /**< unspecified error */
#define  ddi_status_not_found       -12 /**< requested item does not exist or was not found */

typedef int ddi_status_t;

#endif // DDI_STATUS_H

