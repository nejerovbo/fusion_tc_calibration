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
#include <inttypes.h>
#include <AtEthercat.h>
#include "ddi_macros.h"
#include "ddi_status.h"
#include "ddi_os.h"
#include "ddi_debug.h"
#include "ddi_em_config.h"
#include "ddi_em_api.h"
#include "ddi_em_link_layer.h"
#include "ddi_ntime.h"
#include "ddi_em.h"
#include "ddi_em_logging.h"
#include "ddi_em_translate.h"
#include "ddi_em_remote_access.h"
#include "ddi_em_fusion_interface.h"
#include "ddi_em_slave_management.h"

// This file provides basic master capability such as cyclic thread scheduling, SDK initialization
// It contains the main functionality of the DDI ECAT Master SDK

// Global flag that keeps track of if the SDK is initialized or not
static uint32_t g_sdk_initalized = DDI_EM_FALSE;

// Master structure for each supported instance
static ddi_em_instance g_em_instance[DDI_EM_MAX_MASTER_INSTANCES];

// Required for linking in DLOG/ELOG/VLOG macros without using the ddi_em_logging.h file
int ddi_log_level = DDI_EM_LOG_LEVEL_ERRORS;

// Get the master instance pointer for a master handle
ddi_em_instance* get_master_instance(ddi_em_handle em_handle)
{
  return &g_em_instance[em_handle];
}

// Get the slave instance pointer for a slave handle
ddi_em_slave* get_slave_instance(ddi_em_handle em_handle, ddi_em_handle es_handle)
{
  return &g_em_instance[em_handle].slave_info[es_handle];
}

// De-initialize a master instance
EM_API ddi_em_result ddi_em_deinit(ddi_em_handle em_handle)
{
  uint32_t ec_result;
  ddi_em_result result;
  // Shutdown and free the link layer instance
  link_layer_deinit(em_handle);
  result = shutdown_thread_instance(em_handle);
  if ( result != DDI_EM_STATUS_OK )
  {
    ELOG(em_handle, "Shutting down cyclic thread failed %s \n", ddi_em_get_error_string(result));
  }

  // If there's a notification active on this instance, de-register the client
  if ( g_em_instance[em_handle].master_status.notification_registered )
  {
    ec_result = emUnregisterClient(em_handle, g_em_instance[em_handle].master_status.notification_id);
    if (ACONTIS_SUCCESS != ec_result)
    {
      result = translate_ddi_acontis_err_code(em_handle, ec_result);
      ELOG(em_handle, "Event Handler Deinit failed %s \n", ddi_em_get_error_string(result));
    }
  }

  // De-initialize the Acontis EC Master instance
  // The result from the Master De-init takes priority over the registration deinit
  ec_result = emDeinitMaster(em_handle);
  if (ACONTIS_SUCCESS != ec_result)
  {
    result = translate_ddi_acontis_err_code(em_handle, ec_result);
    ELOG(em_handle, "EtherCAT Master Deinit failed %s \n", ddi_em_get_error_string(result));
  }

  ddi_em_close_all_slave_handles(em_handle);
  return result;
}

// Return the DDI ECAT SDK version
EM_API const char* ddi_em_get_version()
{
  return DDI_EM_VERSION;
}

// Register the cyclic callback for an instance
EM_API ddi_em_result ddi_em_register_cyclic_callback(ddi_em_handle em_handle, ddi_em_cyclic_func *callback, void *user_data)
{
  VALIDATE_INSTANCE(em_handle); // Validate the instance argument
  g_em_instance[em_handle].master_config.cyclic_args = user_data;
  g_em_instance[em_handle].master_config.cyclic_callback = callback;
  return DDI_EM_STATUS_OK;
}

