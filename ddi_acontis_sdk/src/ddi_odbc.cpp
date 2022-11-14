/******************************************************************************
 * (c) Copyright 2021 Digital Dynamics Inc. Scotts Valley CA USA.
 * Unpublished copyright. All rights reserved. Contains proprietary and
 * confidential trade secrets belonging to DDI. Disclosure or release without
 * prior written authorization of DDI is prohibited.
 *****************************************************************************/
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sql.h>
#include <sqlext.h>
#include "ddi_odbc.h"
#include "ddi_debug.h"

// Helper function to bind a parameter
static int bind_sql (SQLHSTMT hstmt, SQLUSMALLINT column, SQLSMALLINT c_type, SQLSMALLINT sql_type, void *value, SQLLEN *size)
{
  int ret;
  ret = SQLBindParameter(hstmt, column, SQL_PARAM_INPUT, \
    c_type, sql_type, SQL_MAX_ENTRY_SIZE, 0, \
    value, SQL_MAX_ENTRY_SIZE, size); \
  return ret;
}

// Perform the prepare, execute, cancel and free instructions
static int execute_insert (SQLHSTMT hstmt, SQLCHAR *sql_statement_insert)
{
  int ret;
  // Prepare Statement
  ret = SQLPrepare(hstmt, (SQLCHAR*) sql_statement_insert, SQL_NTS);
  SQL_ASSERT_FAILURE(ret);

  ret = SQLExecute(hstmt);
  SQL_ASSERT_FAILURE(ret);

  ret = SQLCancel(hstmt);
  SQL_ASSERT_FAILURE(ret);

  ret = SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
  SQL_ASSERT_FAILURE(ret);

  return ret;
}

// Perform the prepare, execute, cancel and free instructions with a return code
static int execute_insert_with_return (SQLHSTMT hstmt, SQLCHAR *sql_statement_insert, SQLCHAR *ret_str)
{
  char buf[256];
  int ret;
  SQLLEN last_entry_size;
  // Prepare Statement
  ret = SQLPrepare(hstmt, (SQLCHAR*) sql_statement_insert, SQL_NTS);
  SQL_ASSERT_FAILURE(ret);

  ret = SQLExecute(hstmt);
  SQL_ASSERT_FAILURE(ret);

  memset(buf,0,256);
  // Bind column 1
  ret = SQLBindCol(hstmt, 1, SQL_C_CHAR, buf, 1024, &last_entry_size);
  SQL_ASSERT_FAILURE(ret);
  ret = SQLFetch(hstmt);
  SQL_ASSERT_FAILURE(ret);

  ret = SQLCancel(hstmt);
  SQL_ASSERT_FAILURE(ret);

  ret = SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
  SQL_ASSERT_FAILURE(ret);

  if ( last_entry_size > 0 )
  {
    if ( ret_str != NULL )
    {
      strncpy((char*)ret_str,buf, last_entry_size);
    }
  }

  return 0;
}

// Returns the last entry of a column
static SQLRETURN get_last_entry(sql_handle_t *handle, char *table_name, char *id_name, char *return_str, SQLLEN *size)
{
  SQLCHAR last_entry[1024];
  SQLLEN last_entry_size;
  SQLCHAR buf[1024] = {0};
  int ret;
  SQLHSTMT hstmt = SQL_NULL_HSTMT;  	// Statement handle
  snprintf((char*)buf,1024,"SELECT * FROM %s WHERE %s=(SELECT max(%s) FROM %s);", table_name, id_name, id_name, table_name);
  ret = SQLAllocHandle(SQL_HANDLE_STMT, handle->dbc, &hstmt);
  SQL_ASSERT_FAILURE(ret);
  ret = SQLExecDirect(hstmt, buf, SQL_NTS);
  SQL_ASSERT_FAILURE(ret);
  // Bind column 1
  ret = SQLBindCol(hstmt, 1, SQL_C_CHAR, last_entry, 1024, &last_entry_size);
  SQL_ASSERT_FAILURE(ret);
  ret = SQLFetch(hstmt);
  SQL_ASSERT_FAILURE(ret);
  SQLCancel(hstmt);
  SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
  if ( return_str != NULL)
  {
    strncpy(return_str, (char*)last_entry, last_entry_size);
    *size = last_entry_size;
    return ret;
  }
  return SQL_ERROR;
}

