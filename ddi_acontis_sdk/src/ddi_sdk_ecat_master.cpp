/******************************************************************************
 * (c) Copyright 2020-2021 Digital Dynamics Inc. Scotts Valley CA USA.
 * Unpublished copyright. All rights reserved. Contains proprietary and
 * confidential trade secrets belonging to DDI. Disclosure or release without
 * prior written authorization of DDI is prohibited.
 *****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "ddi_sdk_common.h"
#include "ddi_sdk_display.h"
#include <time.h>
#include <signal.h>
#include <AtEthercat.h>
#include "ddi_macros.h"
#include "ddi_status.h"
#include "ddi_sdk_processing.h"
#include "ddi_os.h"
#include "ddi_debug.h"
#include "ddi_sdk_common.h"
#include "ddi_sdk_ecat_master.h"
#include "ddi_sdk_ecat_link_layer.h"
#include "ddi_sdk_ecat_sdo.h"
#include "ddi_defines.h"
#include "ddi_ntime.h"

#define MASTER_CFG_MAX_ACYC_FRAMES_QUEUED     32    /* max number of acyc frames queued, 127 = the absolute maximum number */
#define MASTER_CFG_MAX_ACYC_BYTES_PER_CYC   4096    /* max number of bytes sent during eUsrJob_SendAcycFrames within one cycle */
#define MASTER_CFG_MAX_ACYC_CMD_RETRIES        3    /* max retry count for acyclic frames */
#define DDI_MAX_ECAT_INSTANCES                16    /* max number of ethercat slaves supported */
#define ETHERCAT_STATE_CHANGE_TIMEOUT         15000 /* master state change timeout in ms */
#define LOST_FRAME_COUNT_MAX                  15    /* maximum lost frames in a row before program exits */

EC_T_LINK_PARMS             *param_ptr;             /*< [in] Link layer parameters, used for optimized and raw socket connections */
static EC_T_REGISTERRESULTS S_oRegisterResults;     /* used for event registration */

ddi_ecat_master_stats_t     ecat_master_stats;      /* keeps track of master statistics */
ddi_ecat_master_cfg_t       ecat_master_cfg;        /* keeps track of the master configuraiton */

slave_t slave[DDI_SDK_MAX_INSTANCES];               /* slave instance instantiation */

uint8_t license_status = 0;                         /* license status, 0 = invalid, 1 = valid */
char    *license_filename = NULL;                   /* filename of the acontis license */

uint32_t client_id;                                 /* ecat client id, used by the COE object dictionary download */

/* forward declarations */
void * ddi_cyclic_thread(const void *arg);

//set license filename, should be called before ddi_sdk_init or ddi_sdk_ecat_init funcitons
ddi_status_t ddi_sdk_ecat_set_license_file(char *filename)
{
  license_filename = filename;
  return ddi_status_ok;
}

ddi_status_t ddi_sdk_ecat_get_mac(char *mac_addr_string)
{
  uint32_t result;
  ETHERNET_ADDRESS ecat_mac_addr;
  int count;

  //check argument validity
  if(!mac_addr_string)
    return ddi_status_param_err;

  result = ecatGetSrcMacAddress(&ecat_mac_addr);
  if (result != EC_E_NOERROR)
  {
    ELOG( "Cannot get MAC address: %s (0x%x))\n", ecatGetText(result), result);
    return ddi_status_err;
  }
  VLOG( "EtherCAT network adapter MAC: %02X-%02X-%02X-%02X-%02X-%02X\n",
      ecat_mac_addr.b[0], ecat_mac_addr.b[1], ecat_mac_addr.b[2], ecat_mac_addr.b[3], ecat_mac_addr.b[4], ecat_mac_addr.b[5]);

  //create a mac address ss
  sprintf(mac_addr_string, "%02X-%02X-%02X-%02X-%02X-%02X\n",
      ecat_mac_addr.b[0], ecat_mac_addr.b[1], ecat_mac_addr.b[2], ecat_mac_addr.b[3], ecat_mac_addr.b[4], ecat_mac_addr.b[5]);

  return ddi_status_ok;
}

