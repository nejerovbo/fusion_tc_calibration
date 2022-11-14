/**************************************************************************
(c) Copyright 2021 Digital Dynamics Inc. Scotts Valley CA USA.
Unpublished copyright. All rights reserved. Contains proprietary and
confidential trade secrets belonging to DDI. Disclosure or release without
prior written authorization of DDI is prohibited.
**************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include <signal.h>
#include <AtEthercat.h>
#include "ddi_debug.h"
#include "ddi_em_api.h"
#include "ddi_em_logging.h"
#include "ddi_em_config.h"
#include "ddi_em.h"
#include "ddi_em_translate.h"

// Update the slave information structure with information from the bus scan
static ddi_em_result update_slave_info_from_bus_scan (ddi_em_handle em_handle, ddi_em_handle slave_instance, EC_T_BUS_SLAVE_INFO *bus_info)
{
  VALIDATE_INSTANCE(em_handle);
  ddi_em_slave *slv_info = get_slave_instance(em_handle, slave_instance);
  if ( (slv_info == NULL) || (bus_info == NULL))
  {
    return DDI_EM_STATUS_NULL_ARGUMENT; // Unsupported argument
  }

  // Transfer bus scan information into the slave information structure
  slv_info->cfg_info.serial_number = bus_info->dwSerialNumber;
  slv_info->cfg_info.vendor_id = bus_info->dwVendorId;
  slv_info->cfg_info.revision = bus_info->dwRevisionNumber;
  slv_info->cfg_info.product_code = bus_info->dwProductCode;
  slv_info->cfg_info.station_address = bus_info->wStationAddress;
  return DDI_EM_STATUS_OK;
}

// Update the slave information structure with information from the config query
static ddi_em_result update_slave_info_from_config_query (ddi_em_handle em_handle, ddi_em_handle slave_instance, EC_T_CFG_SLAVE_INFO *cfg_info)
{
  VALIDATE_INSTANCE(em_handle);
  ddi_em_slave *slv_info = get_slave_instance(em_handle, slave_instance);
  if ( (slv_info == NULL) || (cfg_info == NULL))
  {
    return DDI_EM_STATUS_NULL_ARGUMENT; // Unsupported argument
  }

  // Update the process data portion of the slave information structure, convert to bytes
  // Bytes are used to make the interfacing to the SetProcessData() and GetProcessData() more convenient 
  slv_info->cfg_info.pd_input_offset = cfg_info->dwPdOffsIn/8;
  slv_info->cfg_info.pd_output_offset = cfg_info->dwPdOffsOut/8;
  slv_info->cfg_info.pd_input_size = cfg_info->dwPdSizeIn/8;
  slv_info->cfg_info.pd_output_size = cfg_info->dwPdSizeOut/8;
  return DDI_EM_STATUS_OK;
}

static ddi_em_result open_slave_handle(ddi_em_handle em_handle, EC_T_BUS_SLAVE_INFO *bus_info, ddi_es_handle *es_handle)
{
  ddi_em_result result = DDI_EM_STATUS_OK;
  ddi_fusion_sdk_handle fusion_sdk_handle;
  ddi_em_instance *em_instance;
  em_instance = get_master_instance(em_handle);
  /* slave found */
  update_slave_info_from_bus_scan(em_handle, bus_info->dwSlaveId, bus_info);
  *es_handle = bus_info->dwSlaveId;

  // If this is a Fusion, enable the Fusion-specific handling (v1.3 - Support UART event notification)
  if ((bus_info->dwVendorId == DDI_ETHERCAT_VENDOR_ID) && (bus_info->dwProductCode == DDI_FUSION_PRODUCT_CODE))
  {
    // Open the Fusion SDK handle
    result = ddi_em_open_fusion_interface (em_handle, bus_info, &fusion_sdk_handle);
    if ( result == DDI_EM_STATUS_OK )
    {
      em_instance->slave_info[*es_handle].fusion_handle = fusion_sdk_handle;
      em_instance->ddi_fusion_count++;
      if ( em_instance->ddi_fusion_count == DDI_MAX_FUSION_INSTANCES )
      {
        ELOG(em_handle, "Master[%d] open_slave_handle: max number of Fusion instances exceeded \n", em_handle);
        return DDI_ES_MAX_NUM_OF_FUSIONS_ERR;
      }
    }
  }
  return result;
}