// Returns the last entry of a rim, sorted by RIM ID
static SQLRETURN get_last_entry_rim(sql_handle_t *handle, char *table_name, int port_number, char *id_name, char *return_str, SQLLEN *size)
{
  SQLCHAR last_entry[1024];
  SQLLEN last_entry_size;
  SQLCHAR buf[1024];
  memset(last_entry,0,1024);
  int ret;
  SQLHSTMT hstmt = SQL_NULL_HSTMT;  	// Statement handle
  snprintf((char*)buf,1024,"SELECT TOP 1 * FROM rim WHERE port_number=%d ORDER BY rim_id DESC;", port_number+1);
  ret = SQLAllocHandle(SQL_HANDLE_STMT, handle->dbc, &hstmt);
  SQL_ASSERT_FAILURE(ret);
  ret = SQLExecDirect(hstmt, buf, SQL_NTS);
  SQL_ASSERT_FAILURE(ret);
  // Bind column 1
  ret = SQLBindCol(hstmt, 1, SQL_C_CHAR, last_entry, 1024, &last_entry_size);
  SQL_ASSERT_FAILURE(ret);
  ret = SQLFetch(hstmt);
  SQL_ASSERT_FAILURE(ret);
  SQLCancel(hstmt);
  SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
  if ( return_str != NULL)
  {
    strncpy(return_str, (char*)last_entry, last_entry_size);
    *size = last_entry_size;
    return ret;
  }
  return SQL_ERROR;
}

// Initialize the connection to the database
SQLRETURN ddi_sdk_init_db(sql_handle_t *handle)
{
  SQLRETURN ret;
  SQLCHAR connect_str[] = "DSN=CRAMDB;UID=fusion;PWD=Fusion.IO";

  memset(handle, 0, sizeof(sql_handle_t));
  ret = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &handle->env);
  SQL_ASSERT_FAILURE(ret);

  ret = SQLSetEnvAttr(handle->env, SQL_ATTR_ODBC_VERSION, (void *) SQL_OV_ODBC3, 0);
  SQL_ASSERT_FAILURE(ret);

  ret = SQLAllocHandle(SQL_HANDLE_DBC, handle->env, &handle->dbc);
  SQL_ASSERT_FAILURE(ret);

  ret = SQLDriverConnect(handle->dbc, NULL, connect_str, sizeof(connect_str), NULL, 0, NULL, SQL_DRIVER_COMPLETE);
  SQL_ASSERT_FAILURE(ret);

  if ((ret != SQL_SUCCESS) && (ret != SQL_SUCCESS_WITH_INFO) )
    handle->connected = false;
  else
    handle->connected = true;

  return ret;
}

// Insert an entry to the hardware config
SQLRETURN fusion_sql_insert_hw_config(sql_handle_t *handle, SQLCHAR *hw_config_id)
{
  SQLRETURN ret;
  SQLHSTMT hstmt = SQL_NULL_HSTMT;  	// Statement handle
  SQLCHAR sql_statement_insert[] = "INSERT INTO hw_config OUTPUT Inserted.hw_config_id VALUES (DEFAULT, DEFAULT)";

  if ( handle->connected == false ) // Prevent insertion if the database is not connected
    return SQL_ERROR;

  // Allocate Statement Handle
  ret = SQLAllocHandle(SQL_HANDLE_STMT, handle->dbc, &hstmt);
  SQL_ASSERT_FAILURE(ret);

  ret = execute_insert_with_return(hstmt, sql_statement_insert, hw_config_id);
  SQL_ASSERT_FAILURE(ret);

  return ret;
}

