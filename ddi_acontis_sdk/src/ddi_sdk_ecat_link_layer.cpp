/******************************************************************************
 * (c) Copyright 2020-2021 Digital Dynamics Inc. Scotts Valley CA USA.
 * Unpublished copyright. All rights reserved. Contains proprietary and
 * confidential trade secrets belonging to DDI. Disclosure or release without
 * prior written authorization of DDI is prohibited.
 *****************************************************************************/

#include "ddi_sdk_ecat_link_layer.h"

/* adapter for the raw socket interface */
EC_T_LINK_PARMS_SOCKRAW* adapter = EC_NULL;
/* adapater for the i8254 optimized link layer interface */
EC_T_LINK_PARMS_I8254X* i8254_adapter = EC_NULL;

//initialize the link layer attributes.  The parameters will be different for raw socket and the i8254 version
static EC_T_VOID link_layer_init(EC_T_LINK_PARMS* link_primary_params,
                               const uint32_t signature, const uint32_t size, const char* driver_ident,
                               const uint32_t instance, const EC_T_LINKMODE link_mode, const uint32_t priority = 0)
{
    memset(link_primary_params, 0, sizeof(EC_T_LINK_PARMS));
    link_primary_params->dwSignature = signature;
    link_primary_params->dwSize = size;
    strncpy(link_primary_params->szDriverIdent, driver_ident, MAX_DRIVER_IDENT_LEN - 1);
    link_primary_params->dwInstance = instance;
    link_primary_params->eLinkMode = link_mode;
    link_primary_params->dwIstPriority = priority;
}

// register the i8254 link layer with the acontis link layer driver
static EC_PF_LLREGISTER i8254_link_layer_register(EC_T_CHAR* szDriverIdent)
{
  EC_PF_LLREGISTER pfLlRegister = EC_NULL;
  pfLlRegister = emllRegisterI8254x;
  return pfLlRegister;
}

//create the link layer for the i8254 interface
//It's a port of functionality from the selectlinklayer.cpp from the acontis reference code
uint32_t create_link_params_i8254(EC_T_LINK_PARMS** plink_primary_params, uint32_t instance)
{
  uint32_t ret_val = EC_E_ERROR;

  /* alloc adapter specific link parms */
  i8254_adapter = (EC_T_LINK_PARMS_I8254X*)malloc(sizeof(EC_T_LINK_PARMS_I8254X));
  if (EC_NULL == i8254_adapter)
  {
      return ddi_status_err;
      ret_val = EC_E_NOMEMORY;
  }
  memset(i8254_adapter, 0, sizeof(EC_T_LINK_PARMS_I8254X));
  link_layer_init(&i8254_adapter->linkParms, EC_LINK_PARMS_SIGNATURE_I8254X, sizeof(EC_T_LINK_PARMS_I8254X), EC_LINK_PARMS_IDENT_I8254X, 1, EcLinkMode_POLLING);

  ret_val = EC_E_NOERROR;

  i8254_adapter->linkParms.dwInstance = instance;
  i8254_adapter->linkParms.eLinkMode = EcLinkMode_POLLING;

  /* no errors */
  *plink_primary_params = &i8254_adapter->linkParms;

  OsReplaceGetLinkLayerRegFunc(&i8254_link_layer_register);

  return ret_val;
}

//This function creates the parameters for the sockraw interface
//It's a port of functionality from the selectlinklayer.cpp from the acontis reference code 
uint32_t create_link_params_sockraw(EC_T_LINK_PARMS** plink_primary_params, const char *iface_name)
{
   char *ptcWord;
   uint32_t ret_val = EC_E_ERROR;

    /* alloc adapter specific link parms */
    adapter = (EC_T_LINK_PARMS_SOCKRAW*)malloc(sizeof(EC_T_LINK_PARMS_SOCKRAW));
    if (EC_NULL == adapter)
    {
        ret_val = EC_E_NOMEMORY;
        return ddi_status_err;
    }
    memset(adapter, 0, sizeof(EC_T_LINK_PARMS_SOCKRAW));
    link_layer_init(&adapter->linkParms, EC_LINK_PARMS_SIGNATURE_SOCKRAW, sizeof(EC_T_LINK_PARMS_SOCKRAW), EC_LINK_PARMS_IDENT_SOCKRAW, 1, EcLinkMode_POLLING);

    /* get adapter name */
    ptcWord = (char*)iface_name;
    strncpy(adapter->szAdapterName, (char*)ptcWord, MAX_LEN_SOCKRAW_ADAPTER_NAME - 1);
    //strncpy(link_primary_paramsAdapter->szAdapterName, (char*)ptcWord, 7);

#if (defined DISABLE_FORCE_BROADCAST)
    /* Do not overwrite destination in frame with FF:FF:FF:FF:FF:FF, needed for EAP. */
    ladapter->bDisableForceBroadcast = EC_TRUE;
#endif

    /* no errors */
    *plink_primary_params = &adapter->linkParms;
    ret_val = EC_E_NOERROR;

    return ret_val;
}