// Open the slave instance by vendor_id (required), product_id (required), revision_number (optional), serial_number (optional)
EM_API ddi_em_result ddi_em_open_slave_by_id(ddi_em_handle em_handle, uint32_t vendor_id, uint32_t product_id, uint32_t revision, uint32_t serial_number, ddi_es_handle *es_handle)
{
  uint32_t result = EC_E_ERROR;
  uint32_t slave_index = 0;

  VALIDATE_INSTANCE(em_handle);

  DLOG(em_handle, "Master[%d] open slave by id: number of connected slaves %d \n", em_handle, emGetNumConnectedSlaves(em_handle));
  // For each slave on the bus, scan information about that slave and perform the vendor_id and product_id comparison
  for (slave_index = 0; slave_index < emGetNumConnectedSlaves(em_handle); slave_index++)
  {
    uint16_t            wAutoIncAddress = slave_index;
    EC_T_BUS_SLAVE_INFO bus_slave_info;

    // get information about bus slave
    result = emGetBusSlaveInfo(em_handle, EC_FALSE, wAutoIncAddress, &bus_slave_info);
    if (ACONTIS_SUCCESS != result)
    {
      ELOG(em_handle, "Master[%d] open slave by id: slave information %s (%04x)\n", em_handle, ecatGetText(result), result);
      continue;
    }
    if ((bus_slave_info.dwVendorId == vendor_id) && (bus_slave_info.dwProductCode == product_id))
    {
      // Compare the EEPROM revision at offset 0x000C against the given revision argument
      if ( (revision != DDI_EM_DISABLE_REV_DURING_OPEN) && (bus_slave_info.dwRevisionNumber != revision))
      {
        ELOG(em_handle, "Master[%d] open slave by id: Slave found with vendor[0x%04x] product[0x%04x] but slave \
          revision[0x%x04x] did not match given rev[0x%04x]\n", em_handle, vendor_id, product_id, bus_slave_info.dwRevisionNumber, revision);
        return DDI_ES_SCAN_REV_ERR;
      }
      // Compare the EEPROM revision at offset 0x000C against the given revision argument
      if ( (serial_number != DDI_EM_DISABLE_SERIAL_DURING_OPEN) && (bus_slave_info.dwSerialNumber != serial_number))
      {
        ELOG(em_handle, "Master[%d] open slave by id: Slave found with vendor[0x%04x] product[0x%04x] but slave \
          serial[0x%x04x] did not match given serial[0x%04x]\n", em_handle, vendor_id, product_id, bus_slave_info.dwSerialNumber, serial_number);
        return DDI_ES_SCAN_SERIAL_ERR;
      }
      open_slave_handle(em_handle, &bus_slave_info, es_handle);
      return DDI_EM_STATUS_OK;
    }
  }

  ELOG(em_handle, "Master[%d] open slave by id: No slaves found with vendor[0x%04x] product[0x%04x]\n", em_handle, vendor_id, product_id);
  return DDI_EM_SCAN_NO_SLV_FOUND;
}

// Open a slave instance by station address
EM_API ddi_em_result ddi_em_open_by_station_address(ddi_em_handle em_handle, uint32_t station_address, ddi_es_handle *es_handle)
{
  uint32_t result = EC_E_ERROR;
  EC_T_BUS_SLAVE_INFO bus_slave_info;

  VALIDATE_INSTANCE(em_handle);
  DLOG(em_handle, "Master[%d] open slave by address: station address 0x%04x\
    ,number of connected slaves %d \n", em_handle, station_address, emGetNumConnectedSlaves(em_handle));
  // Retrieve the information about the slave
  result = emGetBusSlaveInfo(em_handle, EC_TRUE, station_address, &bus_slave_info);
  if (EC_E_NOERROR != result)
  {
    ELOG(em_handle, "Master[%d] open slave by address: slave information %s (%04x)\n", em_handle, ecatGetText(result), result);
    return translate_ddi_acontis_err_code(em_handle, result);
  }
  update_slave_info_from_bus_scan(em_handle, bus_slave_info.dwSlaveId, &bus_slave_info);
  // The station address matched a slave, return that ID
  open_slave_handle(em_handle, &bus_slave_info, es_handle);

  return DDI_EM_STATUS_OK;
}

// Open a slave instance by auto increment address
EM_API ddi_em_result ddi_em_open_slave_by_position(ddi_em_handle em_handle, uint32_t auto_inc_address, ddi_es_handle *es_handle)
{
  uint32_t result              = EC_E_ERROR;
  EC_T_BUS_SLAVE_INFO bus_slave_info;

  DLOG(em_handle, "Master[%d] open slave by id: number of connected slaves %d \n", em_handle, emGetNumConnectedSlaves(em_handle));
  /* get information about bus slave */
  result = emGetBusSlaveInfo(em_handle, EC_FALSE, auto_inc_address, &bus_slave_info);
  if (EC_E_NOERROR != result)
  {
    ELOG(em_handle, "Master[%d] open slave by id: slave information %s (%04x)\n", em_handle, ecatGetText(result), result);
    return translate_ddi_acontis_err_code(em_handle, result);
  }
  update_slave_info_from_bus_scan(em_handle, bus_slave_info.dwSlaveId, &bus_slave_info);
  *es_handle = bus_slave_info.dwSlaveId;
  open_slave_handle(em_handle, &bus_slave_info, es_handle);
  return DDI_EM_STATUS_OK;
}

