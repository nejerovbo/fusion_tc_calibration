/**************************************************************************
(c) Copyright 2022 Digital Dynamics Inc. Scotts Valley CA USA.
Unpublished copyright. All rights reserved. Contains proprietary and
confidential trade secrets belonging to DDI. Disclosure or release without
prior written authorization of DDI is prohibited.
**************************************************************************/

#ifndef TCCARD_H
#define TCCARD_H

#include <fcntl.h>
#include <stdint.h>
#include "AcontisTestFixture.h"
#include "IOChannel.h"
#include "SlotCard.h"
#include "CalibrationCommon.h"

class TCCard : public SlotCard
{
private:
  IOChannel io_list[AIO_8_CHANNEL];
  FILE *fp;
  int cal_sucess_status = 1;

public:
  TCCard(void);
  TCCard(ddi_fusion_instance_t *fusion_instance, int pd_offset_out, int pd_offset_in, FILE *fp);
  IOChannel get_ioChannel(int index)                  { return io_list[index]; }
  void      set_ioChannel(int index, IOChannel arg)   { io_list[index] = arg; }
  void      set_file_pointer(FILE* arg)               { fp = arg; }
  FILE*     get_file_pointer()                        { return fp; }
  int       get_cal_success_status()                  { return cal_sucess_status; }
  void      set_cal_success_status(int arg)           { cal_sucess_status = arg;}
  void      write_to_log(uint channel, double commanded_voltage, double voltage);
};

#endif