// Shutdown the thread instance
ddi_em_result shutdown_thread_instance(ddi_em_handle em_handle)
{
  int timeout_count_ms = MSEC_PER_SEC; // One second timeout
  // Start cyclic thread termination
  g_em_instance[em_handle].master_config.thread_exit_enabled = true;
  if ( g_em_instance[em_handle].master_config.cyclic_thread_enabled ) // If there's a thread, wait for it to exit
  {
    // Wait for cyclic thread to exit
    ddi_thread_join(g_em_instance[em_handle].master_status.cyclic_thread_handle,NULL);
  }
  while ( g_em_instance[em_handle].master_status.thread_exit_occurred == 0 ) // Wait for the cyclic thread to exit
  {
    usleep(USEC_PER_MSEC);  // Delay until thread exit occurred
    if ( timeout_count_ms-- == 0)
    {
      return DDI_EM_STATUS_TIMEOUT;
    }
  }

  // Reset thread state parameters
  g_em_instance[em_handle].master_config.enabled = false;
  g_em_instance[em_handle].master_config.thread_exit_enabled = false;
  return DDI_EM_STATUS_OK;
}

// Log cyclic statistics to the master-specific instance
ddi_em_result log_cyclic_stastics (ddi_em_instance *instance, ddi_em_master_stats *stats)
{
  ntime_t current_ts;
  ddi_ntime_get_systime(&current_ts);
  uint64_t total_cyclic_frames = stats->cyclic_frames_with_no_errors + stats->cyclic_err_frame_count;
  if ( total_cyclic_frames > 0 ) // If it's not the first cyclic frame, log cyclic statistics
  {
    int64_t cyclic_delta_ns;
    cyclic_delta_ns = ddi_ntime_diff_ns(&current_ts, &instance->master_status.prev_cyclic_timestamp);
    if ( cyclic_delta_ns < 0 )
    {
      ELOG(instance->master_config.em_handle, "Cyclic timestamp delta was negative\n");
    }
    else
    {
      // Keep track of the max and min deviation
      if ( (uint32_t)cyclic_delta_ns > stats->max_cyclic_timestamp_diff_ns )
      {
        stats->max_cyclic_timestamp_diff_ns = (uint32_t)cyclic_delta_ns;
      }
      if ( ((uint32_t)cyclic_delta_ns < stats->min_cyclic_timestamp_diff_ns) || (stats->min_cyclic_timestamp_diff_ns == 0) )
      {
        stats->min_cyclic_timestamp_diff_ns = (uint32_t)cyclic_delta_ns;
      }
      // Keep track of the average cyclic data delta
      instance->master_status.average_cyclic_delta_sum += (uint64_t)cyclic_delta_ns;
      stats->average_cyclic_timestamp_diff_ns = instance->master_status.average_cyclic_delta_sum /total_cyclic_frames;
    }
  }
  // Update the previous timestmap
  instance->master_status.prev_cyclic_timestamp = current_ts;

  return DDI_EM_STATUS_OK;
}