ddi_status_t ddi_sdk_ecat_set_master_license ( const char *filename )
{
  FILE *fp;
  char buffer[256];
  char adapter[128];
  char *tmp;
  char *MAC;
  char *license;
  ddi_status_t result;
  fp = fopen(filename, "r+");
  if (!fp)
  {
    ELOG(" invalid filename %s \n", filename);
    return ddi_status_param_err;
  }
  //get the adapter mac address
  result = ddi_sdk_ecat_get_mac(adapter);
  if ( result != ddi_status_ok )
  {
    ELOG(" issue getting adapter mac address\n");
    fclose(fp);
    return result;
  }

  //loop until end of file is reached, fgets returns null when EOF is reached.
  while (fgets (buffer , sizeof(buffer) , fp) != NULL )
  {
    tmp = strchr(buffer,',');
    if(tmp == NULL)
      continue;
    //the MAC is after the second semicolon
    MAC = strchr(++tmp,',');
    //the MAC is after the third semicolon
    license = strchr(++MAC,',');
    if( !strncmp (adapter, MAC, 17))
    {
      DLOG(" licensed adapter found at %s \n", adapter);
      license_status = 1;
      //need to remove the last semicolon from the license line
      license[27]='\0';
      //remove the first semicolon
      license++;
      ecatSetLicenseKey(license);
      fclose(fp);
      return ddi_status_ok;
    }
  }

  DLOG(" ddi_sdk_ecat_set_master_license done \n");
  fclose(fp);
  return ddi_status_err;
}

uint8_t ddi_sdk_ecat_get_master_license_status ( void )
{
  return license_status;
}

/***************************************************************************************************/
/**
\brief  Find a specific slave and return its fixed (ethercat) address
\return EC_TRUE on success, EC_FALSE otherwise.
*/
ddi_status_t ddi_sdk_get_slave_fixed_addr(
     slave_t         *slave,
     uint32_t             vendor_id,
     uint32_t             product_id,
     uint16_t*           pwPhysAddr)         /**< [out]  Physical Address of slave */
{
    uint32_t result              = EC_E_ERROR;
    uint32_t slave_index         = 0;
    uint32_t slave_index_count   = 0;

    for (slave_index = 0; slave_index < emGetNumConnectedSlaves(slave->master_instance_id); slave_index++)
    {
      uint16_t           wAutoIncAddress = (uint16_t)(0-slave_index);
      EC_T_BUS_SLAVE_INFO bus_slave_info;

      /* get information about bus slave */
      result = emGetBusSlaveInfo(slave->master_instance_id, EC_FALSE, wAutoIncAddress, &bus_slave_info);
      if (EC_E_NOERROR != result)
      {
          ELOG( "PrintSlaveInfos() emGetBusSlaveInfo return with error 0x%x\n", result);
          continue;
      }
      if ((bus_slave_info.dwVendorId == vendor_id) && (bus_slave_info.dwProductCode == product_id))
      {
        if (slave_index_count == 0)
        {
            /* slave found */
            *pwPhysAddr = bus_slave_info.wStationAddress;
            return EC_TRUE;
        }
        slave_index_count++;
      }
    }
    ELOG(" No fusion.IO slaves found emGetNumConnectedSlaves(slave->master_instance_id) = %d\n", emGetNumConnectedSlaves(slave->master_instance_id));
    return EC_FALSE;
}

//this gets called every time there is a noticiation on Acontis. 
//this is currently not used.
static uint32_t ecatNotifyCallback(
    uint32_t         dwCode,  /**< [in]   Notification code */
    EC_T_NOTIFYPARMS*  pParms   /**< [in]   Notification parameters */
                                         )
{
   //DLOG(" ecatnotify\n");
   return 0;
}

