/******************************************************************************
 * (c) Copyright 2020-2021 Digital Dynamics Inc. Scotts Valley CA USA.
 * Unpublished copyright. All rights reserved. Contains proprietary and
 * confidential trade secrets belonging to DDI. Disclosure or release without
 * prior written authorization of DDI is prohibited.
 *****************************************************************************/

#ifndef DDI_FUSION_INTERFACE_H
#define DDI_FUSION_INTERFACE_H

#include <stdint.h>
#include "ddi_status.h"
#include "ddi_sdk_ecat_master.h"
#include "ddi_odbc.h"

#define DDI_FUSION_MAX_MODULES 255
#define DDI_FUSION_MAX_RIMS    16

typedef struct
{
  uint16_t byte_offset;  //byte offset into process data
  uint8_t  lo;           //lo byte
  uint8_t  hi;           //high byte
  uint32_t mask;         //bitmask
  uint32_t size;         //bitsize
  uint8_t  *pd_input;    // input process data memory
  uint8_t  *pd_output;   // output process data memory
} fusion_pd_desc_t;

/* A fusion slave instance */
typedef struct
{
  uint16_t  *aouts;            /* direct pointer to the aout structure */
  uint16_t  *ains;             /* direct pointer to the ain structure  */
  uint16_t  *dins;             /* direct pointer to the din structure  */
  uint16_t  *douts;            /* direct pointer to the dout structure */
  uint8_t   *pd_input;         /* direct pointer to the input process data for this slave */
  uint8_t   *pd_output;        /* direct pointer to the output process data for this slave */

  uint16_t  dout_count;        /**< The number of DO PDO entries */
  uint16_t  din_count;         /**< The number of DI PDO entries */
  uint16_t  aout_count;        /**< The number of AO PDO entries */
  uint16_t  ain_count;         /**< The number of AI PDO entries */

  uint8_t   is_allocated;      /**< Is this instance free or allocated (0 = free, 1 = allocated) */

  //process data descriptor section
  fusion_pd_desc_t aout_desc[DDI_FUSION_MAX_MODULES];
  fusion_pd_desc_t ain_desc[DDI_FUSION_MAX_MODULES];
  fusion_pd_desc_t din_desc[DDI_FUSION_MAX_MODULES];
  fusion_pd_desc_t dout_desc[DDI_FUSION_MAX_MODULES];

  uint32_t                  eni_count;

  slave_t   *slave;            /* pointer to the slave structure associated with this fusion instance */
 } ddi_fusion_instance_t;

 #define MAX_STR 256

typedef struct {
  uint32_t configured;
  char     part_number[MAX_STR];
  char     fw_version[MAX_STR];
  char     serial_number[MAX_STR];
  char     type[MAX_STR];
  char     unit_information[MAX_STR];
  char     io_cards[24][MAX_STR];
  uint32_t event_mask;
  uint32_t event_status;
  float    power_fan_status;
  float    slot_fan_status;
  float    power_fan_rpm;
  float    slot_fan_rpm;
} ddi_fusion_rim_info_t;

typedef struct {
  char     part_number[MAX_STR];
  char     fw_version[MAX_STR];
  char     serial_number[MAX_STR];
  char     ip_address[MAX_STR];
  char     mac_address[MAX_STR];
  char     config_md5[MAX_STR];
  char     config_date[MAX_STR];
  char     date[MAX_STR];
  uint32_t event_mask;
  uint32_t event_status;
  uint32_t detect_modules_override;
  float    fan_status;
  float    fan_rpm;
  float    temperature_reading;
  ddi_fusion_rim_info_t rim_info[DDI_FUSION_MAX_RIMS];
} ddi_fusion_cm_info_t;

/** ddi_sdk_fusion_open
 * Open the ddi_fusion instance. This will initialize the fusion-specific interface
 * The EtherCAT master should be initialized with a ddi_sdk_init() call prior to calling
 * this function.
 *
 * @param interface the fusion instance to operate on
 * @param cyclic_function the cyclic data function to be called back on
 * @return ddi_status_t
 * @see ddi_status_t
 */
