/**************************************************************************
(c) Copyright 2022 Digital Dynamics Inc. Scotts Valley CA USA.
Unpublished copyright. All rights reserved. Contains proprietary and
confidential trade secrets belonging to DDI. Disclosure or release without
prior written authorization of DDI is prohibited.
**************************************************************************/

/**
 * Each instance of IOChannel holds a speciic offset and enables access to read/write to that channel with EtherCAT
 * It also holds a flag that indicates whether or not an accurate quanta has been found for that channel
 * */
#include "IOChannel.h" 
#include "CalibrationCommon.h"

IOChannel::IOChannel(void)
{
  pd_offset_in = 0;
  pd_offset_out = 0;
  fusion_instance = nullptr;
  filename = "";
  quanta_found = 0;
}

IOChannel::IOChannel(ddi_fusion_instance_t *fusion, uint32_t offset_out, uint32_t offset_in)
{
  fusion_instance = fusion;
  pd_offset_in = offset_in;
  pd_offset_out = offset_out;
  filename = "";
  quanta_found = 0;
}

IOChannel::IOChannel(ddi_fusion_instance_t *fusion, uint32_t offset_out, uint32_t offset_in, string file_name)
{
  fusion_instance = fusion;
  pd_offset_in = offset_in;
  pd_offset_out = offset_out;
  filename = file_name;
  quanta_found = 0;
}

void IOChannel::set_output(uint16_t arg)
{
  ddi_sdk_fusion_set_aout(fusion_instance, pd_offset_out, arg);
}

uint16_t IOChannel::get_input()
{
  return ddi_sdk_fusion_get_ain(fusion_instance, pd_offset_in);
}

void IOChannel::toggle_quanta_found()
{
  if( quanta_found )
  {
    quanta_found = 0;
  }
  else
  {
    quanta_found = 1;
  }
}