//------------ Configure the EtherCAT master stack -------------
ddi_status_t  ddi_sdk_ecat_configure_master( const char *eni_filename)
{
  uint32_t result;
  result = ecatConfigureMaster(eCnfType_Filename, (unsigned char*)eni_filename, 256);
  //result = ecatConfigureMaster(eCnfType_GenOpENI, NULL, 0);
  if (result != EC_E_NOERROR)
  {
    ELOG( "Cannot configure EtherCAT-Master: %s (0x%x))\n", ecatGetText(result), result);
    if ( result == 0x98110011 )
    {
      ELOG(" The libemllSockRaw.so or other socket library is likely not present in the current directory \n");
    }
    if ( result == 0x98110070 )
    {
      ELOG(" Thie file you requested is likely not present.  You entered the following eni: %s \n", eni_filename );
    }
    return ddi_status_err;
  }
  return ddi_status_ok;
}

//initializes the ethercat stack
ddi_status_t ddi_sdk_ecat_init(uint32_t bus_cycle_us, const char *eni_filename, const char *iface_name)
{
  EC_T_INIT_MASTER_PARMS     init_params;
  ETHERNET_ADDRESS           mac_address;
  uint32_t                   result;
  ddi_thread_handle_t        ddi_thread_handle;
  const char                 *instance;
  uint32_t                   instance_value;
  
  memset(&init_params, 0, sizeof(EC_T_INIT_MASTER_PARMS));
  memset(&slave,0, sizeof(slave_t));

  DLOG(" *** SDK init arguments: ddi_sdk_ecat_init scan %d eni %s iface %s \n", bus_cycle_us, eni_filename, iface_name);

  //set the bus scan reate
  ecat_master_cfg.master_scan_rate_us = bus_cycle_us;

  // No longer any calls to set CPU affinity 

  //determine if there is a optimized link layer argument
  if(!strncmp(iface_name,"i8254",5))
  {
    //extract the instance after the : argument
    instance=strchr(iface_name,':');
    //convert from character to numeric
    instance_value = instance[1] - 0x30;
    create_link_params_i8254(&param_ptr, instance_value);
  }
  else
  {
    create_link_params_sockraw(&param_ptr, iface_name);
  }

  //------------- Setup the EtherCAT master-------------------------
  init_params.dwSignature                   = ATECAT_SIGNATURE;
  init_params.dwSize                        = sizeof(EC_T_INIT_MASTER_PARMS);
  init_params.pLinkParms                    = param_ptr;
  init_params.pLinkParmsRed                 = EC_NULL; //redundancy params
  init_params.pOsParms                      = EC_NULL; //OS parameters
  init_params.dwBusCycleTimeUsec            = bus_cycle_us;
  init_params.dwMaxBusSlaves                = 256;
  init_params.dwMaxAcycFramesQueued         = MASTER_CFG_MAX_ACYC_FRAMES_QUEUED;
  if (init_params.dwBusCycleTimeUsec >= 1000)
  {
      init_params.dwMaxAcycBytesPerCycle    = MASTER_CFG_MAX_ACYC_BYTES_PER_CYC;
  }
  else
  {
      init_params.dwMaxAcycBytesPerCycle    = 1500;
      init_params.dwMaxAcycFramesPerCycle   = 1;
      init_params.dwMaxAcycCmdsPerCycle     = 20;
  }
  init_params.dwEcatCmdMaxRetries           = MASTER_CFG_MAX_ACYC_CMD_RETRIES;
  //------------ Inititalize the EtherCAT master stack -------------
  result = ecatInitMaster(&init_params);
  if (result != EC_E_NOERROR)
  {
      ELOG ( "Cannot initialize EtherCAT-Master: %s (0x%x))\n", ecatGetText(result), result);
      if ( result == 0x98110000 )
      {
        ELOG(" You likely need to run this program as sudo or the network interface is incorrect \n");
      }
      if ( result == 0x9811000b )
      {
        ELOG( "This is likely due to missing the network interface driver (i.e. libemllSockRaw.so). \n \
               The .so file needs to be in the LD_LIBRARY_PATH or your local directory \n");
      }
      return ddi_status_err;
  }
  DLOG("init master result = %d \n",result);

  //try setting the license if the parameter has been given
  if ( license_filename )
  {
    if ( ddi_sdk_ecat_set_master_license(license_filename) != ddi_status_ok )
    {
      ELOG(" Warning: License file %s does not contain a valid license for this computer \n", license_filename);
    }
  }

  //------------ Configure the EtherCAT master stack -------------
  result = ecatConfigureMaster(eCnfType_Filename, (unsigned char*)eni_filename, 256);
  //result = ecatConfigureMaster(eCnfType_GenOpENI, NULL, 0);
  if (result != EC_E_NOERROR)
  {
    ELOG( "Cannot configure EtherCAT-Master: %s (0x%x))\n", ecatGetText(result), result);
    if ( result == 0x98110011 )
    {
      ELOG(" The libemllSockRaw.so or other socket library is likely not present in the current directory \n");
    }
    if ( result == 0x98110070 )
    {
      ELOG(" The file you requested is likely not present.  You entered the following eni: %s \n", eni_filename );
    }
    return ddi_status_err;
  }

  //------------  Print out the Ethernet MAC address -------------
  result = ecatGetSrcMacAddress(&mac_address);
  if (0 != result)
  {
    ELOG("Cannot get MAC address! (Result = 0x%x)\n", result);
  }
  DLOG("EtherCAT network adapter MAC: %02X-%02X-%02X-%02X-%02X-%02X\n",
      mac_address.b[0], mac_address.b[1], mac_address.b[2], mac_address.b[3], mac_address.b[4], mac_address.b[5]);

  //------------  Register client for event notification -----------
  VLOG(" register client \n");
  /* Register client */
  memset(&S_oRegisterResults, 0, sizeof(EC_T_REGISTERRESULTS));
  result = ecatRegisterClient(ecatNotifyCallback, NULL, &S_oRegisterResults);
  if (result != EC_E_NOERROR)
  {
      ELOG( "Cannot register client: %s (0x%x))\n", ecatGetText(result), result);
  }
  //save off the client ID
  client_id = S_oRegisterResults.dwClntId;
  VLOG(" done with registering client \n");

  /* create the cyclic data thread */
  ddi_thread_create_with_scheduler(&ddi_thread_handle, SCHED_RR, sched_get_priority_max(SCHED_RR), 2048, "cyclic_process_data", ddi_cyclic_thread, (void*)&ecat_master_cfg );
  ddi_thread_detach(ddi_thread_handle);

  //------------  Scan the bus -----------
  ecatScanBus(10000);
  
  //------------  Set to init  -----------
  VLOG(" setting init \n");
    /* set master and bus state to INIT */
  result = ecatSetMasterState(ETHERCAT_STATE_CHANGE_TIMEOUT, eEcatState_INIT);
  if (result != EC_E_NOERROR)
  {
      ELOG( "Cannot start set master state to INIT: %s (0x%x))\n", ecatGetText(result), result);
      return ddi_status_err;
  }
  VLOG("Change to init master result = %d \n",result);

  return ddi_status_ok;
}

