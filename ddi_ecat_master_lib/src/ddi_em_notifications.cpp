/*************************************************************************
(c) Copyright 2021 Digital Dynamics Inc. Scotts Valley CA USA.
Unpublished copyright. All rights reserved. Contains proprietary and
confidential trade secrets belonging to DDI. Disclosure or release without
prior written authorization of DDI is prohibited.
**************************************************************************/

#include "ddi_debug.h"
#include "ddi_em_config.h"
#include "ddi_em_api.h"
#include "ddi_em_link_layer.h"
#include "EcOsPlatform.h"
#include "EcLog.h"
#include "ddi_em_logging.h"
#include "ddi_em_translate.h"
#include "ddi_em.h"

// Forward declarations
static EC_T_DWORD ecatNotify(
    EC_T_DWORD          dwCode,         /**< [in]   Notification code */
    EC_T_NOTIFYPARMS*   pParms          /**< [in]   Notification data */
);

// Store the callback instance, this address is passed to the EcatNotify callback
// The local instance variable in ddi_em_register_notification cannot be used for the callback since
// The local variable will no longer exist when the actual notification callback executes
uint32_t g_event_cb_instance[DDI_EM_MAX_MASTER_INSTANCES] = { 0 };
static ddi_em_event_func *event_callbacks[DDI_EM_MAX_MASTER_INSTANCES]; // One callback per instance

// Register notifications with the Acontis notification subsystem
EM_API ddi_em_result ddi_em_set_event_handler(ddi_em_handle em_handle, ddi_em_event_func *callback)
{
  uint32_t result;
  ddi_em_instance *instance;
  EC_T_REGISTERRESULTS register_results;
  VALIDATE_INSTANCE(em_handle); // Validate the instance argument
  g_event_cb_instance[em_handle] = em_handle;
  VLOG(em_handle, "Master[%d] register notifications: entry \n", em_handle);
  // Register notifications with the Acontis core
  memset(&register_results, 0, sizeof(EC_T_REGISTERRESULTS));
  result = emRegisterClient(em_handle, ecatNotify, &g_event_cb_instance[em_handle], &register_results);
  if (result != EC_E_NOERROR)
  {
    ELOG(em_handle, "Master[%d] register notifications: Cannot register client: %s (0x%x))\n", em_handle, ecatGetText(result), result);
    return translate_ddi_acontis_err_code(em_handle, result);
  }
  // Register the callback
  event_callbacks[em_handle] = callback;
  VLOG(em_handle, "Master[%d] register notifications: done with registering client \n", em_handle);
  // Store a flag that indicates the callback is active and store off the notification id
  instance = get_master_instance(em_handle);
  instance->master_status.notification_id = register_results.dwClntId;
  instance->master_status.notification_registered = 1;
  return DDI_EM_STATUS_OK;
}

// Enable or disable the notifications for a particular event
EM_API ddi_em_result ddi_em_enable_event_handler(ddi_em_handle em_handle, ddi_em_event_type event_code, ddi_em_event_control enable_notification)
{
    uint32_t result = 0;
    uint32_t acontis_event_code;
    EC_T_IOCTLPARMS io_ctl_params;
    EC_T_SET_NOTIFICATION_ENABLED_PARMS event_params;
    ddi_em_result ddi_result;
    memset(&io_ctl_params, 0, sizeof(EC_T_IOCTLPARMS));
    memset(&event_params, 0, sizeof(EC_T_SET_NOTIFICATION_ENABLED_PARMS));

    // Translate from DDI to Acontis notification code so it can be enabled/disabled with the Acontis library
    ddi_result = translate_ddi_acontis_event_code(event_code, &acontis_event_code);
    if ( ddi_result != DDI_EM_STATUS_OK )
    {
      ELOG(em_handle, "Given notification 0x0%x did not match any supported notifications \n", event_code);
      return ddi_result;
    }
    // Setup the ioctl to modify a notification parameter
    io_ctl_params.pbyInBuf = (uint8_t *)&event_params;
    io_ctl_params.dwInBufSize = sizeof(EC_T_SET_NOTIFICATION_ENABLED_PARMS);

    // Set the notification code and notification flag
    event_params.dwCode = acontis_event_code;
    event_params.dwEnabled = enable_notification;
    // enable notification for this instance
    event_params.dwClientId = 0;
    result = emIoControl(em_handle, EC_IOCTL_SET_NOTIFICATION_ENABLED, &io_ctl_params);
    if (result != ACONTIS_SUCCESS)
    {
      return translate_ddi_acontis_err_code(em_handle, result); // Teturn the appropriate DDI error code
    }
    return DDI_EM_STATUS_OK;
}

/***************************************************************************************/
/** @brief  EtherCAT notification
*
* This function is called on EtherCAT events.
* This function parses Acontis notifications and translates them to DDI ECAT Master Calls.
*
* @return Currently always EC_E_NOERROR has to be returned.
*/
static EC_T_DWORD ecatNotify(
    EC_T_DWORD          dwCode,         /**< [in]   Notification code */
    EC_T_NOTIFYPARMS*   pParms          /**< [in]   Notification data */
                                      )
{
  ddi_em_event   event_event;
  ddi_em_handle* em_handle;
  em_handle = (ddi_em_handle *)pParms->pCallerData;
  DLOG(*em_handle, "Master[%d] handle notify entry: code 0x%x \n", *em_handle, dwCode);
  event_event.master_handle = *em_handle;

  // The following function translates from Acontis Notifications to DDI EM notifications
  if ( translate_acontis_ddi_event_code(*em_handle,0, dwCode, &event_event) == DDI_EM_STATUS_OK )
  {
    // Execute notification callback if valid
    if ( event_callbacks[*em_handle] )
    {
      (*event_callbacks[*em_handle])(&event_event);
    }
  }
  return 0; // Return success always for the acontis event handler
}