// Insert a test start event
SQLRETURN fusion_sql_insert_test_start(sql_handle_t *handle, test_start_t *test_start, SQLCHAR *test_result_id)
{
  SQLRETURN ret;
  SQLHSTMT  hstmt = SQL_NULL_HSTMT;  	  // Statement handle
  char      hw_config_entry[4096];      // ID of the hw_config, used to populate the foreign key
  SQLLEN    hw_size = 0;
  SQLLEN    test_desc_size = SQL_NTS;
  SQLLEN    event_desc_size = SQL_NTS;
  SQLLEN    sw_version_size = SQL_NTS;
  SQLCHAR   sql_statement_insert[4096];

  if ( handle->connected == false ) // Prevent insertion if the database is not connected
    return SQL_ERROR;

  snprintf((char*)sql_statement_insert, 4096, "INSERT INTO test_result OUTPUT Inserted.test_result_id \
    VALUES (DEFAULT, DEFAULT, ?, ?, '%s', 'NULL', 'NULL');", test_start->hw_config_id);

  // Allocate statement Handle
  ret = SQLAllocHandle(SQL_HANDLE_STMT, handle->dbc, &hstmt);
  SQL_ASSERT_FAILURE(ret);

  // Populate the columns to write to
  test_desc_size = strlen((char *)test_start->test_description);
  ret = bind_sql(hstmt, 1, SQL_C_CHAR, SQL_CHAR, test_start->test_description, &test_desc_size);
  SQL_ASSERT_FAILURE(ret);
  event_desc_size = strlen((char *)test_start->test_version);
  ret = bind_sql(hstmt, 2, SQL_C_CHAR, SQL_CHAR, test_start->test_version, &event_desc_size);
  SQL_ASSERT_FAILURE(ret);

  // Insert message
  ret = execute_insert_with_return(hstmt, sql_statement_insert, test_result_id);
  SQL_ASSERT_FAILURE(ret);

  return ret;
}

// Insert an I/O event
SQLRETURN fusion_sql_insert_io_event(sql_handle_t *handle, io_exception_event_t *event)
{
  SQLRETURN ret;
  SQLHSTMT  hstmt = SQL_NULL_HSTMT;  	// Statement handle
  char      test_entry[4096];
  SQLLEN    test_size = 0;
  char      rim_entry[4096];
  SQLLEN    rim_size = 0;
  SQLLEN    size[9];
  SQLCHAR sql_statement_insert[4096];

  if ( handle->connected == false ) // Prevent insertion if the database is not connected
    return SQL_ERROR;

  snprintf((char*)sql_statement_insert, 4096, "INSERT INTO io_event VALUES (DEFAULT, DEFAULT, '%s', ?, ?, ?, ?, ?, ?, ?, '%s');", event->rim_config_id, event->test_start_id);

  // Allocate Statement Handle
  ret = SQLAllocHandle(SQL_HANDLE_STMT, handle->dbc, &hstmt);
  SQL_ASSERT_FAILURE(ret);

  size[0] = strlen((char *)event->slot);
  // Setup parameters for write
  ret = bind_sql(hstmt, 1, SQL_C_CHAR, SQL_CHAR, event->slot, &size[0]);
  SQL_ASSERT_FAILURE(ret);
  size[1] = strlen((char *)event->connector);
  ret = bind_sql(hstmt, 2,  SQL_C_CHAR, SQL_CHAR, event->connector, &size[1]);
  SQL_ASSERT_FAILURE(ret);
  size[2] = strlen((char *)event->pins);
  ret = bind_sql(hstmt, 3, SQL_C_CHAR, SQL_CHAR, event->pins, &size[2]);
  SQL_ASSERT_FAILURE(ret);
  size[3] = sizeof(uint32_t);
  ret = bind_sql(hstmt, 4, SQL_INTEGER, SQL_INTEGER,  &event->actual_value, &size[3]);
  SQL_ASSERT_FAILURE(ret);
  size[4] = sizeof(uint32_t);
  ret = bind_sql(hstmt, 5, SQL_INTEGER, SQL_INTEGER, &event->expected_value, &size[4]);
  SQL_ASSERT_FAILURE(ret);
  size[5] = sizeof(uint32_t);
  ret = bind_sql(hstmt, 6, SQL_INTEGER, SQL_INTEGER,  &event->deviation, &size[5]);
  SQL_ASSERT_FAILURE(ret);
  size[6] = strlen((char *)event->event_string);
  ret = bind_sql(hstmt, 7, SQL_C_CHAR, SQL_CHAR, &event->event_string, &size[6]);
  SQL_ASSERT_FAILURE(ret);

  // Update the database
  ret = execute_insert(hstmt, sql_statement_insert);
  SQL_ASSERT_FAILURE(ret);

  return ret;
}

