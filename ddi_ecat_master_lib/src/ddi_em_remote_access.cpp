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
#include <AtEmRasSrv.h>
#include "ddi_debug.h"
#include "ddi_em_api.h"
#include "ddi_em_config.h"
#include "ddi_em.h"
#include "ddi_em_translate.h"
#include "ddi_em_remote_access.h"

/*
 * This file provides independent remote access functionality.  Each DDI EtherCAT Master instance
 * can have it's own remote access server, or if the master instances are in the same process,
 * the EtherCAT Master instances can share the same remote access server.
 *                           Remote Access Server
 *                                  |
 *        ________________________________________________________
 *        |                   |                  |                |
 *   ECAT Master 0       ECAT Master 1      ECAT Master 2    ECAT Master 3
 */
ddi_em_remote_access_instance g_remote_instance[DDI_EM_MAX_REMOTE_INSTANCES];

// Clear the remote instance access on startup
void ddi_em_remote_access_clear_instance_data (void)
{
  memset(g_remote_instance, 0, sizeof(g_remote_instance));
  return;
}

// Get the next available remote instance
static ddi_em_result get_next_remote_instance (ddi_em_handle *handle)
{
  int count;
  for ( count = 0; count < DDI_EM_MAX_REMOTE_INSTANCES; count++)
  {
    if ( g_remote_instance[count].is_allocated == 0 )
    {
      // Clear the instance state and configuration data
      memset(&g_remote_instance[count], 0, sizeof(ddi_em_remote_access_instance));
      *handle = count;
      g_remote_instance[count].is_allocated = 1;
      return DDI_EM_STATUS_OK;
    }
  }
  return DDI_EM_STATUS_NO_RESOURCES;
}

// Initialize the remote access server
EM_API ddi_em_result ddi_em_remote_access_init (ddi_em_remote_access_init_params *init_params, ddi_ra_handle *remote_handle)
{
  ATEMRAS_T_SRVPARMS remote_config;
  ddi_em_result em_result;
  uint32_t ec_result;
  EC_T_CPUSET cpu_set;

  // Get the next available master access handle
  em_result=get_next_remote_instance(remote_handle);
  if ( em_result != DDI_EM_STATUS_OK ) // Validate instance return code
  {
    printf(RED "Remote access initialization failed \n");
    return em_result;
  }

  // Clear the remote configuration structure
  memset(&remote_config, 0, sizeof(ATEMRAS_T_SRVPARMS));
  remote_config.dwSignature        = ATEMRASSRV_SIGNATURE;
  remote_config.dwSize             = sizeof(ATEMRAS_T_SRVPARMS);
  remote_config.oAddr.dwAddr       = 0; // INADDR_ANY
  if (init_params->remote_port != 0 )
  {
    remote_config.wPort            = init_params->remote_port;
  }
  else
  {
    remote_config.wPort             = REMOTE_ACCESS_PORT_DEFAULT;
  }
  remote_config.dwCycleTime         = REMOTE_ACCESS_DEFAULT_CYCLE; // Set polling rate to Acontis-suggested default 2000 us
  remote_config.dwWDTOLimit         = remote_config.dwCycleTime * 1000; // Timeout after 1000 cycles
  // If CPU affinity enabled, create the listener thread on this CPU
  if ( init_params->remote_enable_cpu_affinity )
  {
    // Zero the CPU set, then set the CPU affinity
    EC_CPUSET_ZERO(cpu_set);
    EC_CPUSET_SET(cpu_set, init_params->remote_cpu_select);
  }
  else
  {
    // Enable any CPU by clearing the CPU set
    EC_CPUSET_ZERO(cpu_set);
  }
  // These are the Acontis defaults for the remote access server
  // For the initial release of remote access, these are sufficient to enable EC-Engineer access
  // Future releases may allow the operator to tweak these values further
  remote_config.cpuAffinityMask     = cpu_set;                      // Set CPU affinity
  remote_config.dwMasterPrio        = DDI_EM_CYCLIC_THREAD_PRI_LOW; // Low priority for remote access
  remote_config.dwClientPrio        = DDI_EM_CYCLIC_THREAD_PRI_LOW; // Low priority for remote access
  remote_config.pvNotifCtxt         = NULL;                         // Notification context
  remote_config.pfNotification      = NULL;                         // Notification function for emras Layer
  remote_config.dwConcNotifyAmount  = 100;                          // for the first pre-allocate 100 Notification spaces
  remote_config.dwMbxNotifyAmount   = 50;                           // for the first pre-allocate 50 Notification spaces
  remote_config.dwMbxUsrNotifySize  = 3000;                         // 3K user space for Mailbox Notifications
  remote_config.dwCycErrInterval    = 500;                          // span between to consecutive cyclic notifications of same type
  remote_config.LogParms.dwLogLevel = EC_LOG_LEVEL_ERROR;
  ec_result = emRasSrvStart(&remote_config, &g_remote_instance[*remote_handle].remote_access_handle);
  if (EC_E_NOERROR != ec_result)
  {
    ELOG("Error during initialization of Remote Access: %s \n", ecatGetText(ec_result));
    return DDI_EM_REMOTE_ACCESS_INIT_ERR;
  }
  g_remote_instance[*remote_handle].remote_access_enabled = DDI_EM_TRUE;
  return DDI_EM_STATUS_OK;
}

// De-initialize the remote access server
ddi_em_result ddi_em_remote_access_deinit (ddi_ra_handle remote_handle)
{
  VALIDATE_INSTANCE(remote_handle);
  ddi_em_result result = DDI_EM_STATUS_OK; // Return success by default
  uint32_t ec_result;
  uint32_t timeout = REMOTE_ACCESS_SHUTDOWN_TIMEOUT_MS;
  // If the remote access server is active, then shut it down
  if ( g_remote_instance[remote_handle].remote_access_enabled )
  {
    ec_result = emRasSrvStop(g_remote_instance[remote_handle].remote_access_handle, timeout);
    if ( ACONTIS_SUCCESS != ec_result )
    {
      ELOG("Error during deinitialzation of Remote Access: %s \n", ecatGetText(ec_result));
      return DDI_EM_REMOTE_ACCESS_DEINIT_ERR;
    }
  }
  g_remote_instance[remote_handle].is_allocated = DDI_EM_FALSE;
  return result;
}
