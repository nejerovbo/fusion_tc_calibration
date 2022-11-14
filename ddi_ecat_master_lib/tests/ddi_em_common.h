/**************************************************************************
(c) Copyright 2021 Digital Dynamics Inc. Scotts Valley CA USA.
Unpublished copyright. All rights reserved. Contains proprietary and
confidential trade secrets belonging to DDI. Disclosure or release without
prior written authorization of DDI is prohibited.
**************************************************************************/

#ifndef __DDI_TEST_COMMON_H__
#define __DDI_TEST_COMMON_H__

#include "ddi_defines.h"
#include "ddi_status.h"

// Return the keyboard input in a blocking or non-blocking manner
ddi_status_t ddi_get_keyboard_input(char *key, bool blocking);

#define clrscr() printf("\e[1;1H\e[2J")

#define TEST_DEFAULT_TIMEOUT 10000

#endif