// Insert a heartbeat event which contains periodic proof that the test is still functional
SQLRETURN fusion_sql_insert_heartbeat(sql_handle_t *handle, SQLCHAR *test_start_id)
{
  SQLRETURN ret;
  SQLHSTMT  hstmt = SQL_NULL_HSTMT;  	// Statement handle
  SQLLEN    default_str_len =  SQL_DEFAULT_PARAM;
  char      test_entry[4096];
  SQLLEN    test_size = 0;
  SQLCHAR sql_statement_insert[4096];

  if ( handle->connected == false ) // Prevent insertion if the database is not connected
    return SQL_ERROR;

  snprintf((char*)sql_statement_insert, 4096, "INSERT INTO heartbeat VALUES (DEFAULT, DEFAULT, '%s');", (char *)test_start_id);

  // Allocate Statement Handle
  ret = SQLAllocHandle(SQL_HANDLE_STMT, handle->dbc, &hstmt);
  SQL_ASSERT_FAILURE(ret);

   // Update the database
  ret = execute_insert(hstmt, sql_statement_insert);
  SQL_ASSERT_FAILURE(ret);

  return ret;
}

SQLRETURN fusion_sql_insert_event(sql_handle_t *handle, event_entry_t *event)
{
  SQLRETURN ret;
  SQLHSTMT  hstmt = SQL_NULL_HSTMT;  	// Statement handle
  SQLLEN    default_str_len =  SQL_DEFAULT_PARAM;
  char      test_entry[4096];
  SQLLEN    test_size = 0;
  SQLCHAR sql_statement_insert[4096];

  if ( handle->connected == false ) // Prevent insertion if the database is not connected
    return SQL_ERROR;

  snprintf((char*)sql_statement_insert, 4096, "INSERT INTO event VALUES (DEFAULT, DEFAULT, '%s', ?);", event->test_start_id);

  // Allocate Statement Handle
  ret = SQLAllocHandle(SQL_HANDLE_STMT, handle->dbc, &hstmt);
  SQL_ASSERT_FAILURE(ret);

  SQLLEN  test_desc_size = SQL_NTS;
  SQLLEN  event_desc_size = SQL_NTS;
  test_desc_size = strlen((char *)event->event_string);

  // Bind Parameters to all fields
  ret = bind_sql(hstmt, 1,  SQL_C_CHAR, SQL_CHAR, event->event_string, &test_desc_size);
  SQL_ASSERT_FAILURE(ret);
  // Insert the record
  ret = execute_insert(hstmt, sql_statement_insert);
  SQL_ASSERT_FAILURE(ret);

  return ret;
}

SQLRETURN fusion_sql_insert_test_end(sql_handle_t *handle, test_end_t *test_end)
{
  SQLRETURN ret;
  SQLHSTMT  hstmt = SQL_NULL_HSTMT;  	// Statement handle
  SQLLEN    default_str_len =  SQL_DEFAULT_PARAM;
  char      test_entry[4096];
  SQLLEN    results_size = 0, test_size, notes_size;
  SQLCHAR sql_statement_insert[4096];

  if ( handle->connected == false ) // Prevent insertion if the database is not connected
    return SQL_ERROR;

  snprintf((char*)sql_statement_insert, 4096, "UPDATE test_result SET result = '%s', notes = '%s' where test_result_id = '%s'", test_end->result, test_end->notes, test_end->test_start_id);

  // Allocate Statement Handle
  ret = SQLAllocHandle(SQL_HANDLE_STMT, handle->dbc, &hstmt);
  SQL_ASSERT_FAILURE(ret);

  // Insert the record
  ret = execute_insert(hstmt, sql_statement_insert);
  SQL_ASSERT_FAILURE(ret);

  return ret;
}

