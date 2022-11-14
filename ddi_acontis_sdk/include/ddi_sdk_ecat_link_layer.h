/******************************************************************************
 * (c) Copyright 2020-2021 Digital Dynamics Inc. Scotts Valley CA USA.
 * Unpublished copyright. All rights reserved. Contains proprietary and
 * confidential trade secrets belonging to DDI. Disclosure or release without
 * prior written authorization of DDI is prohibited.
 *****************************************************************************/

#ifndef __DDI_SDK_ECAT_LINK_LAYER__
#define __DDI_SDK_ECAT_LINK_LAYER__

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "ddi_sdk_common.h"
#include "ddi_sdk_display.h"
#include <time.h>
#include <signal.h>
#include <AtEthercat.h>
#include "ddi_macros.h"
#include "ddi_status.h"
#include "ddi_sdk_processing.h"
#include "ddi_os.h"
#include "ddi_debug.h"

//create the adapater for a raw socket interface
uint32_t create_link_params_sockraw(EC_T_LINK_PARMS** plink_primary_params, const char *iface_name);
//create the adapater for a i8254 optimized link layer interface
uint32_t create_link_params_i8254(EC_T_LINK_PARMS** plink_primary_params, uint32_t instance);

#endif