// Perform Acontis-related job update duties
static uint32_t cyclic_update (ddi_em_instance *instance)
{
  ddi_em_handle em_handle = instance->master_config.em_handle;
  static uint32_t lost_frame_count = 0;
  uint32_t result;
  EC_T_USER_JOB_PARMS  oJobParms;
  memset(&oJobParms, 0, sizeof(EC_T_USER_JOB_PARMS));
  ddi_em_master_stats *stats;
  stats = &instance->master_status.master_stats;

  // Process cyclic data receive
  result = emExecJob(em_handle, eUsrJob_ProcessAllRxFrames,&oJobParms);
  if (EC_E_NOERROR == result)
  {

    if (!oJobParms.bAllCycFramesProcessed)
    {
      stats->cyclic_err_frame_count++;
      stats->cur_consecutive_err_frame_count++;
      if ( stats->cur_consecutive_err_frame_count > stats->max_consecutive_err_frame_count )
      {
        // Update the maximum number of lost frames with the current value
        stats->max_consecutive_err_frame_count = stats->cur_consecutive_err_frame_count;
      }
      ELOG(em_handle, "Master[%d] cyclic thread lost frame detected \n", em_handle);
      if (stats->cur_consecutive_err_frame_count == DDI_EM_LOST_FRAME_COUNT_MAX)
      {
        ELOG(em_handle,"Master[%d] cyclic thread %d consecutive lost frames received\n", em_handle, lost_frame_count);
      }
    }
    else
    {
      stats->cyclic_frames_with_no_errors++;
      // reset lost frame count on successful rx completion
      stats->cur_consecutive_err_frame_count=0;
    }
  }

  // If the master cyclic callback function is registered and the master state is greater than INIT, execute the callback
  if ( (instance->master_config.cyclic_callback  != NULL) && (emGetMasterState(em_handle) >= eEcatState_INIT) )
  {
    // Call the process data callback
    instance->master_config.cyclic_callback(instance->master_config.cyclic_args);
  }

  // Handle any Fusion-specific processing
  ddi_em_fusion_handle_process_data(instance->master_config.em_handle);

  // Record cyclic statistics
  log_cyclic_stastics(instance, stats);

  // Process cyclic data transmit
  result = emExecJob(em_handle, eUsrJob_SendAllCycFrames, &oJobParms);
  if (EC_E_NOERROR != result && EC_E_INVALIDSTATE != result && EC_E_LINK_DISCONNECTED != result)
  {
    ELOG(em_handle, "Master[%d] cyclic thread - Acyclic Frames: %s (0x%x)\n", em_handle, ecatGetText(result), result);
  }
  /* Execute some administrative jobs. No bus traffic is performed by this function */
  result = emExecJob(em_handle, eUsrJob_MasterTimer, EC_NULL);
  if (EC_E_NOERROR != result && EC_E_INVALIDSTATE != result)
  {
    ELOG(em_handle, "Master[%d] cyclic thread - Admin Jobs: %s (0x%x)\n", em_handle, ecatGetText(result), result);
  }
  //send acyclic frames
  result = emExecJob(em_handle, eUsrJob_SendAcycFrames, EC_NULL);
  if (EC_E_NOERROR != result && EC_E_INVALIDSTATE != result && EC_E_LINK_DISCONNECTED != result)
  {
    ELOG(em_handle, "Master[%d] cyclic thread - Acyclic Frames: %s (0x%x)\n", em_handle, ecatGetText(result), result);
  }

  return result;
}

static ddi_em_result cyclic_thread_scheduler (ddi_em_instance *instance)
{
  ntime_t deadline;
  ntime_t current_time;
  pthread_t current_thread_tid;

  ddi_ntime_get_systime(&current_time);
  deadline.ns = current_time.ns;
  deadline.sec = current_time.sec;
  ddi_ntime_add_ns(&deadline, 0, instance->master_config.bus_cycle_us * NSEC_PER_USEC); // Increment the deadline

  if ( instance->master_config.enable_cpu_affinity ) // Enable CPU affinity for this thread, Move this to ddi_common
  {
    current_thread_tid = pthread_self();
    cpu_set_t cpu_set;
    CPU_ZERO(&cpu_set);
    CPU_SET(instance->master_config.cyclic_cpu_select, &cpu_set);
    int ret = pthread_setaffinity_np(current_thread_tid, sizeof(cpu_set_t), &cpu_set);
    if ( ret != 0 )
    {
      ELOG(instance->master_config.em_handle, "Master[%d] init: Setting CPU affinity failed ret %d \n", instance->master_config.em_handle, ret);
      return DDI_EM_STATUS_CPU_AFFINITY_ERR;
    }
  }

  while(instance->master_config.thread_exit_enabled == 0)
  {
    ddi_ntime_sleep_ns(&deadline); // Sleep until the deadline using clock_nanosleep
    cyclic_update(instance); // Update the cyclic job
    ddi_ntime_add_ns(&deadline, 0, instance->master_config.bus_cycle_us * NSEC_PER_USEC); // Increment the deadline;
  }

  instance->master_status.thread_exit_occurred = 1;
  return DDI_EM_STATUS_OK;
}

static void * ddi_cyclic_thread(const void *arg)
{
  ddi_em_instance *instance = (ddi_em_instance *)arg;
  cyclic_thread_scheduler(instance);
  return NULL;
}

// Start the cyclic task.  This function is used if the cyclic thread is created outside of the DDI ECAT SDK
// This function will still perform scheduling. It can be stopped with the ddi_em_cyclic_task_stop() call
ddi_em_result ddi_em_cyclic_task_start(ddi_em_handle em_handle)
{
  ddi_em_instance *instance = (ddi_em_instance *)&g_em_instance[em_handle];
  return cyclic_thread_scheduler(instance);
}

