/**************************************************************************
(c) Copyright 2021 Digital Dynamics Inc. Scotts Valley CA USA.
Unpublished copyright. All rights reserved. Contains proprietary and
confidential trade secrets belonging to DDI. Disclosure or release without
prior written authorization of DDI is prohibited.
**************************************************************************/

#include  "ddi_em_api.h"
#include  "ddi_em.h"
#include  "ddi_em_config.h"
#include  "ddi_debug.h"
#include  "ddi_macros.h"

// Transfer process data from ESC -> application buffer
// TODO: bounds check on input
EM_API ddi_em_result ddi_em_get_process_data(uint32_t em_handle, uint32_t offset, uint8_t *data, uint length, uint8_t is_output)
{
  VALIDATE_INSTANCE(em_handle); // Validate the instance argument
  uint8_t *s_ptr; // Source memory pointer
  ddi_em_instance *instance=get_master_instance(em_handle);
  if ( is_output )
    s_ptr = (uint8_t *)&instance->master_config.pd_output[offset];
  else
    s_ptr = (uint8_t *)&instance->master_config.pd_input[offset];
  // Copy from s
  if ( s_ptr != NULL)
    memcpy(data, s_ptr, length);
  return DDI_EM_STATUS_OK;
}

// Transfer process data from application buffer -> ESC
EM_API ddi_em_result ddi_em_set_process_data(ddi_em_handle em_handle, uint32_t offset, uint8_t *data, uint32_t length)
{
  VALIDATE_INSTANCE(em_handle); // Validate the instance argument
  ddi_em_instance *instance=get_master_instance(em_handle);
  // Set destination pointer
  uint8_t *d_ptr = (uint8_t *)&instance->master_config.pd_output[offset];
  if ( d_ptr != NULL)
  {
    // Protect updates of the process data occuring from multiple instances at the same time
    ddi_mutex_lock(instance->master_status.pd_out_mutex, DDI_TIMEOUT_FOREVER);
    memcpy(d_ptr, data, length);
    ddi_mutex_unlock(instance->master_status.pd_out_mutex);
  }
  return DDI_EM_STATUS_OK;
}

// Transfer process data from ESC -> application buffer, support bit-level access
EM_API ddi_em_result ddi_em_set_process_data_bits(ddi_em_handle em_handle, uint32_t src_bit_offset, uint32_t dst_bit_offset, uint8_t *data, uint32_t bit_length)
{
  VALIDATE_INSTANCE(em_handle); // Validate the instance argument
  // Get the byte offset of the source data
  uint32_t byte = ( (src_bit_offset + 8 ) / 8) - 1;
  uint8_t *d_ptr; // Source memory pointer
  ddi_em_instance *instance=get_master_instance(em_handle);
  d_ptr = (uint8_t *)&instance->master_config.pd_input[byte];
  // Copy from the source buffer into ESC memory
  EC_COPYBITS(d_ptr, dst_bit_offset, data, src_bit_offset, bit_length);
  return DDI_EM_STATUS_OK;
}

// Transfer process data from application buffer -> ESC, support bit-level access
EM_API ddi_em_result ddi_em_get_process_data_bits(ddi_em_handle em_handle, uint32_t src_bit_offset, uint32_t dst_bit_offset, uint8_t *data, uint32_t bit_length, uint8_t is_output )
{
  VALIDATE_INSTANCE(em_handle); // Validate the instance argument
  // Get the byte offset of the source data
  uint32_t byte = ( (src_bit_offset + 8 ) / 8) - 1;
  uint8_t *s_ptr; // Source memory pointer
  ddi_em_instance *instance=get_master_instance(em_handle);
  if ( is_output )
    s_ptr = (uint8_t *)&instance->master_config.pd_output[byte];
  else
    s_ptr = (uint8_t *)&instance->master_config.pd_input[byte];
  // Copy from ESC memory into dest buffer
  EC_COPYBITS(data, dst_bit_offset, s_ptr, src_bit_offset, bit_length);
  return DDI_EM_STATUS_OK;
}

