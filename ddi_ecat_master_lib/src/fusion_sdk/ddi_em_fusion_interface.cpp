/**************************************************************************
(c) Copyright 2022 Digital Dynamics Inc. Scotts Valley CA USA.
Unpublished copyright. All rights reserved. Contains proprietary and
confidential trade secrets belonging to DDI. Disclosure or release without
prior written authorization of DDI is prohibited.
**************************************************************************/

#include "ddi_em_fusion_interface.h"
#include "ddi_em_logging.h"
#include "ddi_em.h"
#include "ddi_em_fusion_uart.h"

#define FIRST_UART_MODULE_CHANNEL 1

// Global array of Fusion instance handles
static fusion_instance_type g_fusion_instance[DDI_EM_MAX_MASTER_INSTANCES][DDI_MAX_FUSION_INSTANCES] = {};

// Get the next available Fusion instance handle
static ddi_em_result get_next_fusion_instance (ddi_em_handle em_handle, ddi_fusion_sdk_handle *handle)
{
  int count;
  for ( count = 0; count < DDI_MAX_FUSION_INSTANCES; count++)
  {
    if ( g_fusion_instance[em_handle][count].is_allocated == 0 )
    {
      // Clear the instance state and configuration data
      memset(&g_fusion_instance[count], 0, sizeof(fusion_instance_type));
      *handle = count;
      g_fusion_instance[em_handle][count].is_allocated = 1;
      return DDI_EM_STATUS_OK;
    }
  }
  return DDI_EM_STATUS_NO_RESOURCES;
}

// Handle Fusion-specific extensions to process data
void ddi_em_fusion_handle_process_data (ddi_em_handle em_handle)
{
  ddi_em_instance *em_instance = get_master_instance(em_handle);
  fusion_instance_type *fusion_instance;
  uint fusion_count = 0;
  uint64_t uart_count = 0;
  for ( fusion_count = 0; fusion_count < DDI_MAX_FUSION_INSTANCES; fusion_count++ )
  {
    fusion_instance = &g_fusion_instance[em_handle][fusion_count];
    if ( fusion_instance->is_uart_event_registered == 0 )
    {
      // No UART instances are registered: exit
      break;
    }

    // Process any detected UART events
    ddi_fusion_uart_check_for_events (em_handle);
  }
}

// Sets up a process data descriptor from the process variable entry
void setup_em_pd_desc(EC_T_PROCESS_VAR_INFO_EX *entry, fusion_pd_desc_t *desc, uint8_t *pd_input, uint8_t *pd_output)
{
  // perform a one-time calculation to derive the byte offset, hi, lo, mask and size of the entry in bytes
  uint32_t offset = (uint32_t)entry->nBitOffs;
  uint32_t bitsize = (uint32_t)entry->nBitSize;
  uint16_t byte = ((offset + 8) / 8) - 1;
  uint32_t byten = (byte % 4) * 8;
  uint8_t lo = (offset - byten) % 32;
  uint8_t hi = (lo + bitsize - 1);
  uint32_t mask = BIT32_SHIFT_MASK(hi:lo);
  uint32_t size = BIT32_SIZE(hi:lo);
  size = (size + 7) >> 3;

  // setup descriptor
  desc->pd_input = pd_input;
  desc->pd_output = pd_output;
  desc->byte_offset = byte;
  desc->hi = hi;
  desc->lo = lo;
  desc->mask = mask;
  desc->size = size;
}

// Close all Fusion instance handles
ddi_em_result ddi_em_fusion_close_all_handles (ddi_em_handle em_handle)
{
  uint fusion_count = 0;
  for (fusion_count = 0; fusion_count < DDI_MAX_FUSION_INSTANCES; fusion_count++)
  {
     g_fusion_instance[em_handle][fusion_count].is_allocated = 0;
  }
  ddi_fusion_uart_close_all_handles();
  return DDI_EM_STATUS_OK;
}

