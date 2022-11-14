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
#include "ddi_em_config.h"
#include "ddi_em_logging.h"
#include "ddi_em_translate.h"

// Support COE read functionality
EM_API ddi_em_result ddi_em_coe_read(ddi_em_handle em_handle, ddi_es_handle es_handle, uint16_t index, uint16_t subindex, uint8_t *data,
 uint32_t len, uint32_t *out_len, uint32_t timeout, uint32_t flags )
{
  uint32_t result;
  VALIDATE_INSTANCE(em_handle); // Validate the instance argument
  result = emCoeSdoUpload(em_handle, es_handle, index, subindex, data, len, out_len, timeout, flags);
  if ( result != ACONTIS_SUCCESS )
  {
    ELOG(em_handle, "Master[%d] Slave[%d] CAN over EtherCAT Read: %s (0x%x)\n", em_handle, es_handle, ecatGetText(result), result);
    return translate_ddi_acontis_err_code(em_handle, result); // Return the Acontis->DDI translated error code
  }
  return DDI_EM_STATUS_OK;
}

// Support COE write functionality
EM_API ddi_em_result ddi_em_coe_write(ddi_em_handle em_handle, ddi_es_handle es_handle, uint16_t index, uint16_t subindex, uint8_t *data,
 uint32_t len, uint32_t timeout, uint32_t flags )
{
  uint32_t result;
  VALIDATE_INSTANCE(em_handle); // Validate the instance argument
  result = emCoeSdoDownload(em_handle, es_handle, index, subindex, data, len, timeout, flags);
  if ( result != ACONTIS_SUCCESS )
  {
    ELOG(em_handle, "Master[%d] Slave[%d] CAN over EtherCAT Write: %s (0x%x)\n", em_handle, es_handle, ecatGetText(result), result);
    return translate_ddi_acontis_err_code(em_handle, result); // Return the Acontis->DDI translated error code
  }
  return DDI_EM_STATUS_OK;
}
