/**************************************************************************
(c) Copyright 2021 Digital Dynamics Inc. Scotts Valley CA USA.
Unpublished copyright. All rights reserved. Contains proprietary and
confidential trade secrets belonging to DDI. Disclosure or release without
prior written authorization of DDI is prohibited.
**************************************************************************/

#include "ddi_em_api.h"
#include "ddi_em_link_layer.h"
#include "ddi_em_config.h"
#include "ddi_em.h"
#include "ddi_debug.h"

// adapater for the i8254 optimized link layer interface
static EC_T_LINK_PARMS_I8254X g_i8254_adapter[DDI_EM_MAX_MASTER_INSTANCES];

// register the i8254 link layer with the acontis link layer driver
static EC_PF_LLREGISTER link_layer_i8254_register(EC_T_CHAR* szDriverIdent)
{
  EC_PF_LLREGISTER pfLlRegister = EC_NULL;
  pfLlRegister = emllRegisterI8254x;
  return pfLlRegister;
}

// initialize the link layer attributes.  The parameters will be different for raw socket and the i8254 version
static EC_T_VOID link_layer_init(EC_T_LINK_PARMS* link_primary_params,
                               const uint32_t signature, const uint32_t size, const char* driver_ident,
                               const uint32_t instance, const EC_T_LINKMODE link_mode, const uint32_t priority = 0)
{
    link_primary_params->dwSignature = signature;
    link_primary_params->dwSize = size;
    strncpy(link_primary_params->szDriverIdent, driver_ident, MAX_DRIVER_IDENT_LEN - 1);
    link_primary_params->dwInstance = instance;
    link_primary_params->eLinkMode = link_mode;
    link_primary_params->dwIstPriority = priority;
}

// create the link layer for the i8254 interface
// It's a port of functionality from the selectlinklayer.cpp from the acontis reference code
ddi_em_result link_layer_i8254_init(ddi_em_handle instance, EC_T_LINK_PARMS** plink_primary_params, ddi_em_interface_select nic_interface)
{
  // Get a pointer to the link layer instance
  EC_T_LINK_PARMS_I8254X *i8254_ptr = &g_i8254_adapter[instance];
  if (EC_NULL == i8254_ptr)
  {
    return DDI_EM_STATUS_NO_RESOURCES;
  }
  link_layer_init(&i8254_ptr->linkParms, EC_LINK_PARMS_SIGNATURE_I8254X, sizeof(EC_T_LINK_PARMS_I8254X), EC_LINK_PARMS_IDENT_I8254X, 1, EcLinkMode_POLLING);

  i8254_ptr->linkParms.dwInstance = nic_interface;
  i8254_ptr->linkParms.eLinkMode = EcLinkMode_POLLING;

  // no errors
  *plink_primary_params = &i8254_ptr->linkParms;

  if (instance == 0)
  {
    // Register the optimized link layer driver
    OsReplaceGetLinkLayerRegFunc(&link_layer_i8254_register);
  }

  return DDI_EM_STATUS_OK;
}

// Determine if a network adapater has been allcoated already
ddi_em_result link_layer_adapter_allocated (ddi_em_interface_select nic_interface)
{
  int count;
  for (count = 0; count < DDI_EM_MAX_MASTER_INSTANCES; count++)
  {
    if (nic_interface == g_i8254_adapter[count].linkParms.dwInstance)
    {
      // Link layer already registered
      return DDI_EM_NIC_ALREADY_REG;
    }
  }
  return DDI_EM_STATUS_OK;
}

// Free i8254 adapter and set the pointer to null
void link_layer_deinit (ddi_em_handle instance)
{
  // Clear the structure info for this instance
  memset (&g_i8254_adapter[instance], 0, sizeof(EC_T_LINK_PARMS_I8254X));
}