// Stop the cyclic task.  This function is used if the cyclic thread is created outside of the DDI ECAT SDK
ddi_em_result ddi_em_cyclic_task_stop(ddi_em_handle em_handle)
{
  ddi_em_result result;
  result = shutdown_thread_instance(em_handle); // Shutdown the cyclic thread
  if (result == DDI_EM_STATUS_OK )
  {
    return result;
  }
  else // Return thread stop error
  {
    return DDI_EM_STATUS_CYC_THREAD_STOP;
  }
}

// Returns the NIC MAC address given by the instance handle
static ddi_em_result ddi_em_get_em_nic_mac_addr(ddi_em_handle instance, char *mac_addr_string)
{
  uint32_t result;
  ETHERNET_ADDRESS ecat_mac_addr;

  // Check argument validity
  if(!mac_addr_string)
    return DDI_EM_STATUS_INVALID_ARG;

  result = emGetSrcMacAddress(instance, &ecat_mac_addr);
  if (result != EC_E_NOERROR)
  {
    ELOG(instance, "Master[%d] init: Cannot get MAC address: %s (0x%x))\n", instance, ecatGetText(result), result);
    return DDI_EM_NIC_MAC_ACCESS_ERR;
  }
  VLOG(instance, "Master[%d] init: EtherCAT network adapter MAC: %02X-%02X-%02X-%02X-%02X-%02X\n", instance,
      ecat_mac_addr.b[0], ecat_mac_addr.b[1], ecat_mac_addr.b[2], ecat_mac_addr.b[3], ecat_mac_addr.b[4], ecat_mac_addr.b[5]);

  // Create a mac address nn-nn-nn-nn-nn-nn
  sprintf(mac_addr_string, "%02X-%02X-%02X-%02X-%02X-%02X\n",
      ecat_mac_addr.b[0], ecat_mac_addr.b[1], ecat_mac_addr.b[2], ecat_mac_addr.b[3], ecat_mac_addr.b[4], ecat_mac_addr.b[5]);

  return DDI_EM_STATUS_OK;
}

// Set the master license for a particular NIC card
// The format of the license file is a CSV file which has the following fields:
// <license_number>, <date>, <MAC address>, <key> e.g.,
// 1,11.02.2020,68-05-CA-0F-C9-A1,6A0F5256-1497A34F-0A953150
static ddi_em_result ddi_em_set_master_license ( ddi_em_handle instance, const char *filename )
{
  FILE *fp;
  char buffer[256];
  char adapter[128];
  char *tmp;
  char *MAC;
  char *license;
  uint32_t result;
  ddi_em_result em_result;

  fp = fopen(filename, "r+");
  if (!fp)
  {
    ELOG(instance, "Master[%d] init: Invalid filename %s \n", instance, filename);
    return DDI_EM_STATUS_INVALID_ARG;
  }
  // Get the adapter mac address
  em_result = ddi_em_get_em_nic_mac_addr(instance, adapter);
  if ( em_result != DDI_EM_STATUS_OK )
  {
    ELOG(instance, "Master[%d] init: Issue getting adapter mac address result = 0x%x \n", instance, em_result);
    fclose(fp);
    return em_result;
  }

  // loop until end of file is reached, fgets returns null when EOF is reached.
  while (fgets (buffer , sizeof(buffer) , fp) != NULL )
  {
    tmp = strchr(buffer,',');
    if(tmp == NULL)
      continue;
    // The MAC is after the second semicolon
    MAC = strchr(++tmp,',');
    // The MAC is after the third semicolon
    license = strchr(++MAC,',');
    if( !strncmp (adapter, MAC, 17))
    {
      DLOG(instance, "Master[%d] init: Licensed adapter found at %s \n", instance, adapter);
      g_em_instance[instance].master_config.licensed_status = true;
      // need to remove the last semicolon from the license line
      license[27]='\0';
      // remove the first semicolon
      license++;
      result = emSetLicenseKey(instance, license);
      if ( result != ACONTIS_SUCCESS )
      {
        return DDI_EM_STATUS_INVALID_LIC;
      }
      fclose(fp);
      return DDI_EM_STATUS_OK;
    }
  }

  WLOG(instance, "Master[%d] init: Locating master license failed \n", instance);
  fclose(fp);
  return DDI_EM_STATUS_INVALID_LIC;
}

