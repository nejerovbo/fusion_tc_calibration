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
#include "ddi_em_translate.h"
#include "ddi_em_logging.h"

// Get the EtherCAT Master state for a particular EtherCAT instance
ddi_em_result ddi_em_get_master_state(ddi_em_handle em_handle, ddi_em_state *master_state)
{
  EC_T_STATE ec_master_state;
  ddi_em_state em_master_state;
  VALIDATE_INSTANCE(em_handle); // Validate the instance argument
  if ( master_state == NULL )
  {
    return DDI_EM_STATUS_NULL_ARGUMENT;
  }
  ec_master_state = emGetMasterState(em_handle);
  // Translate from Acontis to DDI master states
  switch (ec_master_state)
  {
    case eEcatState_INIT:
      em_master_state = DDI_EM_STATE_INIT;
      break;
    case eEcatState_PREOP:
      em_master_state = DDI_EM_STATE_PREOP;
      break;
    case eEcatState_SAFEOP:
      em_master_state = DDI_EM_STATE_SAFEOP;
      break;
    case eEcatState_OP:
      em_master_state = DDI_EM_STATE_OP;
      break;
    default:
      em_master_state = DDI_EM_INVALID_STATE;
      ELOG(em_handle, "Master[%d] state change: ddi_em_get_master_state_inst, invalid master state: %d \n", em_handle, *master_state);
      return DDI_EM_STATE_ERR;
  }
  *master_state = em_master_state; // Pass back the master state
  return DDI_EM_STATUS_OK;
}

// Get the EtherCAT Slave state
ddi_em_result ddi_em_get_slave_state(ddi_em_handle em_handle, ddi_es_handle slave_instance, ddi_es_state *slave_state)
{
  uint32_t result;
  EC_T_WORD current_state, requested_state;
  ddi_es_state em_slave_state;
  VALIDATE_INSTANCE(em_handle); // Validate the instance argument
  if ( slave_state == NULL )
  {
    return DDI_EM_STATUS_NULL_ARGUMENT;
  }
  result = emGetSlaveState(em_handle, slave_instance, &current_state, &requested_state);
  if ( result != ACONTIS_SUCCESS )
  {
    ELOG(em_handle, "Master[%d] Slave[%d] retrieve slave state: emGetMasterState %s (0x%x)\n", em_handle, slave_instance, ecatGetText(result), result);
    em_slave_state = DDI_ES_INVALID_STATE;
    return translate_ddi_acontis_err_code(em_handle, result);
  }
  // Translate from Acontis to DDI slave states
  switch (current_state)
  {
    case eEcatState_INIT:
      em_slave_state = DDI_ES_STATE_INIT;
      break;
    case eEcatState_BOOTSTRAP:
      em_slave_state = DDI_ES_STATE_BOOT;
      break;
    case eEcatState_PREOP:
      em_slave_state = DDI_ES_STATE_PREOP;
      break;
    case eEcatState_SAFEOP:
      em_slave_state = DDI_ES_STATE_SAFEOP;
      break;
    case eEcatState_OP:
      em_slave_state = DDI_ES_STATE_OP;
      break;
    default:
      em_slave_state = DDI_ES_INVALID_STATE;
      ELOG(em_handle, "Master[%d] Slave[%d] slave state retrieve: invalid master state: %d \n", em_handle, slave_instance, current_state);
      return DDI_ES_STATE_ERR;
  }
  *slave_state = em_slave_state; // Pass back the slave state
  return DDI_EM_STATUS_OK;
}

// Set the EtherCAT Slave state
ddi_em_result ddi_em_set_slave_state(ddi_em_handle em_handle, ddi_es_handle slave_instance, ddi_es_state slave_state, uint32_t timeout)
{
  uint32_t result;
  EC_T_STATE state;
  ddi_em_result em_result = DDI_EM_STATUS_OK;
  // Translate from DDI states to Acontis states
  switch (slave_state)
  {
    case DDI_ES_STATE_INIT:
      state = eEcatState_INIT;
      break;
    case DDI_ES_STATE_BOOT:
      state = eEcatState_BOOTSTRAP;
      break;
    case DDI_ES_STATE_PREOP:
      state = eEcatState_PREOP;
      break;
    case DDI_ES_STATE_SAFEOP:
      state = eEcatState_SAFEOP;
      break;
    case DDI_ES_STATE_OP:
      state = eEcatState_OP;
      break;
    default:
      em_result = DDI_ES_SET_STATE_ERR;
      ELOG(em_handle, "Master[%d] Slave[%d] set slave state: invalid slave state: %d \n", em_handle, slave_instance, slave_state);
      return em_result;
  }
  // Transition the slave state
  result = emSetSlaveState(em_handle, slave_instance, state, timeout);
  if ( result != ACONTIS_SUCCESS )
  {
    ELOG(em_handle, "Master[%d] Slave[%d] set slave state: %s (0x%x)\n", em_handle, slave_instance, ecatGetText(result), result);
    em_result = translate_ddi_acontis_err_code(em_handle, result); // Return the Acontis->DDI translated error code
  }
  return em_result;
}

// Set the EtherCAT Master state
ddi_em_result ddi_em_set_master_state(ddi_em_handle em_handle, ddi_em_state master_state, uint32_t timeout)
{
  uint32_t result;
  EC_T_STATE state;
  ddi_em_result em_result = DDI_EM_STATUS_OK;
  // Translate from DDI states to Acontis states
  switch (master_state)
  {
    case DDI_EM_STATE_INIT:
      state = eEcatState_INIT;
      break;
    case DDI_EM_STATE_PREOP:
      state = eEcatState_PREOP;
      break;
    case DDI_EM_STATE_SAFEOP:
      state = eEcatState_SAFEOP;
      break;
    case DDI_EM_STATE_OP:
      state = eEcatState_OP;
      break;
    default:
      em_result = DDI_EM_STATUS_INVALID_ARG;
      ELOG(em_handle, "Master[%d] set master state: invalid master state: 0x%04x \n", em_handle, master_state);
      return DDI_EM_STATUS_INVALID_ARG;
  }
  // Transition the master state
  result = emSetMasterState(em_handle, timeout, state);
  if ( result != ACONTIS_SUCCESS )
  {
    ELOG(em_handle, "Master[%d] set master state: %s (0x%x)\n", em_handle, ecatGetText(result), result);
    em_result = DDI_ES_STATE_ERR;
  }
  return em_result;
}