ddi_status_t ddi_sdk_fusion_open(ddi_fusion_instance_t **instance, AcontisCallback *cyclic_callback);

/** ddi_sdk_fusion_open
 * Open the ddi_fusion instance. This will initialize the fusion-specific interface
 * The EtherCAT master should be initialized with a ddi_sdk_init() call prior to calling
 * this function.
 *
 * @param interface the fusion instance to operate on
 * @param cyclic_function the cyclic data function to be called back on
 * @return ddi_status_t
 * @see ddi_status_t
 */
ddi_status_t ddi_sdk_fusion_open(ddi_fusion_instance_t **instance, cyclic_func_t *cyclic_function);

/** ddi_sdk_fusion_close
 * Closes the ddi_fusion instance
 *
 * @param interface the fusion interface to operate on
 * @return ddi_status_t
 * @see ddi_status_t
 */
ddi_status_t ddi_sdk_fusion_close(ddi_fusion_instance_t *instance);

/** ddi_sdk_fusion_set_aout
 * Set an aout channel by index.  This function will populate the AOUT using the ddi_fusion_interface_t process
 * data descriptor. For example, to talk to the first aout channel of a Fusion,
 * pass in the value of 0 to this function as the index
 *
 * The index is 0-based.
 *
 * Example: ddi_sdk_fusion_set_aout (instance, 1, 0x7FFFF) will set the 2nd aout channel to 0x7FFF.
 *
 * @param instance the fusion instance to operate on
 * @param index the aout index to write to
 * @return ddi_status_t
 * @see ddi_status_t
 */
void ddi_sdk_fusion_set_aout    (ddi_fusion_instance_t *instance, uint32_t index, uint16_t  value );

/** ddi_sdk_fusion_get_ain
 * Get an ain channel by index.  This function will retreieve the ain value represented in the channel
 * poitned to by index. For example, to retreive the first channel of a Fusion,
 * pass in the value of 0 to this function.
 *
 * The index is 0-based.
 *
 * Example: ddi_sdk_fusion_get_ain (instance, 1) will return the value of the 2nd AIN channel
 *
 * @param instance the fusion instance to operate on
 * @param index the aout index to write to
 * @return ddi_status_t
 * @see ddi_status_t
 */
uint16_t ddi_sdk_fusion_get_ain (ddi_fusion_instance_t *instance, uint32_t index );

/** ddi_sdk_fusion_set_dout8
 * Set a DOUT 8 bit value.  This function will set the dout value represented in the slot
 * poitned to by index. For example, to retreive DOUT 2 of a Fusion,
 * pass in the value of 1 to this function.
 *
 * The index is 0-based.
 *
 * Example: ddi_sdk_fusion_set_dout8 (instance, 1, 0x3F) will set the 2nd dout to 0x3F
 *
 * @param instance the fusion instance to operate on
 * @param index the dout index to write to
 * @return ddi_status_t
 * @see ddi_status_t
 */
void ddi_sdk_fusion_set_dout8   (ddi_fusion_instance_t *instance, uint32_t index, uint8_t  value );

/** ddi_sdk_fusion_set_dout16
 * Set a DOUT 16 bit value.  This function will set the dout value represented in the slot
 * poitned to by index. For example, to retreive DOUT 2 of a Fusion,
 * pass in the value of 1 to this function.
 *
 * The index is 0-based.
 *
 * Example: ddi_sdk_fusion_set_dout16 (instance, 1, 0x3F30) will set the 2nd dout to 0x3F30
 *
 * @param instance the fusion instance to operate on
 * @param index the dout index to write to
 * @return ddi_status_t
 * @see ddi_status_t
 */
void ddi_sdk_fusion_set_dout16  (ddi_fusion_instance_t *instance, uint32_t index, uint16_t  value );