// Get information about a slave and copy that to the cfg_info argument
EM_API ddi_em_result ddi_em_get_slave_config(ddi_em_handle em_handle, ddi_em_handle slave_handle, ddi_em_slave_config *es_cfg)
{
  EC_T_CFG_SLAVE_INFO slave_cfg;
  uint32_t result;
  VALIDATE_INSTANCE(em_handle);
  ddi_em_slave *slv_info = get_slave_instance(em_handle, slave_handle);
  if ((slv_info == NULL) || (es_cfg == NULL)) // Validate arguments
  {
    return DDI_EM_STATUS_INVALID_ARG;
  }
  // Get the slave configuration information
  result = emGetCfgSlaveInfo(em_handle, EC_TRUE, slv_info->cfg_info.station_address,&slave_cfg);
  if ( result != ACONTIS_SUCCESS)
  {
    return translate_ddi_acontis_err_code(em_handle, result); // Translate from Acontis to DDI error code
  }
  // Update the internal slave tracking information
  update_slave_info_from_config_query(em_handle, slave_handle, &slave_cfg);
  // Copy the result to the caller
  memcpy(es_cfg, &slv_info->cfg_info, sizeof(ddi_em_slave_config));
  return DDI_EM_STATUS_OK;
}

// Get the ALSTATUS (ESC register 0x130) and ALSTATUS_CODE (ESC Register 0x134) for a slave
EM_API ddi_em_result ddi_em_get_alstatus (ddi_em_handle em_handle, ddi_es_handle es_handle, uint16_t *alstatus, uint16_t *alstatus_code, uint32_t timeout)
{
  uint32_t result;
  uint16_t alstatus_reg;
  uint16_t alstatus_code_reg;
  VALIDATE_INSTANCE(em_handle);
  // Retrieve the slave information (station_address) from the slave
  ddi_em_slave *slv_info = get_slave_instance(em_handle, es_handle);
  if ((slv_info == NULL) || (alstatus == NULL) || (alstatus_code == NULL)) // Validate arguments
  {
    return DDI_EM_STATUS_INVALID_ARG;
  }

  uint32_t station_address = slv_info->cfg_info.station_address;
  // Read ALSTATUS (ESC Register 0x130)
  result = emReadSlaveRegister(em_handle, EC_TRUE,station_address, 0x130, (uint8_t*)&alstatus_reg, sizeof(alstatus_reg), timeout);
  if ( result != ACONTIS_SUCCESS )
  {
    return translate_ddi_acontis_err_code(em_handle, result); // Translate from Acontis to DDI error code
  }
  // Read ALSTATUS_CODE (ESC Register 0x134)
  result = emReadSlaveRegister(em_handle, EC_TRUE,station_address, 0x134, (uint8_t*)&alstatus_code_reg, sizeof(alstatus_reg), timeout);
  if ( result != ACONTIS_SUCCESS )
  {
    return translate_ddi_acontis_err_code(em_handle, result); // Translate from Acontis to DDI error code
  }

  // Update the ALSTATUS and ALSTATUS_CODE
  *alstatus = alstatus_reg;
  *alstatus_code = alstatus_code_reg;
  return DDI_EM_STATUS_OK;
}

// Close the slave handle (mark slave instance as unallocated)
uint32_t ddi_em_get_slave_address(ddi_em_handle em_handle, ddi_es_handle es_handle )
{
  VALIDATE_INSTANCE(em_handle);
  // Retrieve the slave information (station_address) from the slave
  ddi_em_slave *slv_info = get_slave_instance(em_handle, es_handle);
  return slv_info->cfg_info.station_address;
}

// Close the slave handle (mark slave instance as unallocated)
EM_API ddi_em_result ddi_em_close_slave (ddi_em_handle em_handle, ddi_es_handle es_handle )
{
  VALIDATE_INSTANCE(em_handle);
  // Retrieve the slave information (station_address) from the slave
  ddi_em_slave *slv_info = get_slave_instance(em_handle, es_handle);
  slv_info->allocated = 0; // Mark slave instance as unallocated
  return DDI_EM_STATUS_OK;
}

// Close all ethercat master slave handles
ddi_em_result ddi_em_close_all_slave_handles (ddi_em_handle em_handle)
{
  VALIDATE_INSTANCE(em_handle);
  ddi_em_fusion_close_all_handles(em_handle);
  return DDI_EM_STATUS_OK;
}

