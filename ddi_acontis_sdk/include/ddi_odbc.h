/******************************************************************************
 * (c) Copyright 2020-2021 Digital Dynamics Inc. Scotts Valley CA USA.
 * Unpublished copyright. All rights reserved. Contains proprietary and
 * confidential trade secrets belonging to DDI. Disclosure or release without
 * prior written authorization of DDI is prohibited.
 *****************************************************************************/
#include <sql.h>
#include <sqlext.h>
#include <stdint.h>
#include "ddi_debug.h"

#ifndef __DDI_ODBC__
#define __DDI_ODBC__

#define SQL_MAX_ENTRY_SIZE 256

typedef struct
{
  SQLHENV env;
  SQLHDBC dbc;
  SQLHSTMT stmt;
  bool connected;
} sql_handle_t;

typedef struct
{
  SQLCHAR str[SQL_MAX_ENTRY_SIZE];
} sqlentry_str_t;

typedef struct
{
  float entry;
  SQLLEN len;
} sqlentry_float_t;

typedef struct
{
  SQLCHAR sw_version[SQL_MAX_ENTRY_SIZE];
  SQLCHAR part_number[SQL_MAX_ENTRY_SIZE];
  SQLCHAR fw_version[SQL_MAX_ENTRY_SIZE];
  SQLCHAR serial_number[SQL_MAX_ENTRY_SIZE];
  SQLCHAR ipv4_address[SQL_MAX_ENTRY_SIZE];
  SQLCHAR mac_address[SQL_MAX_ENTRY_SIZE];
  SQLCHAR config_md5[SQL_MAX_ENTRY_SIZE];
  SQLCHAR config_date[SQL_MAX_ENTRY_SIZE];
  SQLCHAR clock[SQL_MAX_ENTRY_SIZE];
  uint32_t event_mask;
  uint32_t event_status;
  uint32_t detect_modules_override;
  float    cpu_fan_status;
  float    cpu_fan_rpm;
  float    cpu_temperature;
} cm_entry_t;

typedef struct
{
  uint32_t port_number;
  SQLCHAR  part_number[SQL_MAX_ENTRY_SIZE];
  SQLCHAR  fw_version[SQL_MAX_ENTRY_SIZE];
  SQLCHAR  serial_number[SQL_MAX_ENTRY_SIZE];
  SQLCHAR  type[SQL_MAX_ENTRY_SIZE];
  SQLCHAR  unit_information[SQL_MAX_ENTRY_SIZE];
  SQLCHAR  slot_cards[SQL_MAX_ENTRY_SIZE];
  uint32_t event_mask;
  uint32_t event_status;
  float    power_fan_status;
  float    slot_fan_status;
  float    power_fan_rpm;
  float    slot_fan_rpm;
} rim_entry_t;

typedef struct
{
  SQLCHAR *test_start_id;
  SQLCHAR event_string[SQL_MAX_ENTRY_SIZE];
} event_entry_t;

typedef struct
{
  SQLCHAR *hw_config_id;
  SQLCHAR test_description[SQL_MAX_ENTRY_SIZE];
  SQLCHAR test_version[SQL_MAX_ENTRY_SIZE];
} test_start_t;

typedef struct
{
  int cm;
  int rim;
  SQLCHAR *test_start_id;
  SQLCHAR *rim_config_id;
  SQLCHAR slot[SQL_MAX_ENTRY_SIZE];
  SQLCHAR connector[SQL_MAX_ENTRY_SIZE];
  SQLCHAR pins[SQL_MAX_ENTRY_SIZE];
  uint32_t actual_value;
  uint32_t expected_value;
  uint32_t deviation;
  SQLCHAR event_string[SQL_MAX_ENTRY_SIZE];
} io_exception_event_t;


typedef struct
{
  SQLCHAR *test_start_id;
  SQLCHAR result[SQL_MAX_ENTRY_SIZE];
  SQLCHAR notes[SQL_MAX_ENTRY_SIZE];
} test_end_t;

