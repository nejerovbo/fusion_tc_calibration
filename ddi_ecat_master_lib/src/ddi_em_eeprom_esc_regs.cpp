/**************************************************************************
(c) Copyright 2022 Digital Dynamics Inc. Scotts Valley CA USA.
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
#include "ddi_em_config.h"
#include "ddi_em_translate.h"
#include "ddi_em_logging.h"

uint32_t ddi_em_get_slave_address(ddi_em_handle em_handle, ddi_es_handle es_handle );

// Read the Slave eeprom
ddi_em_result ddi_em_read_eeprom(ddi_em_handle em_handle, ddi_es_handle es_handle, uint16_t offset, uint16_t *data, uint32_t read_len, uint32_t *out_len, uint32_t timeout)
{
  uint32_t result;
  uint16_t station_address;
  EC_T_BOOL is_slave_access_active;

  VALIDATE_INSTANCE(em_handle); // Validate the master instance argument

  if ( (data == NULL) || (out_len == NULL))  // Validate pointer arguments
  {
    return DDI_EM_STATUS_NULL_ARGUMENT;
  }

  // Get the station address from the slave
  station_address = ddi_em_get_slave_address(em_handle, es_handle);

  // Make sure the EtherCAT master has access to the EEPROM
  result = emActiveSlaveEEPRom(em_handle, EC_TRUE, station_address, &is_slave_access_active, timeout);
  if ( result != ACONTIS_SUCCESS )
  {
    ELOG(em_handle, "Master[%d] Slave[%d] retrieve slave state: emActiveSlaveEEPRom %s (0x%x)\n", em_handle, es_handle, ecatGetText(result), result);
    return translate_ddi_acontis_err_code(em_handle, result);
  }
  if ( is_slave_access_active )
  {
    ELOG(em_handle, "Master[%d] does not have access to the PDI EEPROM \n", em_handle);
    return DDI_ES_EEPROM_BUSY;
  }

  // Read the slave eeprom
  result = emReadSlaveEEPRom(em_handle, EC_TRUE, station_address, offset, data, read_len, out_len, timeout);
  if ( result != ACONTIS_SUCCESS )
  {
    ELOG(em_handle, "Master[%d] Slave[%d] retrieve slave state: emReadSlaveEEPRom %s (0x%x)\n", em_handle, es_handle, ecatGetText(result), result);
    return translate_ddi_acontis_err_code(em_handle, result);
  }

  return DDI_EM_STATUS_OK;
}

// Write the Slave eeprom
ddi_em_result ddi_em_write_eeprom(ddi_em_handle em_handle, ddi_es_handle es_handle, uint16_t offset, uint16_t *data, uint32_t write_len, uint32_t timeout)
{
  uint32_t result;
  uint16_t station_address;
  EC_T_BOOL is_slave_access_active;

  VALIDATE_INSTANCE(em_handle); // Validate the master instance argument

  if ( (data == NULL))  // Validate pointer arguments
  {
    return DDI_EM_STATUS_NULL_ARGUMENT;
  }

  // Get the station address from the slave
  station_address = ddi_em_get_slave_address(em_handle, es_handle);

  // Make sure the EtherCAT master has access to the EEPROM
  result = emActiveSlaveEEPRom(em_handle, EC_TRUE, station_address, &is_slave_access_active, timeout);
  if ( result != ACONTIS_SUCCESS )
  {
    ELOG(em_handle, "Master[%d] Slave[%d] retrieve slave state: emActiveSlaveEEPRom %s (0x%x)\n", em_handle, es_handle, ecatGetText(result), result);
    return translate_ddi_acontis_err_code(em_handle, result);
  }
  if ( is_slave_access_active )
  {
    ELOG(em_handle, "Master[%d] does not have access to the PDI EEPROM \n", em_handle);
    return DDI_ES_EEPROM_BUSY;
  }

  // Write the slave eeprom
  result = emWriteSlaveEEPRom(em_handle, EC_TRUE, station_address, offset, data, write_len, timeout);
  if ( result != ACONTIS_SUCCESS )
  {
    ELOG(em_handle, "Master[%d] Slave[%d] retrieve slave state: emReadSlaveEEPRom %s (0x%x)\n", em_handle, es_handle, ecatGetText(result), result);
    return translate_ddi_acontis_err_code(em_handle, result);
  }

  return DDI_EM_STATUS_OK;
}

// Write the ESC registers
ddi_em_result ddi_em_write_esc_reg(ddi_em_handle em_handle, ddi_es_handle es_handle, uint16_t offset, uint8_t *data, uint32_t write_len, uint32_t timeout)
{
  uint32_t result;
  uint16_t station_address;

  VALIDATE_INSTANCE(em_handle); // Validate the master instance argument

  if ( (data == NULL))  // Validate pointer arguments
  {
    return DDI_EM_STATUS_NULL_ARGUMENT;
  }

  // Get the station address from the slave
  station_address = ddi_em_get_slave_address(em_handle, es_handle);

  // Write the slave eeprom - use the station address alias to access the slave
  result = emWriteSlaveRegister(em_handle, EC_TRUE, station_address, offset, data, write_len, timeout);
  if ( result != ACONTIS_SUCCESS )
  {
    ELOG(em_handle, "Master[%d] Slave[%d] retrieve slave state: emReadSlaveEEPRom %s (0x%x)\n", em_handle, es_handle, ecatGetText(result), result);
    return translate_ddi_acontis_err_code(em_handle, result);
  }

  return DDI_EM_STATUS_OK;
}

// Read the ESC registers
ddi_em_result ddi_em_read_esc_reg(ddi_em_handle em_handle, ddi_es_handle es_handle, uint16_t offset, uint8_t *data, uint32_t read_len, uint32_t timeout)
{
  uint32_t result;
  uint16_t station_address;

  VALIDATE_INSTANCE(em_handle); // Validate the master instance argument

  if ( (data == NULL))  // Validate pointer arguments
  {
    return DDI_EM_STATUS_NULL_ARGUMENT;
  }

  // Get the station address from the slave
  station_address = ddi_em_get_slave_address(em_handle, es_handle);

  // Read the slave eeprom - use the station address alias to access the slave
  result = emReadSlaveRegister(em_handle, EC_TRUE, station_address, offset, data, read_len, timeout);
  if ( result != ACONTIS_SUCCESS )
  {
    ELOG(em_handle, "Master[%d] Slave[%d] retrieve slave state: emReadSlaveEEPRom %s (0x%x)\n", em_handle, es_handle, ecatGetText(result), result);
    return translate_ddi_acontis_err_code(em_handle, result);
  }

  return DDI_EM_STATUS_OK;
}