/** ddi_sdk_fusion_get_din8
 * Get a DIN 8-bit value.  This function will retreive the value represented in the slot
 * poitned to by index. For example, to retreive DIN 5 of a Fusion,
 * pass in the value of 4 to this function.
 *
 * The index is 0-based.
 *
 * Example: ddi_sdk_fusion_get_din8 (instance, 1, 0x3F30) will return 8 bits of the 2nd DIN.
 *
 * @param instance the fusion instance to operate on
 * @param index the dout index to write to
 * @return ddi_status_t
 * @see ddi_status_t
 */
uint8_t ddi_sdk_fusion_get_din8    (ddi_fusion_instance_t *instance, uint32_t index, uint16_t  *value);

/** ddi_sdk_fusion_get_din16
 * Get a DIN 16-bit value.  This function will retreive the value represented in the slot
 * poitned to by index. For example, to retreive DIN 5 of a Fusion,
 * pass in the value of 4 to this function.
 *
 * The index is 0-based.
 *
 * Example: ddi_sdk_fusion_get_din16 (instance, 1, 0x3F30) will set the 2nd dout to 0x3F30
 *
 * @param instance the fusion instance to operate on
 * @param index the dout index to write to
 * @return ddi_status_t
 * @see ddi_status_t
 */
uint16_t ddi_sdk_fusion_get_din16   (ddi_fusion_instance_t *instance, uint32_t index, uint16_t  *value);

/** ddi_sdk_fusion_get_din16
 * Get a DIN 8-bit or 16-bit value.  This function will retreive the value represented in the slot
 * poitned to by index. For example, to retreive DIN 5 of a Fusion,
 * pass in the value of 4 to this function.
 * The length parameter of this function should be checked.
 *
 * The index is 0-based.
 *
 * Example: ddi_sdk_fusion_get_din (instance, 1, 0x3F30) will store the 8 or 16-bits of the 2nd
 * DIN in the return value
 *
 * @param instance the fusion instance to operate on
 * @param index the dout index to write to
 * @param length the amount in bytes that were written
 * @return ddi_status_t
 * @see ddi_status_t
 */
uint16_t ddi_sdk_fusion_get_din (ddi_fusion_instance_t *instance, uint32_t index, uint16_t  *length );

/** ddi_sdk_fusion_log_system_info
 * Log the CM and RIM info to file and a database
 *
 * @param instance the fusion instance to operate on
 * @param filenane the output filename to log to
 * @param rate the amount in bytes that were written to
 * @return void
 */
void ddi_sdk_fusion_log_system_info (ddi_fusion_instance_t *instance, char *filename, uint32_t rate);

/** ddi_sdk_fusion_log_test_startup
 * Log the test start event to file and a database
 *
 * @param instance the fusion instance to operate on
 * @param rate the test rate
 * @param test_version the version of the test in character format
 * @param test_description the description of the test in character format
 * @return void
 */
void ddi_sdk_fusion_log_test_startup (ddi_fusion_instance_t *instance, uint32_t rate, const char * test_version, const char * test_description);

/** ddi_sdk_fusion_get_hw_id
 * Get the hw_config_id associated with the hw_config_table
 *
 * @param none
 * @return the hw_config_id associated with this test run.
 */
SQLCHAR* ddi_sdk_fusion_get_hw_id (void);
/** ddi_sdk_fusion_get_test_start_id
 * Get the test_start_id associated with the test_start
 *
 * @param none
 * @return the test_start_id associated with this test run.
 */
SQLCHAR* ddi_sdk_fusion_get_test_start_id (void);
/** ddi_sdk_fusion_get_rim_id
 * Get the rim_id associated with this RIM index. Eventually (maybe?) there will be more than 1 CM supported
 * in which case this API will need to be modified or overloaded.
 *
 * @param rim the rim to look up the rim id for. This is 0-based.
 * @return the rim_id associated with this test run.
 */
SQLCHAR* ddi_sdk_fusion_get_rim_id (int rim);

#endif // DDI_FUSION_INTERFACE_H

