
/**************************************************************************
(c) Copyright 2022 Digital Dynamics Inc. Scotts Valley CA USA.
Unpublished copyright. All rights reserved. Contains proprietary and
confidential trade secrets belonging to DDI. Disclosure or release without
prior written authorization of DDI is prohibited.
**************************************************************************/

#include "IOChannel.h"
#include "DACChip.h"
#include "SlotCard.h"
#include "CalibrationCommon.h"

DACChip::DACChip(void)
{
  filename = "";
  int count;
  for(count = 0; count < sizeof(io_list); count++)
  {
    IOChannel temp;
    io_list[count] = temp;
  }
  fp = nullptr;
}
DACChip::DACChip(ddi_fusion_instance_t *fusion_instance, uint32_t pd_offset_out, uint32_t pd_offset_in)
{
  filename = "default.csv";
  int count;
  for(count = 0; count < AIO_8_CHANNEL; count++)
  {
    IOChannel temp(fusion_instance, pd_offset_out + count, pd_offset_in + count, filename);
    io_list[count] = temp;
  }
  fp = fopen(filename.c_str(), "w");
  if( fp == NULL )
  {
    printf("Error: Failed to open file %s\n", filename.c_str());
    return;
  }
  fprintf(fp,"Channel, MultiMeter, Data1, ResidualError, Timeout\n");
}
DACChip::DACChip(ddi_fusion_instance_t *fusion_instance, uint32_t pd_offset_out, uint32_t pd_offset_in, string file_name)
{
  filename = file_name;
  int count;
  for(count = 0; count < AIO_8_CHANNEL; count++)
  {
    IOChannel temp(fusion_instance, pd_offset_out + count, pd_offset_in + count, filename);
    io_list[count] = temp;
  }
  fp = fopen(filename.c_str(), "w");
  if( fp == NULL )
  {
    printf("Error: Failed to open file %s\n", filename.c_str());
    return;
  }
  fprintf(fp, "%s\n", file_name.c_str());
  fprintf(fp,"Channel, MultiMeter, Data, ResidualError, Timeout\n");
}

DACChip::DACChip(ddi_fusion_instance_t *fusion_instance, uint32_t pd_offset_out, uint32_t pd_offset_in, FILE *file_pointer, int chip_id)
{
  chip_index = chip_id;
  int count;
  for(count = 0; count < AIO_8_CHANNEL; count++)
  {
    IOChannel temp(fusion_instance, pd_offset_out + count + (chip_id * 8), pd_offset_in + count + (chip_id * 8), filename);
    io_list[count] = temp;
  }
  fp = file_pointer;
}

void DACChip::write_to_log(uint chn, double desired_voltage, uint dac_code_estimate, double residual_error, uint timeout_occurred)
{
  if ( fp != NULL )
  {
    fprintf(fp, "%d, %f, %d, %f, %d\n", chn, desired_voltage, dac_code_estimate, residual_error, timeout_occurred);
    fflush(fp);
  }
}