EM_API ddi_em_result ddi_em_sdk_init(void)
{
  ddi_em_result result = DDI_EM_STATUS_OK;
  if(geteuid() != 0) // Ensure this SDK is ran as root
  {
    printf(RED "Please execute this SDK as a root-user" CLEAR "\n");
    return DDI_EM_STATUS_NON_ROOT_USER;
  }
  if ( g_sdk_initalized == 0 ) // Ensure this SDK has not been previously initialized
  {
    // Initialize the global instance structure the first time SDK init is called
    memset(g_em_instance, 0, sizeof(g_em_instance));
    // Clear the remote access instance data
    ddi_em_remote_access_clear_instance_data();
    g_sdk_initalized = 1;
    // Initialize the logging subsystem
    result = ddi_em_log_init();
  }
  return result;
}

EM_API ddi_em_result ddi_em_sdk_deinit(void)
{
  // De-initialize the global instance structure
  g_sdk_initalized = 0;
  return DDI_EM_STATUS_OK;
}

static ddi_em_result get_next_instance (ddi_em_handle *handle)
{
  int count;
  for ( count = 0; count < DDI_EM_MAX_MASTER_INSTANCES; count++)
  {
    if ( g_em_instance[count].master_config.enabled == 0 )
    {
      // Clear the instance state and configuration data
      memset(&g_em_instance[count], 0, sizeof(ddi_em_instance));
      DLOG(count, "Master[%d] init: Available master instance found \n", count);
      *handle = count;
      g_em_instance[count].master_config.enabled = 1;
      return DDI_EM_STATUS_OK;
    }
  }
  return DDI_EM_STATUS_NO_RESOURCES;
}

