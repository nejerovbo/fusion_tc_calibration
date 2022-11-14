/**************************************************************************
(c) Copyright 2022 Digital Dynamics Inc. Scotts Valley CA USA.
Unpublished copyright. All rights reserved. Contains proprietary and
confidential trade secrets belonging to DDI. Disclosure or release without
prior written authorization of DDI is prohibited.
**************************************************************************/

#ifndef DAQ_UTIL_H
#define DAQ_UTIL_H

#include <inttypes.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <vector>
#include "sys/socket.h"
#include "arpa/inet.h"
#include "CalibrationCommon.h"

#define MAX_DAQ_CHANNELS    24
#define MAX_DAQ_ENTRIES     4096

using namespace std;

class DAQ {
private:
  int daq_init;
  double meter_data[MAX_DAQ_ENTRIES];
  int daq_socket;
  std::vector<string> requests;
public:
  DAQ(void);

  double daq_get_channel_reading(uint channel);

  void daq_collect_data(uint channel_start, uint channel_stop);

  int daq_connect(const char *daq_ip_addr);

  void add_channel(int module, int bank, int channel, int is_16_ch);

  void daq_collect_data_multi(int start_index, int end_index);

  double get_meter_data(int index) { return meter_data[index]; };

  int get_requests_size() { return requests.size(); }

  int get_daq_init_status() { return daq_init; };

  void clear_meter_data_and_requests();
};
#endif
