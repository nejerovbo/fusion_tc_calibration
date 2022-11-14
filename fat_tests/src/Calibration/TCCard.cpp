/**************************************************************************
(c) Copyright 2022 Digital Dynamics Inc. Scotts Valley CA USA.
Unpublished copyright. All rights reserved. Contains proprietary and
confidential trade secrets belonging to DDI. Disclosure or release without
prior written authorization of DDI is prohibited.
**************************************************************************/
#include "AcontisTestFixture.h"
#include "CRAMProcessData.h"
#include "TCCard.h"
#include "IOChannel.h"
#include "SlotCard.h"
#include "CalibrationCommon.h"

TCCard::TCCard(void)
{
  int count;
  for(count = 0; count < sizeof(io_list); count++)
  {
    IOChannel temp;
    io_list[count] = temp;
  }
  fp = nullptr;
}

TCCard::TCCard(ddi_fusion_instance_t *fusion_instance, int pd_offset_out, int pd_offset_in, FILE *fp)
{
  int count;
  for(count = 0; count < sizeof(io_list); count++)
  {
    IOChannel temp(fusion_instance, pd_offset_out + count, pd_offset_in + count);
    io_list[count] = temp;
  }

  fp = fp;
}

void TCCard::write_to_log(uint channel, double commanded_voltage, double voltage)
{

}