// Instance version of the init routine
ddi_em_result ddi_em_init(ddi_em_init_params *em_init_params, ddi_em_handle *em_handle)
{
  EC_T_INIT_MASTER_PARMS     init_params;
  ETHERNET_ADDRESS           mac_address;
  uint32_t                   result;
  ddi_em_result              em_result;
  ddi_em_handle              instance;
  char                       *license_file_name;

  if ( g_sdk_initalized == 0 )
    return DDI_EM_SDK_NOT_INITIALIZED;
  
  // Clear the Acontis initialization structure
  memset(&init_params, 0, sizeof(EC_T_INIT_MASTER_PARMS));

  // Validate function input parameters
  if ( (em_init_params == NULL) || (em_handle == NULL ))
  {
    // Logging not available yet for this instance
    printf(RED "Master init: ddi_em_init() Argument null" CLEAR "\n");
    return DDI_EM_STATUS_INVALID_ARG;
  }
  // Validate the CPU affinity arguments
  if ( em_init_params->enable_cyclic_thread && em_init_params->enable_cpu_affinity
         && (em_init_params->cyclic_cpu_select <DDI_EM_CPU_MIN) && (em_init_params->cyclic_cpu_select > DDI_EM_CPU_MAX ))
  {
    // Logging not available yet for this instance
    printf(RED "Master init: CPU affinity selection invalid " CLEAR "\n");
    return DDI_EM_STATUS_CPU_AFFINITY_ERR;
  }

  em_result=get_next_instance(em_handle);
  if ( em_result != DDI_EM_STATUS_OK ) // Validate instance return code
  {
    printf(RED "Master instance failed \n");
    return em_result;
  }
  instance = *em_handle;
  VALIDATE_INSTANCE(instance); // Validate the instance argument

  em_result = ddi_em_logging_init(instance); // Initialize logging subsystem for this master instance
  if (em_result != DDI_EM_STATUS_OK ) // Validate logging mechanism initialized properly
  {
    printf(RED "Master[%d] init: Issue with initalizing logging " CLEAR "\n", instance);
    return DDI_EM_STATUS_LOG_DIR_FAILED;
  }

  // Check if the given network adapater has been used in another master instance
  em_result = link_layer_adapter_allocated(em_init_params->network_adapter);
  if ( em_result != DDI_EM_STATUS_OK )
  {
    ELOG(instance, "Master[%d] init: Network adapter %d already registered \n", instance, em_init_params->network_adapter);
    return DDI_EM_NIC_ALREADY_REG;
  }
  ddi_mutex_create(&g_em_instance[instance].master_status.pd_out_mutex);
  g_em_instance[instance].master_config.em_handle = instance;
  // Set the initial scan rate
  g_em_instance[instance].master_config.bus_cycle_us = em_init_params->scan_rate_us;
  // Create the optimized link layer instance
  link_layer_i8254_init(instance,&g_em_instance[instance].master_config.param_ptr, em_init_params->network_adapter);

  //------------- Setup the EtherCAT master-------------------------
  init_params.dwSignature                   = ATECAT_SIGNATURE;
  init_params.dwSize                        = sizeof(EC_T_INIT_MASTER_PARMS);
  init_params.pLinkParms                    = g_em_instance[instance].master_config.param_ptr;
  init_params.pLinkParmsRed                 = EC_NULL; //redundancy params
  init_params.pOsParms                      = EC_NULL; //OS parameters
  // Set the cyclic update rate
  init_params.dwBusCycleTimeUsec            = em_init_params->scan_rate_us;
  init_params.dwMaxBusSlaves                = DDI_EM_MAX_BUS_SLAVES;
  // The following parameters determine the amount of bytes and acyclic commands sent per cyclic update
  // These values were borrowed from the Acontis EcMasterDemo.cpp file
  init_params.dwMaxAcycFramesQueued         = DDI_EM_MAX_ACYC_FRAMES_QUEUED;
  init_params.dwMaxAcycFramesPerCycle       = DDI_EM_MAX_ACYC_FRAMES_PER_CYCLE;
  init_params.dwMaxAcycCmdsPerCycle         = DDI_EM_MAX_ACYC_CMD_PER_CYCLE;
  if (init_params.dwBusCycleTimeUsec >= 1000)
  {
    init_params.dwMaxAcycBytesPerCycle      = DDI_EM_MAX_ACYC_BYTES_PER_CYC_1MS;
  }
  else
  {
    init_params.dwMaxAcycBytesPerCycle      = DDI_EM_MAX_ACYC_BYTES_PER_CYC;
  }
  init_params.dwEcatCmdMaxRetries           = DDI_EM_CFG_MAX_ACYC_CMD_RETRIES;

  //------------ Initialize the EtherCAT master stack -------------
  result = emInitMaster(instance, &init_params);
  if (result != EC_E_NOERROR)
  {
    ELOG (instance, "Master[%d] init: Cannot initialize EtherCAT-Master: %s (0x%x))\n", instance, ecatGetText(result), result);
    if ( result == EC_E_ERROR )
    {
      ELOG(instance, "Master[%d] init: You likely need to run this program as sudo or the network interface is incorrect \n", instance);
    }
    if ( result == EC_E_INVALIDPARM )
    {
      ELOG(instance, "Master[%d] init: This is likely due to missing the network interface driver (i.e. libemllSockRaw.so). \n \
              The .so file needs to be in the LD_LIBRARY_PATH or your local directory \n", instance);
    }
    return translate_ddi_acontis_err_code(instance, result); // Return the Acontis->DDI translated error code
  }

  DLOG(instance, "Master[%d] init: init master result = 0x%04x \n",result);

  // Setup the license file
  license_file_name = getenv("DDI_EM_LICENSE_FILE");
  // If the DDI_EM_LICENSE_FILE enviornent variable is not defined, use the default license file
  if ( license_file_name == NULL )
  {
    license_file_name = (char *)DDI_EM_LICENSE_FILE;
  }

  // Attempt setting the license using the environment or default parameter
  if ( g_em_instance[instance].master_config.licensed_status == false )
  {
    em_result = ddi_em_set_master_license(instance, license_file_name);
    if ( em_result != DDI_EM_STATUS_OK )
    {
      WLOG(instance, "Master[%d] init: License file %s does not contain a valid license for this computer \n", instance, license_file_name);
    }
  }

  //------------  Print out the Ethernet MAC address -------------
  result = emGetSrcMacAddress(instance, &mac_address);
  if (ACONTIS_SUCCESS != result)
  {
    ELOG(instance, "Master[%d] init: Cannot get MAC address! (Result = 0x%x)\n", instance, result);
    return translate_ddi_acontis_err_code(instance, result); // Return the Acontis->DDI translated error code
  }
  DLOG(instance, "Master[%d] init: EtherCAT network adapter MAC: %02X-%02X-%02X-%02X-%02X-%02X\n", instance,
      mac_address.b[0], mac_address.b[1], mac_address.b[2], mac_address.b[3], mac_address.b[4], mac_address.b[5]);

  if ( em_init_params->enable_cyclic_thread == 1 )
  {
    g_em_instance[instance].master_config.cyclic_thread_enabled = 1;
    // create the cyclic data thread
    ddi_thread_create_with_scheduler(&g_em_instance[instance].master_status.cyclic_thread_handle,
      SCHED_RR, em_init_params->polling_thread_priority, DDI_CYCLIC_THREAD_STACK_SIZE, "cyclic_process_data", ddi_cyclic_thread, (void*)&g_em_instance[instance]);
    ddi_thread_detach(g_em_instance[instance].master_status.cyclic_thread_handle);

    g_em_instance[instance].master_config.enable_cpu_affinity = em_init_params->enable_cpu_affinity;
    g_em_instance[instance].master_config.cyclic_cpu_select = em_init_params->cyclic_cpu_select;
  }

  // EM-57, update network control flags in the master configuration instance
  g_em_instance[instance].master_config.network_control_flags = em_init_params->network_control_flags;

  return DDI_EM_STATUS_OK;
}

