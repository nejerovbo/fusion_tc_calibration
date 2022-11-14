/**************************************************************************
(c) Copyright 2022 Digital Dynamics Inc. Scotts Valley CA USA.
Unpublished copyright. All rights reserved. Contains proprietary and
confidential trade secrets belonging to DDI. Disclosure or release without
prior written authorization of DDI is prohibited.
**************************************************************************/

#ifndef DMM_UTIL_H
#define DMM_UTIL_H

#include <inttypes.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "sys/socket.h"
#include "arpa/inet.h"

double read_meter_volts();

int open_connection_to_meter();

void set_meter_to_local_mode();

int close_connection_to_meter();

double read_meter_ohms();

#define MAX_DAQ_CHANNELS 24

#endif