SQLRETURN fusion_sql_insert_cm_bridge(sql_handle_t *handle)
{
  SQLRETURN ret;
  SQLHSTMT  hstmt = SQL_NULL_HSTMT;  	// Statement handle
  SQLLEN    default_str_len =  SQL_DEFAULT_PARAM;
  char      cm_id[4096];
  SQLLEN    cm_id_size = 0;
  char      hw_config_id[4096];
  SQLLEN    hw_config_size = 0;
  SQLCHAR sql_statement_insert[4096];

  if ( handle->connected == false ) // Prevent insertion if the database is not connected
    return SQL_ERROR;
  get_last_entry(handle, (char*)"cm",(char *)"cm_id", cm_id, &cm_id_size);
  get_last_entry(handle, (char *)"hw_config",(char *)"hw_config_id", hw_config_id, &hw_config_size);
  snprintf((char*)sql_statement_insert, 4096, "INSERT INTO cm_bridge VALUES (?, ?);");

  // Allocate Statement Handle
  ret = SQLAllocHandle(SQL_HANDLE_STMT, handle->dbc, &hstmt);
  SQL_ASSERT_FAILURE(ret);

  // Create the record
  ret = bind_sql(hstmt, 1, SQL_C_CHAR, SQL_CHAR, hw_config_id, &hw_config_size);
  SQL_ASSERT_FAILURE(ret);
  ret = bind_sql(hstmt, 2, SQL_C_CHAR, SQL_CHAR, cm_id, &cm_id_size);
  SQL_ASSERT_FAILURE(ret);

  // Insert the record
  ret = execute_insert(hstmt, sql_statement_insert);
  SQL_ASSERT_FAILURE(ret);
  return ret;
}

SQLRETURN fusion_sql_insert_rim_bridge(sql_handle_t *handle)
{
  SQLRETURN ret;
  SQLHSTMT  hstmt = SQL_NULL_HSTMT;  	// Statement handle
  SQLLEN    default_str_len =  SQL_DEFAULT_PARAM;
  char      cm_id[4096];
  SQLLEN    cm_id_size = 0;
  char      rim_id[4096];
  SQLLEN    rim_id_size = 0;
  // 15 Column table
  SQLCHAR sql_statement_insert[4096];
  if ( handle->connected == false ) // Prevent insertion if the database is not connected
    return SQL_ERROR;

  get_last_entry(handle, (char *)"cm", (char *)"cm_id", cm_id, &cm_id_size);
  get_last_entry(handle, (char *)"rim", (char *)"rim_id", rim_id, &rim_id_size);
  snprintf((char*)sql_statement_insert, 4096, "INSERT INTO rim_bridge VALUES (?, ?);");

  // Allocate Statement Handle
  ret = SQLAllocHandle(SQL_HANDLE_STMT, handle->dbc, &hstmt);
  SQL_ASSERT_FAILURE(ret);

  // Bind Parameters to all fields
  ret = bind_sql(hstmt, 1, SQL_C_CHAR, SQL_CHAR, cm_id, &cm_id_size);
  SQL_ASSERT_FAILURE(ret);
  ret = bind_sql(hstmt, 2,  SQL_C_CHAR, SQL_CHAR, rim_id, &rim_id_size);
  SQL_ASSERT_FAILURE(ret);

  // Insert the record
  ret = execute_insert(hstmt, sql_statement_insert);
  SQL_ASSERT_FAILURE(ret);
  return ret;
}

#define RIM_PARAMETERS 8 // Amount of size parameters to support

