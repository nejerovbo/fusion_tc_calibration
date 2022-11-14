/**************************************************************************
(c) Copyright 2022 Digital Dynamics Inc. Scotts Valley CA USA.
Unpublished copyright. All rights reserved. Contains proprietary and
confidential trade secrets belonging to DDI. Disclosure or release without
prior written authorization of DDI is prohibited.
**************************************************************************/

#include "ADCChip.h"
#include "IOChannel.h"
#include "SlotCard.h"
#include "CalibrationCommon.h"

ADCChip::ADCChip(void)
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

ADCChip::ADCChip(ddi_fusion_instance_t *fusion_instance, uint32_t pd_offset_out, uint32_t pd_offset_in)
{
  filename = "Default.csv";
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
}

ADCChip::ADCChip(ddi_fusion_instance_t *fusion_instance, uint32_t pd_offset_out, uint32_t pd_offset_in, string file_name)
{
  filename = file_name;
  int count;
  for(count = 0; count < AIO_8_CHANNEL; count++)
  {
    IOChannel temp(fusion_instance, pd_offset_out + count, pd_offset_in + count, filename);
    io_list[count] = temp;
  }
  fp = fopen(file_name.c_str(), "w");
  if( fp == NULL )
  {
    printf("Error: Failed to open file %s\n", file_name.c_str());
    return;
  }
  fprintf(fp, "Channel,Commanded Voltage,MultiMeter,");
  int sample_count;
  for(sample_count = 0; sample_count < NUM_AI_CAL_SAMPLES -1; sample_count++)
  {
    fprintf(fp, "Data%d,", sample_count);
  }
  // Terminate the calibration log file header
  fprintf(fp, "Data%d,ResidualError\n", sample_count);
}

ADCChip::ADCChip(ddi_fusion_instance_t *fusion_instance, uint32_t pd_offset_out, uint32_t pd_offset_in, FILE *file_pointer, int chip_id)
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

void ADCChip::write_to_log(uint channel, double commanded_voltage, double voltage)
{
  if ( fp != NULL )
  {
    int sample_count;
    fprintf(fp, "%d,%f,%f,", channel + (chip_index * 8), commanded_voltage, voltage);
    for(sample_count = 0; sample_count < NUM_AI_CAL_SAMPLES-1; sample_count++)
    {
      fprintf(fp, "%d,", output_array[sample_count]);
    }
    // Print the last data without a comma, MATLAB will throw an error during interpolation otherwise
    fprintf(fp, "%d", output_array[sample_count]);
    fprintf(fp, "\n");  
    fflush(fp);
  }
}
