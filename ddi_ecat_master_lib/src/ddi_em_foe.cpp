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

// Support FoE write operations
EM_API ddi_em_result ddi_em_foe_write(ddi_em_handle em_handle, ddi_es_handle es_handle, const char *filename, uint32_t file_length,
  uint8_t *data, uint32_t data_length, uint32_t password, uint32_t timeout )
{
   uint32_t result;
   VALIDATE_INSTANCE(em_handle); // Validate the instance argument
   result = emFoeFileDownload(em_handle, es_handle, (char *)filename, file_length, data, data_length, password, timeout);
   if ( result != ACONTIS_SUCCESS )
   {
     ELOG(em_handle,"Master[%d] Slave[%d] File over EtherCAT Download: %s (0x%x)\n", em_handle, es_handle, ecatGetText(result), result);
     return translate_ddi_acontis_err_code(em_handle, result); // Return the Acontis->DDI translated error code
   }
   return DDI_EM_STATUS_OK;
}

// Support FoE read operations
EM_API ddi_em_result ddi_em_foe_read(ddi_em_handle em_handle, ddi_es_handle es_handle, const char *filename, uint32_t file_length,
  uint8_t *data, uint32_t data_length, uint32_t *read_len, uint32_t password, uint32_t timeout )
{
   uint32_t result;
   VALIDATE_INSTANCE(em_handle); // Validate the instance argument
   result = emFoeFileUpload(em_handle, es_handle, (char *)filename, file_length, data, data_length, read_len, password, timeout);
   if ( result != ACONTIS_SUCCESS )
   {
     ELOG(em_handle, "Master[%d] Slave[%d] File over EtherCAT Upload: %s (0x%x)\n", em_handle, es_handle, ecatGetText(result), result);
     return translate_ddi_acontis_err_code(em_handle, result); // Return the Acontis->DDI translated error code
   }
   return DDI_EM_STATUS_OK;
}