//open a slave instance
ddi_status_t ddi_sdk_ecat_open(slave_t *slave)
{
  uint32_t result;
  uint16_t count;

  for(count=0; count < DDI_MAX_ECAT_INSTANCES; count++)
  {
    if( slave->allocated == 0)
    {
      VLOG("ecat: found free instance at index %d \n", count);
      slave->allocated = 1;
      return ddi_status_ok;
    }
  }
  if (count == DDI_MAX_ECAT_INSTANCES)
  {
    ELOG("ecat: did not find free instance %d \n", count);
    return ddi_status_not_found;
  }
  return ddi_status_ok;
}

//set the state of the ethercat master
ddi_status_t ddi_sdk_ecat_set_master_state(uint32_t state)
{
  uint32_t result;
  const char *state_string="undefined";

  if(state == DDI_ECAT_INIT )
   state_string = "INIT";
  else if ( state == DDI_ECAT_PREOP )
   state_string = "PREOP";
  else if ( state == DDI_ECAT_SAFEOP )
   state_string = "SAFEOP";
  else if ( state == DDI_ECAT_OP )
   state_string = "OP";

  VLOG(" before ecatSetMasterState \n");
  /* set master and bus state to PREOP */
  result = ecatSetMasterState(ETHERCAT_STATE_CHANGE_TIMEOUT, (EC_T_STATE)state);
  if (result != EC_E_NOERROR)
  {
      ELOG("Cannot set master state to %s, result string: %s, result code: (0x%x))\n", state_string, ecatGetText(result), result);
      if(result == 0x98110024 )
      {
         ELOG("This is likely due to an invalid eni file for the slave you are using \n");
         display_alstatus(slave);
      }
      return ddi_status_err;
  }

  VLOG(" ddi_sdk_ecat_set_state to %s success \n", state_string);

  return ddi_status_ok;
}