SQLRETURN fusion_sql_insert_rim(sql_handle_t *handle, rim_entry_t *rim, SQLCHAR *rim_id)
{
  SQLRETURN ret;
  SQLHSTMT hstmt = SQL_NULL_HSTMT;  	// Statement handle
  SQLHDBC  hdbc  = SQL_NULL_HDBC;   	// Connection handle
  // 15 Column table
  SQLCHAR sql_statement_insert[4096] = "INSERT INTO rim OUTPUT Inserted.rim_id  VALUES (DEFAULT, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);";

  if ( handle->connected == false ) // Prevent insertion if the database is not connected
    return SQL_ERROR;

  // Allocate Statement Handle
  ret = SQLAllocHandle(SQL_HANDLE_STMT, handle->dbc, &hstmt);
  SQL_ASSERT_FAILURE(ret);

  // Setup the table for write
  SQLLEN size[RIM_PARAMETERS];
  int count = 0;
  char *data = (char *)rim->part_number;
  count++;
  SQL_ASSERT_FAILURE(ret);
  ret = bind_sql(hstmt, count,SQL_INTEGER, SQL_INTEGER, &rim->port_number, &size[0]);
  for ( count = 2; count <= 7; count++)
  {
    size[count-1] = strlen(data);
    // Bind Parameters to all fields
    ret = bind_sql(hstmt, count, SQL_C_CHAR, SQL_CHAR, data, &size[count-1]);
    SQL_ASSERT_FAILURE(ret);
    data += SQL_MAX_ENTRY_SIZE;
  }
  ret = bind_sql(hstmt, count, SQL_INTEGER, SQL_INTEGER, &rim->event_mask, &size[8]);
  count++;
  SQL_ASSERT_FAILURE(ret);
  ret = bind_sql(hstmt, count, SQL_INTEGER, SQL_INTEGER, &rim->event_status, &size[8]);
  count++;
  SQL_ASSERT_FAILURE(ret);
  ret = bind_sql(hstmt, count, SQL_C_FLOAT, SQL_FLOAT, &rim->slot_fan_status, &size[8]);
  count++;
  SQL_ASSERT_FAILURE(ret);
  ret = bind_sql(hstmt, count, SQL_C_FLOAT, SQL_FLOAT, &rim->power_fan_status, &size[8]);
  count++;
  SQL_ASSERT_FAILURE(ret);
  ret = bind_sql(hstmt, count, SQL_C_FLOAT, SQL_FLOAT, &rim->slot_fan_status, &size[8]);
  count++;
  SQL_ASSERT_FAILURE(ret);
  ret = bind_sql(hstmt, count,SQL_C_FLOAT, SQL_FLOAT, &rim->power_fan_rpm, &size[8]);
  count++;
  SQL_ASSERT_FAILURE(ret);
  ret = bind_sql(hstmt, count,  SQL_C_FLOAT, SQL_FLOAT, &rim->slot_fan_rpm, &size[8]);
  SQL_ASSERT_FAILURE(ret);

  // Insert the record
  ret = execute_insert_with_return(hstmt, sql_statement_insert, rim_id);
  SQL_ASSERT_FAILURE(ret);

  return ret;
}

#define CM_PARAMETERS 16 // 16 fields
SQLRETURN fusion_sql_insert_cm(sql_handle_t *handle, cm_entry_t *cm)
{
  SQLRETURN ret;
  SQLHSTMT hstmt = SQL_NULL_HSTMT;  	// Statement handle
  SQLHDBC  hdbc  = SQL_NULL_HDBC;   	// Connection handle

  // 15 Column table
  SQLCHAR sql_statement_insert[4096] = "INSERT INTO cm VALUES (DEFAULT, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";

  if ( handle->connected == false) // Prevent insertion if the database is not connected
    return SQL_ERROR;

  // Allocate Statement Handle
  ret = SQLAllocHandle(SQL_HANDLE_STMT, handle->dbc, &hstmt);
  SQL_ASSERT_FAILURE(ret);

  // Setup the table for write
  SQLLEN size[CM_PARAMETERS];
  int count = 0;
  char *data = (char *)cm;
  SQLLEN numeric_size = 4;
  for ( count = 1; count <= 9; count++)
  {
    size[count-1] = strlen(data);
    // Bind Parameters to all fields
    ret = bind_sql(hstmt, count, SQL_C_CHAR, SQL_CHAR,  data, &size[count-1]);
    SQL_ASSERT_FAILURE(ret);
    data += SQL_MAX_ENTRY_SIZE;
  }
  ret = bind_sql(hstmt, count, SQL_INTEGER, SQL_INTEGER, &cm->event_mask, &size[9]);
  SQL_ASSERT_FAILURE(ret);
  count++;
  ret = bind_sql(hstmt, count, SQL_INTEGER, SQL_INTEGER, &cm->event_status, &size[10]);
  count++;
  SQL_ASSERT_FAILURE(ret);
  ret = bind_sql(hstmt, count, SQL_INTEGER, SQL_INTEGER, &cm->detect_modules_override, &size[11]);
  count++;
  SQL_ASSERT_FAILURE(ret);
  ret = bind_sql(hstmt, count, SQL_C_FLOAT, SQL_FLOAT,  &cm->cpu_fan_status, &size[12]);
  count++;
  SQL_ASSERT_FAILURE(ret);
  ret = bind_sql(hstmt, count, SQL_C_FLOAT, SQL_FLOAT, &cm->cpu_fan_rpm, &size[13]);
  count++;
  SQL_ASSERT_FAILURE(ret);
  ret = bind_sql(hstmt, count, SQL_C_FLOAT, SQL_FLOAT, &cm->cpu_temperature, &size[14]);
  SQL_ASSERT_FAILURE(ret);
  count++;

  // Insert the record
  ret = execute_insert(hstmt, sql_statement_insert);
  SQL_ASSERT_FAILURE(ret);
  return ret;
}

