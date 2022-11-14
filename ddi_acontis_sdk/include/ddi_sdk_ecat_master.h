/******************************************************************************
 * (c) Copyright 2020-2021 Digital Dynamics Inc. Scotts Valley CA USA.
 * Unpublished copyright. All rights reserved. Contains proprietary and
 * confidential trade secrets belonging to DDI. Disclosure or release without
 * prior written authorization of DDI is prohibited.
 *****************************************************************************/

#ifndef DDI_SDK_ECAT_MASTER
#define DDI_SDK_ECAT_MASTER

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

#define DDI_ECAT_INIT   1
#define DDI_ECAT_PREOP  2
#define DDI_ECAT_BOOT   3
#define DDI_ECAT_SAFEOP 4
#define DDI_ECAT_OP     8

#define DDI_SDK_MAX_INSTANCES 16



class AcontisCallback {
public:
    virtual void cyclic_method(void *) = 0;
};

typedef void (cyclic_func_t)(void *);

typedef struct
{
  uint32_t lost_frame_count;      //number of lost frames
} ddi_ecat_master_stats_t;

typedef struct
{
  int master_scan_rate_us;
} ddi_ecat_master_cfg_t;

//This structure is the main Acontis EtherCAT interface for DDI applications


//typedef struct
//{
//  uint8_t                   allocated;          //allocated or not
//  uint32_t                  cylic_time_us;      //cycle time in microseconds
//  EC_T_CFG_SLAVE_INFO       info;
//  EC_T_DWORD                master_instance_id;
//  EC_T_WORD                 eni_count;
//  EC_T_PROCESS_VAR_INFO_EX *eni_entries;        /**< The full list of ENI PDO info entries */
//  uint8_t                  *pd_input;           // input process data memory
//  uint8_t                  *pd_output;          // output process data memory
//  void                     *callback_args;      // callback instance for process data
//  cyclic_func_t            *cyclic_func;        // callback function for process data
//} slave_t;
class slave_t
{
public:
    uint8_t                   allocated;          //allocated or not
    uint32_t                  cylic_time_us;      //cycle time in microseconds
    EC_T_CFG_SLAVE_INFO       info;
    EC_T_DWORD                master_instance_id;
    EC_T_WORD                 eni_count;
    EC_T_PROCESS_VAR_INFO_EX *eni_entries;        /**< The full list of ENI PDO info entries */
    uint8_t                  *pd_input;           // input process data memory
    uint8_t                  *pd_output;          // output process data memory
    void                     *callback_args;      // callback instance for process data
    cyclic_func_t            *cyclic_func = nullptr;   // callback function for process data
    AcontisCallback          *cyclic_object = nullptr; // callback object for process data
};

//other files can access the slave structure
extern slave_t slave[DDI_SDK_MAX_INSTANCES];

/** ddi_sdk_ecat_init
 * Initialize the DDI ethercat sdk.  This function is called from ddi_sdk_init but 
 * may be called directly if the ddi_sdk_common functionality is not used
 *
 * @param bus_cycle_us the microsecond cyclic data rate
 * @param eni_filename the filename of the ENI
 * @param iface_name the interface name to connect to
 * @return ddi_status_t
 * @see ddi_status_t
 */
ddi_status_t ddi_sdk_ecat_init(uint32_t bus_cycle_us, const char *eni_filename, const char *iface_name);

/** ddi_sdk_ecat_open
 * Open a ddi_sdk_ecat_open.  Should be called from an interface layer
 *
 * @param slave the slave structure pointer
 * @return ddi_status_t
 * @see ddi_status_t
 */
ddi_status_t ddi_sdk_ecat_open(slave_t* slave);

/** ddi_sdk_ecat_set_master_state
 * Sets the state of the ethercat master
 *
 * @param state the state to switch state to
 * @return ddi_status_t
 * @see ddi_status_t
 */
ddi_status_t ddi_sdk_ecat_set_master_state(uint32_t state);

/** ddi_sdk_ecat_get_master state
 * Gets the state of the ethercat master
 *
 * @param state the state to retrieve the current master state to
 * @return ddi_status_t
 * @see ddi_status_t
 */
ddi_status_t ddi_sdk_ecat_get_master_state(uint16_t *state);

/** ddi_sdk_ecat_configure_master
 * Reconfigure the ethercat master with a new eni
 * If sucessful, the master will be in INIT state
 *
 * @param eni the new eni to configure the master with
 * @return ddi_status_t
 * @see ddi_status_t
 */
ddi_status_t ddi_sdk_ecat_configure_master( const char *eni_filename);

/** ddi_sdk_ecat_set_state
 * Sets the state of the ethercat slave
 *
 * @param slave the slave structure pointer
 * @param state the state to switch state to
 * @return ddi_status_t
 * @see ddi_status_t
 */
ddi_status_t ddi_sdk_ecat_set_slave_state(slave_t *slave, uint16_t state);

/** ddi_sdk_ecat_set_state
 * Gets the state of the ethercat slave
 *
 * @param slave the slave structure pointer
 * @param state the state to retrieve the current master state to
 * @return ddi_status_t
 * @see ddi_status_t
 */
ddi_status_t ddi_sdk_ecat_get_slave_state(slave_t *slave, uint16_t *current_state, uint16_t *requested_state);

/** ddi_sdk_get_slave_fixed_addr
 * Get the ethercat address of the slave instance
 *
 * @param eni the new eni to configure the master with
 * @return ddi_status_t
 * @see ddi_status_t
 */
ddi_status_t ddi_sdk_get_slave_fixed_addr ( slave_t         *slave,
     uint32_t             vendor_id,
     uint32_t             product_id,
     uint16_t*           pwPhysAddr);

/** ddi_sdk_ecat_get_master_stats
 * Get the ethercat master statistics
 * Currently this structure has the number of lost frames
 *
 * @param stats the ethercat master stat structure
 * @return ddi_status_t
 * @see ddi_status_t
 * @see ddi_ecat_master_stats_t
 */
ddi_status_t ddi_sdk_ecat_get_master_stats ( ddi_ecat_master_stats_t * stats);

/** ddi_sdk_ecat_get_master_stats
 * Get the ethercat master NIC adapater MAC address
 * This function returns the MAC address in nn-nn-nn-nn-nn-nn format
 *
 * @param stats MAC address structure in characters
 * @return ddi_status_t
 * @see ddi_status_t
 * @see ddi_mac_address_t
 */
ddi_status_t ddi_sdk_ecat_get_mac(char *mac_address);

/** ddi_sdk_ecat_get_master_license_status
 * Get the ethercat license status for the NIC in use by the master
 *
 * @param void
 * @return 0 - not licensed, 1 - licensed
 */
uint8_t ddi_sdk_ecat_get_master_license_status ( void );

/** ddi_sdk_ecat_set_license_file
 * Set the ethercat license filename. This call should be made before
 * the ddi_sdk_ecat_init or ddi_sdk_init calls are made
 *
 * @param filename the filename where the Acontis license is kept
 * @return 0 - not licensed, 1 - licensed
 */
ddi_status_t ddi_sdk_ecat_set_license_file(char *filename);

#endif // DDI_SDK_ECAT_MASTER