//set the state of the ethercat master
ddi_status_t ddi_sdk_ecat_set_slave_state(slave_t *slave, uint16_t state)
{
  uint32_t result;
  const char *state_string="undefined";

  if(state == DDI_ECAT_INIT )
   state_string = "INIT";
  if(state == DDI_ECAT_BOOT )
   state_string = "INIT";
  else if ( state == DDI_ECAT_PREOP )
   state_string = "PREOP";
  else if ( state == DDI_ECAT_SAFEOP )
   state_string = "SAFEOP";
  else if ( state == DDI_ECAT_OP )
   state_string = "OP";

  VLOG(" before emSetSlaveState \n");
  /* set master and bus state to PREOP */
  result = ecatSetSlaveState(slave->info.dwSlaveId, (EC_T_STATE)state, ETHERCAT_STATE_CHANGE_TIMEOUT);
  if (result != EC_E_NOERROR)
  {
      ELOG("Cannot set slave state to %s, result string: %s, result code: (0x%x))\n", state_string, ecatGetText(result), result);
      if(result == 0x98110024 )
      {
         ELOG("This is likely due to an invalid eni file for the slave you are using \n");
         display_alstatus(slave);
      }
      return ddi_status_err;
  }
  VLOG(" ddi_sdk_ecat_set_state to %s success \n", state_string);

  return ddi_status_ok;
}

//get the state of the ethercat master
ddi_status_t ddi_sdk_ecat_get_master_state(slave_t *slave, uint32_t *current_state )
{
  uint32_t result;

  /* get master state */
  *current_state = ecatGetMasterState();

  return ddi_status_ok;
}

//set the state of the ethercat master
ddi_status_t ddi_sdk_ecat_get_slave_state(slave_t *slave, uint16_t *state, uint16_t *requested_state)
{
  uint32_t result;

  /* set master and bus state to PREOP */
  result = ecatGetSlaveState(slave->info.dwSlaveId, state, requested_state);
  if (result != EC_E_NOERROR)
  {
    ELOG("Cannot get slave state, result string: %s, result code: (0x%x))\n", ecatGetText(result), result);
    return ddi_status_err;
  }

  return ddi_status_ok;
}



ddi_status_t ddi_sdk_ecat_get_master_stats ( ddi_ecat_master_stats_t * stats)
{
  if(!stats)
    return ddi_status_param_err;
  /* copy over statistics */
  memcpy(stats, &ecat_master_stats, sizeof(ddi_ecat_master_stats_t));
  return ddi_status_ok;
}