#if SELF_TEST
int main (void)
{
  SQLCHAR hw_config[256];
  SQLCHAR test_start[256];
  SQLCHAR rim_id[5][2156];
  int ret;
  sql_handle_t handle;
  ret=ddi_sdk_init_db(&handle);

  printf("****** Inserting hw_config \n");
  fusion_sql_insert_hw_config(&handle, hw_config);
  test_start_t start;
  strcpy((char*)start.test_description,(char*)"Test description");
  strcpy((char *)start.test_version,(char*)"1.0.1");
  start.hw_config_id = hw_config;
  printf("****** Inserting test_start \n");
  fusion_sql_insert_test_start(&handle, &start, test_start);
  event_entry_t event;
  strcpy((char*)event.event_string,(char*)"This an event \n");
  event.test_start_id = test_start;
  printf("****** Inserting event \n");
  fusion_sql_insert_event(&handle, &event);

  rim_entry_t rim;
  rim.port_number = 5;
  strcpy((char *)rim.part_number,(char*)"12-200214-00");
  strcpy((char *)rim.fw_version,(char*)"1.09.6");
  strcpy((char *)rim.serial_number,(char*)"12-200201-00-0001");
  strcpy((char *)rim.type,(char*)"24-slot");
  strcpy((char *)rim.unit_information,(char*)"LOW SIDE");
  strcpy((char *)rim.slot_cards,(char*)"f081,f082");
  rim.power_fan_status = 1.0f;
  rim.slot_fan_status = 703.0f;
  rim.power_fan_rpm = 1.0f;
  rim.slot_fan_rpm = 703.0f;

  printf("****** Inserting rim \n");
  fusion_sql_insert_rim(&handle, &rim,rim_id[0]);
  //ret=ddi_sdk_fusion_insert_hw_config(&handle);
  cm_entry_t cm;
#if 0
  strcpy((char *)cm.part_number,(char*)"12-200201-00");
  strcpy((char *)cm.fw_version,(char*)"1.09.6");
  strcpy((char *)cm.serial_number,(char*)"12-200201-00-0001");
  strcpy((char *)cm.ipv4_address,(char*)"192.168.9.50");
  strcpy((char *)cm.mac_address,(char*)"ab:cd:ef:gh:ij");
  strcpy((char *)cm.config_md5,(char*)"abcdefgh");
  strcpy((char *)cm.config_date,(char*)"2021-10-01");
  strcpy((char *)cm.clock,(char*)"12:34");
#endif
  cm.event_mask = 0xffffffff;
  cm.event_status = 0x0;
  cm.detect_modules_override = 1;
  cm.cpu_fan_status = 1.0f;
  cm.cpu_fan_rpm = 7203.0f;
  cm.cpu_temperature = 72.3f;

  printf("****** Inserting cm \n");
  fusion_sql_insert_cm(&handle, &cm);

  printf("****** Inserting cm bridge\n");
  fusion_sql_insert_cm_bridge(&handle);

  printf("****** Inserting rim bridge\n");
  fusion_sql_insert_rim_bridge(&handle);

  io_exception_event_t io_event;
  strcpy((char*)io_event.slot,(char*)"S10");
  strcpy((char*)io_event.connector,(char*)"J1");
  strcpy((char*)io_event.pins,(char *)"1,3");
  strcpy((char*)io_event.event_string,(char *)"Analog testing...");
  io_event.actual_value = 0x7fff;
  io_event.expected_value = 0;
  io_event.deviation = 0x7fff;
  io_event.test_start_id = test_start;

  printf("****** Inserting io event\n");
  fusion_sql_insert_io_event(&handle, &io_event);

  test_end_t test_end;
  test_end.test_start_id = test_start;
  strcpy((char*)test_end.result,(char*)"PASS");

  printf("****** Inserting test end\n");
  fusion_sql_insert_test_end(&handle, &test_end);

  printf("****** Inserting heartbeat\n");
  fusion_sql_insert_heartbeat(&handle, test_start);

}
#endif
