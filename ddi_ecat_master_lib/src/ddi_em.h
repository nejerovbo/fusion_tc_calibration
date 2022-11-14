/**************************************************************************
(c) Copyright 2021-2022 Digital Dynamics Inc. Scotts Valley CA USA.
Unpublished copyright. All rights reserved. Contains proprietary and
confidential trade secrets belonging to DDI. Disclosure or release without
prior written authorization of DDI is prohibited.
**************************************************************************/

#ifndef DDI_EM_H
#define DDI_EM_H

// Internal instance definion for the DDI ECAT master SDK

#include <AtEthercat.h>
#include "ddi_em_api.h"
#include "ddi_em_config.h"
#include "ddi_os.h"
#include "ddi_ntime.h"
#include "ddi_em_fusion_interface.h"

/** @struct ddi_em_init_params
 *  @brief Slave information structure
 */
typedef struct {
  ddi_em_slave_config      cfg_info;           /**< Slave configuration information */
  bool                     allocated;          /**< Is this slave in use */
  ddi_fusion_sdk_handle    fusion_handle;      /**< Fusion instance handle (if applicable) */
} ddi_em_slave;

/** @struct ddi_em_status
 *  @brief Master status information
 */
typedef struct {
  ddi_em_master_stats master_stats;            /**< EtherCAT Master statistic structure */
  ntime_t             prev_cyclic_timestamp;   /**< Previous cyclic timestamp (used in statistics calculation) */
  uint32_t            max_cyclic_delta_ns;     /**< Max cyclic delta jitter detected */
  uint32_t            min_cyclic_delta_ns;     /**< Min cyclic delta jitter detected */
  uint32_t            average_cyclic_delta_ns; /**< Average cyclic delta reading  */
  uint64_t            average_cyclic_delta_sum;/**< Average accumulator  */
  ddi_mutex_handle_t  pd_out_mutex;            /**< Mutex for setting the output process data */
  ddi_thread_handle_t cyclic_thread_handle;    /**< Cyclic thread handle */
  uint32_t            thread_exit_occurred;    /**< Has the thread exit occurred? */
  uint32_t            notification_registered; /**< Has the event notification been registered? */
  uint32_t            notification_id;         /**< The acontis-based notification id */
} ddi_em_status;

/** @struct ddi_em_config
 *  @brief Master configuration information
 */
typedef struct {
  EC_T_LINK_PARMS*     param_ptr;              /**< Link layer parameters */
  bool                 licensed_status;        /**< Is the SDK licensed? */
  uint32_t             bus_cycle_us;           /**< Cyclic rate in microseconds */
  bool                 enabled;                /**< Is this master instance enabled? */
  ddi_em_cyclic_func*  cyclic_callback;        /**< Cyclic callback pointer */
  void*                cyclic_args;            /**< Cyclic callback arguments for this instance */
  uint8_t*             pd_input;               /**< PD input pointer */
  uint8_t*             pd_output;              /**< PD output pointer */
  ddi_em_handle        em_handle;              /**< EtherCAT Master handle */
  bool                 thread_exit_enabled;    /**< Force a thread exit*/
  uint32_t             enable_cpu_affinity;    /**< Enable CPU affinity selection, 0 = disable CPU affinity, 1 = use the value in cyclic_cpu_select */
  ddi_em_cpu_select    cyclic_cpu_select;      /**< CPU affinity selection */
  uint32_t             cyclic_thread_enabled;  /**< Is the cyclic thread enabled? */
  uint32_t             network_control_flags;  /**< EtherCAT network control flags, used for partital network support */
} ddi_em_config;

/** @struct ddi_em_instance
 *  @brief Composite EtherCAT master instance
 */
typedef struct {
  ddi_em_config    master_config;              /**< Master configuration information */
  ddi_em_status    master_status;              /**< Master status information */
  ddi_em_slave     slave_info[DDI_EM_MAX_BUS_SLAVES]; /**< Slave information */
  ddi_em_slave     *slave_ptr[DDI_MAX_FUSION_INSTANCES]; /**< Slave pointer to fusion_instances */
  uint8_t          ddi_fusion_count;           /**< Fusion count */
} ddi_em_instance;

/** get_fusion_sdk_handle
 @brief Return the Fusion SDK handle
 @param[in] em_handle The EtherCAT master handle
 @param[in] es_handle The EtherCAT slave handle
 @return ddi_fusion_sdk_handle The Fusion SDK handle
 */
ddi_fusion_sdk_handle get_fusion_sdk_handle(ddi_em_handle em_handle, ddi_es_handle es_handle);

/** get_master_instance
 @brief Return the master instance pointer
 @param em_handle The EtherCAT master handle
 @return ddi_em_instance The pointer to the master instance structure
 */
ddi_em_instance* get_master_instance(ddi_em_handle em_handle);

/** get_slave_instance
 @brief Return the slave instance pointer
 @param em_handle The EtherCAT master handle
 @param slave_handle The EtherCAT slave handle
 @return ddi_em_slave The pointer to the slave instance structure
 */
ddi_em_slave*    get_slave_instance(ddi_em_handle em_handle, ddi_em_handle slave_handle);

/** shutdown_thread_instance
 @brief Shutdown the main cyclic thread instance (if running)
 @param em_handle The EtherCAT master handle
 @return ddi_em_result The result code of the operation, @see ddi_em_result for details
 */
ddi_em_result shutdown_thread_instance(ddi_em_handle em_handle);

#endif // __DDI_EM_H__
