/**************************************************************************
(c) Copyright 2022 Digital Dynamics Inc. Scotts Valley CA USA.
Unpublished copyright. All rights reserved. Contains proprietary and
confidential trade secrets belonging to DDI. Disclosure or release without
prior written authorization of DDI is prohibited.
**************************************************************************/

#ifndef IOPOINT_H
#define IOPOINT_H

#include <fcntl.h>
#include <stdint.h>
#include "AcontisTestFixture.h"

class IOChannel
{
private:
  uint32_t pd_offset_in, pd_offset_out;
  ddi_fusion_instance_t *fusion_instance;
  std::string filename;
  int quanta_found;


public:
  IOChannel(void);
  IOChannel(ddi_fusion_instance_t *fusion_instance, uint32_t pd_offset_out, uint32_t pd_offset_in);
  IOChannel(ddi_fusion_instance_t *fusion_instance, uint32_t pd_offset_out, uint32_t pd_offset_in, string file_name);
  void     set_output(uint16_t arg);

  uint16_t get_input();

  void     set_pd_offset_in(uint32_t arg) { pd_offset_in = arg; }

  void     set_pd_offset_out(uint32_t arg) { pd_offset_out = arg; }

  void     set_fusion_instance(ddi_fusion_instance_t *arg) { fusion_instance = arg; }

  uint32_t get_pd_offset_in() { return pd_offset_in; }

  uint32_t get_pd_offset_out() { return pd_offset_out; }

  ddi_fusion_instance_t* get_fusion_instance() { return fusion_instance; }

  string get_log_filename() { return filename; }

  void set_log_filename(string arg) { filename = arg; }

  void toggle_quanta_found();

  int get_quanta_found() { return quanta_found; }

};
#endif
