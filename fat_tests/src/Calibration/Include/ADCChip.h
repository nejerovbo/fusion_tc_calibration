/**************************************************************************
(c) Copyright 2022 Digital Dynamics Inc. Scotts Valley CA USA.
Unpublished copyright. All rights reserved. Contains proprietary and
confidential trade secrets belonging to DDI. Disclosure or release without
prior written authorization of DDI is prohibited.
**************************************************************************/

#ifndef ADCCHIP_H
#define ADCCHIP_H

#include <fcntl.h>
#include <stdint.h>
#include "AcontisTestFixture.h"
#include "IOChannel.h"
#include "SlotCard.h"
#include "CalibrationCommon.h"

class ADCChip : public SlotCard
{
private:
  IOChannel io_list[AIO_8_CHANNEL];
  FILE* fp;
  string filename;
  uint16_t output_array[NUM_AI_CAL_SAMPLES];
  int chip_index;
  int cal_sucess_status = 1;

public:
  ADCChip(void);
  ADCChip(ddi_fusion_instance_t *fusion_instance, uint32_t pd_offset_out, uint32_t pd_offset_in);
  ADCChip(ddi_fusion_instance_t *fusion_instance, uint32_t pd_offset_out, uint32_t pd_offset_in, string file_name);
  ADCChip(ddi_fusion_instance_t *fusion_instance, uint32_t pd_offset_out, uint32_t pd_offset_in, FILE *file_pointer, int chip_id);
  IOChannel get_ioChannel(int index)                  { return io_list[index]; }
  void      set_ioChannel(int index, IOChannel arg)   { io_list[index] = arg; }
  void      set_file_pointer(FILE* arg)               { fp = arg; }
  FILE*     get_file_pointer()                        { return fp; }
  void      set_output_array(int index, uint16_t val) { output_array[index] = val; }
  uint16_t  get_output_array(int index)               { return output_array[index]; }
  int       get_chip_id()                             { return chip_index; }
  void      write_to_log(uint channel, double commanded_voltage, double voltage);
  int get_cal_success_status() { return cal_sucess_status; }
  void set_cal_success_status(int arg) { cal_sucess_status = arg;}
};
#endif
