#ifndef DC_UTIL_H
#define DC_UTIL_H

#include <inttypes.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "sys/socket.h"
#include "arpa/inet.h"

enum vr_e{ONE_VOLT = 0, TEN_VOLTS = 1, ONE_HUNDRED_VOLTS = 2};


int DC_init();

void set_DC_voltage(double voltage);
void set_DC_range(vr_e voltage_range);
void enable_DC_out(void);
void disable_DC_out(void);

#endif