/* DDI test entry point, this function gets executed each ethercat train */
int ddi_sdk_process_cyclic_frame (slave_t *slave)
{
  //lost frame detection
  if (ecat_master_stats.lost_frame_count)
  {
    ddi_sdk_log_error(" lost frame detected - not processing inputs \n");
  }
  else
  {
    //process the cyclic data callback for that slave
    if ( slave->cyclic_func )
      slave->cyclic_func(slave->callback_args);
    else if (slave->cyclic_object)
        slave->cyclic_object->cyclic_method(slave->callback_args);
  }
  return 0;
}

//this is the main EtherCAT cyclic thread
//it runs at 1ms. If a slave requests a slower rate, then this thread can accomodate that by
void * ddi_cyclic_thread(const void *arg)
{
  int result, count;
  ntime_t deadline;
  static uint32_t lost_frame_count = 0;
  uint32_t time_since_wakeup_us[DDI_SDK_MAX_INSTANCES];
  EC_T_USER_JOB_PARMS  oJobParms;
  memset(&oJobParms, 0, sizeof(EC_T_USER_JOB_PARMS));

  while(1)
  {
    // Derive the next scheduled time from the current time in case there is gap in the Linux scheduler
    ddi_ntime_get_systime(&deadline);
    uint64_t cycle_time_ns = ecat_master_cfg.master_scan_rate_us * NSEC_PER_USEC;
    ddi_ntime_add_ns(&deadline, 0, cycle_time_ns); // Increment the deadline
    sched_yield(); // Give other processes a chance to run since this is the Highest priority thread
    ddi_ntime_sleep_ns(&deadline); // clock_nanosleep using TIMER_ABSTIME

    // process cyclic data receive
    result = ecatExecJob(eUsrJob_ProcessAllRxFrames,&oJobParms);
    if (EC_E_NOERROR == result)
    {
      if (!oJobParms.bAllCycFramesProcessed)
      {
        ecat_master_stats.lost_frame_count++;
        //ELOG(" Lost frame detected \n");
        ddi_sdk_log_error(" Lost frame detected \n");
        if (lost_frame_count == LOST_FRAME_COUNT_MAX)
        {
          ELOG(" warning: %d consecutive lost frames received\n", lost_frame_count);
        }
      }
      else
      {
        //reset lost frame count on successful rx completion
        ecat_master_stats.lost_frame_count = 0;
      }
    }
    for (count = 0; count < DDI_SDK_MAX_INSTANCES; count++)
    {
      if (slave[count].allocated)
      {
        if ((slave[count].pd_input != EC_NULL) && (slave[count].pd_output != EC_NULL))
        {
          ddi_sdk_process_cyclic_frame(&slave[count]);
        }
      }
    }
    //process cyclic data trasnmit
    result = ecatExecJob(eUsrJob_SendAllCycFrames, &oJobParms);
    if (EC_E_NOERROR != result && EC_E_INVALIDSTATE != result && EC_E_LINK_DISCONNECTED != result)
    {
      ELOG( "ecatExecJob( eUsrJob_SendAllCycFrames,    EC_NULL ): %s (0x%x)\n", ecatGetText(result), result);
    }
    /* Execute some administrative jobs. No bus traffic is performed by this function */
    result = ecatExecJob(eUsrJob_MasterTimer, EC_NULL);
    if (EC_E_NOERROR != result && EC_E_INVALIDSTATE != result)
    {
      ELOG("ecatExecJob(eUsrJob_MasterTimer, EC_NULL): %s (0x%x)\n", ecatGetText(result), result);
    }
    //send acyclic frames
    result = ecatExecJob(eUsrJob_SendAcycFrames, EC_NULL);
    if (EC_E_NOERROR != result && EC_E_INVALIDSTATE != result && EC_E_LINK_DISCONNECTED != result)
    {
      ELOG("ecatExecJob(eUsrJob_SendAcycFrames, EC_NULL): %s (0x%x)\n", ecatGetText(result), result);
    }
  }
}