// Print out an error message if there is a failure
#define SQL_ASSERT_FAILURE(x) \
  if ((x != SQL_SUCCESS) && (x != SQL_SUCCESS_WITH_INFO) )  \
  { \
    printf("ERROR x %d FILE %s LINE %d\n", x, __FILE__, __LINE__); \
  }

/** ddi_sdk_init_db
 * Initialize a connection to the SQL database
 *
 * @param handle the database handle to write to
 * @return SQLRETURN
 * @see SQLRETURN
 */
SQLRETURN ddi_sdk_init_db(sql_handle_t *handle);
/** fusion_sql_insert_hw_config
 * Insert hw_configuration information into the database. This function should be called first
 * after the database initialization call.
 *
 * @param handle the database handle to write to
 * @return SQLRETURN
 * @see SQLRETURN
 */
SQLRETURN fusion_sql_insert_hw_config(sql_handle_t *handle, SQLCHAR *hw_config_id);
/** ddi_sdk_init_db
 * Insert a test start event to the SQL database.
 *
 * @param handle the database handle to write to
 * @param test_id the test_start structure should be filled out prior to calling this function.
 * @return SQLRETURN
 * @see SQLRETURN
 */
SQLRETURN fusion_sql_insert_test_start(sql_handle_t *handle, test_start_t *test_start, SQLCHAR *test_result_id);
/** fusion_sql_insert_cm
 * Insert cm information into the database.
 *
 * @param handle the database handle to write to.
 * @param cm the cm_entry_t structure should be filled out prior to calling this function.
 * @return SQLRETURN
 * @see SQLRETURN
 */
SQLRETURN fusion_sql_insert_cm(sql_handle_t *handle, cm_entry_t *cm);
/** fusion_sql_insert_rim
 * Insert rim information into the database.
 *
 * @param handle the database handle to write to
 * @param rim the rim_entry_t structure should be filled out prior to calling this function.
 * @return SQLRETURN
 * @see SQLRETURN
 */
SQLRETURN fusion_sql_insert_rim(sql_handle_t *handle, rim_entry_t *rim, SQLCHAR *rim_id);
/** fusion_sql_insert_rim_bridge
 * Insert a rim SQL bridge into the database. This links a RIM and CM. It should be called
 * after the CM and RIM have been adeded.
 *
 * @param handle the database handle to write to
 * @return SQLRETURN
 * @see SQLRETURN
 */
SQLRETURN fusion_sql_insert_rim_bridge(sql_handle_t *handle);
/** fusion_sql_insert_cm_bridge
 * Insert a CM SQL bridge into the database. This links a hw_config and CM. It should be called
 * after the CM and hw_config have been added.
 *
 * @param handle the database handle to write to
 * @return SQLRETURN
 * @see rim_entry_t
 */
SQLRETURN fusion_sql_insert_cm_bridge(sql_handle_t *handle);
/** fusion_sql_insert_test_end
 * Insert a test end into the database.
 * @param handle the database handle to write to
 * @param test_end The test_end_t
 * @return SQLRETURN
 * @see rim_entry_t
 */
SQLRETURN fusion_sql_insert_test_end(sql_handle_t *handle, test_end_t *test_end);
/** fusion_sql_insert_event
 * Insert a generic event into the database.
 * @param handle the database handle to write to
 * @param event The test_end_t structure should be passed to this argument
 * @return SQLRETURN
 * @see test_end_t
 */
SQLRETURN fusion_sql_insert_event(sql_handle_t *handle, event_entry_t *event);
/** fusion_sql_insert_io_event
 * Insert an IO event into the database.
 * @param handle the database handle to write to
 * @param event_entry_t The event_entry_t should be passed to this argument.
 * @return SQLRETURN
 * @see SQLRETURN
 */
SQLRETURN fusion_sql_insert_io_event(sql_handle_t *handle, io_exception_event_t *event);
/** fusion_sql_insert_heartbeat
 * Insert a hearbeat event into the database.
 * @param handle the database handle to write to
 * @param event_entry_t The io_exception_event_t should be passed to this argument.
 * @return SQLRETURN
 * @see SQLRETURN
 */
SQLRETURN fusion_sql_insert_heartbeat(sql_handle_t *handle, SQLCHAR *test_start_id);

#endif