// Configure the ENI file and transition the master to INIT
EM_API ddi_em_result ddi_em_configure_master(ddi_em_handle em_handle, const char* eni_filename)
{
  uint8_t enable_partial_network_support = 0;
  uint32_t result;

  VALIDATE_INSTANCE(em_handle); // Validate the instance argument

  // If the DDI_EM_NETWORK_MASTER_STATE_CHECK_DISABLE flag is set, enable patial network support
  if (g_em_instance[em_handle].master_config.network_control_flags & DDI_EM_NETWORK_MASTER_STATE_CHECK_DISABLE)
  {
    enable_partial_network_support = 1;
  }

  // EM-57: Check for the disabling of WKC, this setting allows for partial EtherCAT network suppport by preventing WKC errors each cycle
  // With DDI_EM_NETWORK_DISABLE_WKC set, the network requires all slaves in the ENI to increment their WKC to go into SAFEOP mode
  if ((g_em_instance[em_handle].master_config.network_control_flags & DDI_EM_NETWORK_DISABLE_WKC) || enable_partial_network_support)
  {
    // expected WKC should automatically ajusted according the presence and state of the slaves
    EC_T_BOOL adjust_wkc = EC_TRUE;
    result = emIoCtl(em_handle, EC_IOCTL_SET_AUTO_ADJUST_CYCCMD_WKC_ENABLED, (EC_T_BYTE*)&adjust_wkc, sizeof(EC_T_BOOL), EC_NULL, 0, EC_NULL);
    if ( result != ACONTIS_SUCCESS )
    {
      return translate_ddi_acontis_err_code(em_handle, result); // Return the Acontis->DDI translated error code
    }
  }

  // Configure the EtherCAT master stack
  result = emConfigureMaster(em_handle, eCnfType_Filename, (uint8_t *)eni_filename, strnlen(eni_filename, 256));
  if (result != ACONTIS_SUCCESS)
  {
    if ( result == EC_E_OPENFAILED )
    {
      ELOG(em_handle, "Master[%d] configure: The libemllSockRaw.so or other socket library is likely not present in the current directory \n", em_handle);
      return translate_ddi_acontis_err_code(em_handle, result); // Return the Acontis->DDI translated error code
    }
    else if ( result == EC_E_CFGFILENOTFOUND )
    {
      ELOG(em_handle, "Master[%d] configure: The file you requested is likely not present.  You entered the following eni: %s \n", em_handle, eni_filename );
      return translate_ddi_acontis_err_code(em_handle, result); // Return the Acontis->DDI translated error code
    }
  }

  // Update the input process data pointer for this master
  g_em_instance[em_handle].master_config.pd_input  = emGetProcessImageInputPtr(em_handle);
  // Update the output process data pointer for this master
  g_em_instance[em_handle].master_config.pd_output = emGetProcessImageOutputPtr(em_handle);

  // Scan the EtherCAT network
  result = emScanBus(em_handle, DDI_EM_SCAN_NETWORK_TIMEOUT);

  // EM-57: If there's an error scanning the network, check the DDI_EM_NETWORK_MASTER_STATE_CHECK_DISABLE flag
  if (result == EC_E_BUSCONFIG_MISMATCH)
  {
    // EM-57: Check for the disabling of all slaves much reach the master state, this setting allows for partial EtherCAT network support
    // With DDI_EM_NETWORK_MASTER_STATE_CHECK_ENABLE set, the network requires all slaves to reach the master state
    if (enable_partial_network_support)
    {
      // If the master state enforcement is removed, call the Acontis IOCTL to disable the enforcement
      EC_T_BOOL bAllSlavesMustReachMasterstate = EC_FALSE;
      result = emIoCtl(em_handle, EC_IOCTL_ALL_SLAVES_MUST_REACH_MASTER_STATE, (EC_T_BYTE*)&bAllSlavesMustReachMasterstate, sizeof(EC_T_BOOL), EC_NULL, 0, EC_NULL);
      if ( result != ACONTIS_SUCCESS )
      {
        return translate_ddi_acontis_err_code(em_handle, result); // Return the Acontis->DDI translated error code
      }
    }
    else
    {
      return translate_ddi_acontis_err_code(em_handle, result); // Return the Acontis->DDI translated error code
    }
  }

  //------------  Set to init  -----------
  VLOG(em_handle, "Master[%d] configure: Setting master to init \n", em_handle);
    /* set master and bus state to INIT */
  result = emSetMasterState(em_handle, DDI_EM_SCAN_NETWORK_TIMEOUT, eEcatState_INIT);
  if (result != EC_E_NOERROR)
  {
    ELOG(em_handle, "Master[%d] configure: Cannot start set master state to INIT: %s (0x%x))\n", em_handle, ecatGetText(result), result);
    return translate_ddi_acontis_err_code(em_handle, result); // Return the Acontis->DDI translated error code
  }
  VLOG(em_handle, "Master[%d] configure: Change to init master result = 0x%x \n",em_handle, result);
  return DDI_EM_STATUS_OK;
}

