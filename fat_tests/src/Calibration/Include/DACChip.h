/**************************************************************************
(c) Copyright 2022 Digital Dynamics Inc. Scotts Valley CA USA.
Unpublished copyright. All rights reserved. Contains proprietary and
confidential trade secrets belonging to DDI. Disclosure or release without
prior written authorization of DDI is prohibited.
**************************************************************************/

#ifndef DACCHIP_H
#define DACCHIP_H

#include "IOChannel.h"
#include "ADCChip.h"
#include "SlotCard.h"
#include <fcntl.h>
#include <stdint.h>
#include "AcontisTestFixture.h"

class DACChip : public SlotCard
{
private:
  IOChannel io_list[AIO_8_CHANNEL];
  FILE* fp;
  string filename;
  int chip_index;
  int cal_sucess_status = 1;
  
public:
  DACChip(void);
  DACChip(ddi_fusion_instance_t *fusion_instance, uint32_t pd_offset_out, uint32_t pd_offset_in);
  DACChip(ddi_fusion_instance_t *fusion_instance, uint32_t pd_offset_out, uint32_t pd_offset_in, string file_name);
  DACChip(ddi_fusion_instance_t *fusion_instance, uint32_t pd_offset_out, uint32_t pd_offset_in, FILE *file_pointer, int chip_id);
  IOChannel get_ioChannel(int index)                 { return io_list[index]; }
  void      set_ioChannel(int index, IOChannel arg)  { io_list[index] = arg; }
  FILE*     get_file_pointer()                       { return fp; }
  void      set_file_pointer(FILE* file_pointer)     { fp = file_pointer; }
  void      write_to_log(uint chn, double desired_voltage, uint dac_code_estimate, double residual_error, uint timeout_occurred);
  int       get_chip_id()  { return chip_index; };
  int get_cal_success_status() { return cal_sucess_status; }
  void set_cal_success_status(int arg) { cal_sucess_status = arg;}
};

#endif