// Open a Fusion instance handle
ddi_em_result ddi_em_open_fusion_interface (ddi_em_handle em_handle, EC_T_BUS_SLAVE_INFO* slave_info, ddi_fusion_sdk_handle *fusion_sdk_handle)
{
  EC_T_PROCESS_VAR_INFO_EX *pd_var_info;
  EC_T_CFG_SLAVE_INFO cfg_info;
  uint32_t status, number_of_entries, fusion_module_count = 0;
  uint16_t input, slot;
  uint8_t uart_channel_count = 0, uart_module_count = 0, physical_uart_channel_count = 0;
  ddi_em_result em_result = DDI_EM_STATUS_OK;
  EC_T_PROCESS_VAR_INFO_EX *entry;
  ddi_em_instance *master_instance;
  ddi_fusion_sdk_handle fusion_sdk_handle_local;
  uint8_t *input_byte_offset, *output_byte_offset;
  fusion_pd_desc_t *uart_pd_desc;

  VALIDATE_INSTANCE(em_handle); // Validate the instance argument
  
  // now get the offset of this device in the process data buffer
  status = emGetCfgSlaveInfo(em_handle, EC_TRUE, slave_info->wStationAddress, &cfg_info);
  if (status != 0)
  {
    ELOG(em_handle, "ERROR: ecatGetCfgSlaveInfo() returns with error: %s : 0x%x\n", ecatGetText(status), status);
    em_result = translate_ddi_acontis_err_code(em_handle, status); // Return the Acontis->DDI translated error code
    goto exit;
  }

  // Get the next available Fusion instance (if available)
  em_result = get_next_fusion_instance(em_handle, &fusion_sdk_handle_local);
  if ( em_result != DDI_EM_STATUS_OK )
  {
    ELOG(em_handle, "ERROR: get_next_fusion_instance: %s : 0x%x\n", ddi_em_get_error_string(em_result), status);
    em_result = translate_ddi_acontis_err_code(em_handle, status); // Return the Acontis->DDI translated error code
    goto exit;
  }

  // Get a pointer to the EtherCAT Master Instance
  master_instance=get_master_instance(em_handle);

  // Get the number of input and output entries
  number_of_entries = cfg_info.wNumProcessVarsInp + cfg_info.wNumProcessVarsOutp;

  // Reserve space to retrieve the process data information
  pd_var_info = (EC_T_PROCESS_VAR_INFO_EX *)calloc(number_of_entries, sizeof(EC_T_PROCESS_VAR_INFO_EX));

  // Get information regarding the input process data entries
  status = ecatGetSlaveInpVarInfoEx(EC_TRUE, slave_info->wStationAddress, cfg_info.wNumProcessVarsInp, pd_var_info, &input);
  if (status != EC_E_NOERROR)
  {
    ELOG(em_handle, "ERROR: 0x%x ecatGetSlaveInpVarInfoEx failed: %s \n", status, ecatGetText(status));
    em_result = translate_ddi_acontis_err_code(em_handle, status); // Return the Acontis->DDI translated error code
    goto exit;
  }

  // Get information regarding the output process data entries, append this to the input entries
  status = ecatGetSlaveOutpVarInfoEx(EC_TRUE, slave_info->wStationAddress, cfg_info.wNumProcessVarsOutp, &pd_var_info[input], &input);
  if (status != EC_E_NOERROR)
  {
    ELOG(em_handle, "ERROR: 0x%x ecatGetSlaveOutpVarInfoEx failed: %s\n", status, ecatGetText(status));
    em_result = translate_ddi_acontis_err_code(em_handle, status); // Return the Acontis->DDI translated error code
    goto exit;
  }

  // Interate through each input and output entry
  entry = pd_var_info;
  for (fusion_module_count = 0; fusion_module_count < number_of_entries; fusion_module_count++)
  {
    switch (entry->wIndex & DDI_FUSION_MODULE_INDEX_MASK)
    {
      case DDI_FUSION_UART_TXPDO_ENTRY_TYPE: // Register the Fusion-specific UART process data

        // Setup the UART process data copy descriptor
        input_byte_offset = master_instance->master_config.pd_input;
        output_byte_offset = master_instance->master_config.pd_output;

        // Get the UART process data descriptor pointer
        uart_pd_desc = ddi_fusion_uart_get_pd_desc(physical_uart_channel_count);
        setup_em_pd_desc(entry,uart_pd_desc, input_byte_offset, output_byte_offset);

        // Calculate the slot number
        slot = (entry->wIndex / DDI_FUSION_SLOT_INCREMENT) & 0xFF;
        // Map the EtherCAT index to the physical uart_channel (0-63)
        ddi_fusion_uart_map_slot_to_channel(slot, physical_uart_channel_count);
        if ( uart_channel_count % MAX_NUMBER_UART_PER_MODULE == (MAX_NUMBER_UART_PER_MODULE-1))  // New module type every 4 UART channels
        {
          // Reset the uart_channel_count variable every 4 channels
          uart_channel_count = 0;
        }

        // Increment the physical and logical UART counts
        physical_uart_channel_count++;
        uart_channel_count++;
        break;
      default: // No Fusion extension loaded
        break;
    }
    entry++;
  }

exit:
  if ( em_result == DDI_EM_STATUS_OK )
  {
    *fusion_sdk_handle = fusion_sdk_handle_local;
  }
  if ( pd_var_info ) // Free the process data variable information
  {
    free (pd_var_info);
  }
  return em_result;
}

// Set the UART registered event for a channel
void ddi_em_fusion_set_registered_uart_events (ddi_em_handle em_handle, ddi_fusion_sdk_handle fusion_sdk_handle, uint64_t uart_channel)
{
  g_fusion_instance[em_handle][fusion_sdk_handle].is_uart_event_registered |= uart_channel;
}