// Set the cycle rate
EM_API ddi_em_result ddi_em_set_cycle_rate(ddi_em_handle em_handle, uint32_t cycle_rate_us)
{
  VALIDATE_INSTANCE(em_handle); // Validate the instance argument
  g_em_instance[em_handle].master_config.bus_cycle_us = cycle_rate_us;
  return DDI_EM_STATUS_OK;
}

// Set the master statistics
EM_API ddi_em_result ddi_em_get_master_stats(ddi_em_handle em_handle, ddi_em_master_stats *master_stats)
{
  VALIDATE_INSTANCE(em_handle); // Validate the instance argument
  if ( (master_stats == NULL) || ( em_handle < 0 ) || (em_handle > DDI_EM_MAX_MASTER_INSTANCES))
  {
    return DDI_EM_STATUS_INVALID_ARG;
  }
  memcpy(master_stats,&g_em_instance[em_handle].master_status.master_stats, sizeof(ddi_em_master_stats));
  return DDI_EM_STATUS_OK;
}

// Return the Fusion SDK handle for an EtherCAT Master and EtherCAT slave handle
ddi_fusion_sdk_handle get_fusion_sdk_handle(ddi_em_handle em_handle, ddi_es_handle es_handle)
{
  return g_em_instance[em_handle].slave_info[es_handle].fusion_handle;
}
