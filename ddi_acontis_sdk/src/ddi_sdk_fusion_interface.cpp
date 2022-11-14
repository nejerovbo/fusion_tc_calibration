/******************************************************************************
 * (c) Copyright 2020-2021 Digital Dynamics Inc. Scotts Valley CA USA.
 * Unpublished copyright. All rights reserved. Contains proprietary and
 * confidential trade secrets belonging to DDI. Disclosure or release without
 * prior written authorization of DDI is prohibited.
 *****************************************************************************/
#include "ddi_sdk_fusion_interface.h"
#include "ddi_odbc.h"

/*
 * This file provides a fusion-centric binding to the acontis library
 */

//forward declarations
static uint32_t ddi_sdk_ecat_scan_slave(ddi_fusion_instance_t *instance);

ddi_fusion_instance_t fusion_instances[DDI_SDK_MAX_INSTANCES] = {0};

static const int out_name_suffix_len = sizeof(".Output mapping 0");
static const int in_name_suffix_len = sizeof(".Input mapping 0");

#define RIM_ONLINE 2

sql_handle_t db_handle;

SQLCHAR fusion_db_hw_config_id[256] = { 0 };
SQLCHAR fusion_db_test_start_id[256] = { 0 } ;
SQLCHAR fusion_db_rim_id[DDI_FUSION_MAX_RIMS][256] = { 0 };

SQLCHAR* ddi_sdk_fusion_get_hw_id (void)
{
  return fusion_db_hw_config_id;
}
SQLCHAR* ddi_sdk_fusion_get_test_start_id (void)
{
  return fusion_db_test_start_id;
}
SQLCHAR* ddi_sdk_fusion_get_rim_id (int rim_id)
{
  return fusion_db_rim_id[rim_id];
}

// Provides a function call to a test startup event
void ddi_sdk_fusion_log_test_startup (ddi_fusion_instance_t *instance, uint32_t rate, const char * test_version, const char * test_description)
{
  char buf[256];
  int ret;
  test_start_t test_start;
  strncpy((char*)test_start.test_version,  test_version, SQL_MAX_ENTRY_SIZE);
  strncpy((char*)test_start.test_description,  test_description, SQL_MAX_ENTRY_SIZE);
  // Generate the hardware configuration entry.  This is generated once per startup.
  ret = fusion_sql_insert_hw_config(&db_handle, fusion_db_hw_config_id);
  SQL_ASSERT_FAILURE(ret);
  test_start.hw_config_id = (SQLCHAR*)fusion_db_hw_config_id;
  // Log that the test has started.
  ret = fusion_sql_insert_test_start(&db_handle, &test_start, fusion_db_test_start_id);
  SQL_ASSERT_FAILURE(ret);
}

// Provides a function call to store manufacturer-specific information to a database
void ddi_sdk_fusion_log_system_info (ddi_fusion_instance_t *instance, char *filename, uint32_t rate)
{
  cm_entry_t cm_db;
  int rim_count;
  int ret;
  ddi_fusion_cm_info_t info;
  memset(&info,0,sizeof(info));
  uint32_t len;
  FILE *fp = NULL;
  fp = fopen(filename, "a");
  if ( fp == NULL)
    return;
  time_t curtime;
  char software_version[MAX_STR];

  time(&curtime);

  // Log test-specific information to a file
  fprintf(fp,"*******************************\n");
  fprintf(fp,"Scan rate %d (us) \n", rate);
  fflush(fp);

  memset(&cm_db,0,sizeof(cm_entry_t));

  // Update the CM parameters
  ret = ecatCoeSdoUpload(instance->slave->info.dwSlaveId,0x100A,0,(uint8_t*)cm_db.sw_version, SQL_MAX_ENTRY_SIZE, &len, 20000, 0);
  fprintf(fp, "0x2000.01 Part Number = %s \n", cm_db.part_number);
  ret = ecatCoeSdoUpload(instance->slave->info.dwSlaveId,0x2000,1,(uint8_t*)cm_db.part_number, SQL_MAX_ENTRY_SIZE, &len, 20000, 0);
  fprintf(fp, "0x2000.01 Part Number = %s \n", cm_db.part_number);
  ret = ecatCoeSdoUpload(instance->slave->info.dwSlaveId,0x2000,2,(uint8_t*)cm_db.fw_version, SQL_MAX_ENTRY_SIZE, &len, 20000, 0);
  fprintf(fp, "0x2000.02 Firmware Version = %s \n", cm_db.fw_version);
  ret = ecatCoeSdoUpload(instance->slave->info.dwSlaveId,0x2000,3,(uint8_t*)cm_db.serial_number, SQL_MAX_ENTRY_SIZE, &len, 20000, 0);
  fprintf(fp, "0x2000.03 Serial Number = %s \n", cm_db.serial_number);
  ret = ecatCoeSdoUpload(instance->slave->info.dwSlaveId,0x2000,4,(uint8_t*)cm_db.ipv4_address, SQL_MAX_ENTRY_SIZE, &len, 20000, 0);
  fprintf(fp, "0x2000.04 IPv4 = %s \n", cm_db.ipv4_address);
  ret = ecatCoeSdoUpload(instance->slave->info.dwSlaveId,0x2000,5,(uint8_t*)cm_db.mac_address, SQL_MAX_ENTRY_SIZE, &len, 20000, 0);
  fprintf(fp, "0x2000.05 MAC address = %s \n", cm_db.mac_address);
  ret = ecatCoeSdoUpload(instance->slave->info.dwSlaveId,0x2000,6,(uint8_t*)cm_db.config_md5, SQL_MAX_ENTRY_SIZE, &len, 20000, 0);
  fprintf(fp, "0x2000.06 Configuration MD5 = %s len %d\n", cm_db.config_md5, len);
  ret = ecatCoeSdoUpload(instance->slave->info.dwSlaveId,0x2000,7,(uint8_t*)cm_db.config_date,SQL_MAX_ENTRY_SIZE, &len, 20000, 0);
  fprintf(fp, "0x2000.07 MD5 Date = %s \n", cm_db.config_date);
  ret = ecatCoeSdoUpload(instance->slave->info.dwSlaveId,0x2000,8,(uint8_t*)cm_db.clock,SQL_MAX_ENTRY_SIZE, &len, 20000, 0);
  fprintf(fp, "0x2000.08 Clock = %s \n", cm_db.clock);
  ret = ecatCoeSdoUpload(instance->slave->info.dwSlaveId,0x2000,9,(uint8_t*)&cm_db.event_mask,sizeof(uint32_t), &len, 20000, 0);
  fprintf(fp, "0x2000.09 Event Mask = 0x%04x len %d \n", cm_db.event_mask, len);
  ret = ecatCoeSdoUpload(instance->slave->info.dwSlaveId,0x2000,10,(uint8_t*)&cm_db.event_status,sizeof(uint32_t), &len, 20000, 0);
  fprintf(fp, "0x2000.0A Event Status = 0x%04x len %d \n", cm_db.event_status, len);
  ret = ecatCoeSdoUpload(instance->slave->info.dwSlaveId,0x2000,11,(uint8_t*)&cm_db.detect_modules_override,sizeof(uint32_t), &len, 20000, 0);
  fprintf(fp, "0x2000.0B Detect Modules Override = 0x%04x \n", cm_db.detect_modules_override);
  ret = ecatCoeSdoUpload(instance->slave->info.dwSlaveId,0x2000,13,(uint8_t*)&cm_db.cpu_fan_status,sizeof(float), &len, 20000, 0);
  fprintf(fp, "0x2000.0D CPU Fan Status = %.04f\n", cm_db.cpu_fan_status);
  ret = ecatCoeSdoUpload(instance->slave->info.dwSlaveId,0x2000,14,(uint8_t*)&cm_db.cpu_fan_rpm,sizeof(float), &len, 20000, 0);
  fprintf(fp, "0x2000.0E CPU Fan RPM = %.04f\n", cm_db.cpu_fan_rpm);
  ret = ecatCoeSdoUpload(instance->slave->info.dwSlaveId,0x2000,15,(uint8_t*)&cm_db.cpu_temperature,sizeof(float), &len, 20000, 0);
  fprintf(fp, "0x2000.0F CPU Fan temp = %.04f\n", cm_db.cpu_temperature);
  ret = fusion_sql_insert_cm(&db_handle, &cm_db);
  SQL_ASSERT_FAILURE(ret);
  ret= fusion_sql_insert_cm_bridge(&db_handle);
  SQL_ASSERT_FAILURE(ret);

  for (rim_count = 0; rim_count < DDI_FUSION_MAX_RIMS; rim_count++) // Get the RIM-specific information
  {
    rim_entry_t rim_entry;
    char rim_index_string[256];
    uint32_t status;
    uint32_t rim_index = 0x2011 + (rim_count * 0x10);
    ret = ecatCoeSdoUpload(instance->slave->info.dwSlaveId,rim_index,8,(uint8_t*)&rim_entry.event_status,4, &len, 20000, 0);
    if ( (ret != 0) || !( rim_entry.event_status & RIM_ONLINE ) )
    {
      continue;
    }
    rim_entry.port_number = rim_count + 1;
    sprintf(rim_index_string, "0x%04x", rim_index);
    fprintf(fp, "\n***Info for Active RIM port %d \n\n", rim_count+1);
    ret = ecatCoeSdoUpload(instance->slave->info.dwSlaveId,rim_index,1,(uint8_t*)rim_entry.part_number,1024, &len, 20000, 0);
    fprintf(fp, "%s.01 Part Number w = %s \n", rim_index_string, rim_entry.part_number);
    ret = ecatCoeSdoUpload(instance->slave->info.dwSlaveId,rim_index,2,(uint8_t*)rim_entry.fw_version,1024, &len, 20000, 0);
    fprintf(fp, "%s.02 Firmware Version = %s \n", rim_index_string, rim_entry.fw_version);
    ret = ecatCoeSdoUpload(instance->slave->info.dwSlaveId,rim_index,3,(uint8_t*)rim_entry.serial_number,1024, &len, 20000, 0);
    fprintf(fp, "%s.03 Serial Number = %s \n", rim_index_string, rim_entry.serial_number);
    ret = ecatCoeSdoUpload(instance->slave->info.dwSlaveId,rim_index,4,(uint8_t*)rim_entry.type, 1024, &len, 20000, 0);
    fprintf(fp, "%s.04 Type = %s \n", rim_index_string, (uint8_t*)rim_entry.type);
    ret = ecatCoeSdoUpload(instance->slave->info.dwSlaveId,rim_index,5,(uint8_t*)rim_entry.unit_information, 1024, &len, 20000, 0);
    fprintf(fp, "%s.05 Unit Information = %s \n", rim_index_string, rim_entry.unit_information);
    ret = ecatCoeSdoUpload(instance->slave->info.dwSlaveId,rim_index,6,(uint8_t*)rim_entry.slot_cards, 1024, &len, 20000, 0);
    fprintf(fp, "%s.06 Slot Cards = %s len %d \n", rim_index_string, rim_entry.slot_cards, len );
    ret = ecatCoeSdoUpload(instance->slave->info.dwSlaveId,rim_index,7,(uint8_t*)&rim_entry.event_mask, 4, &len, 20000, 0);
    fprintf(fp, "%s.07 Event Mask = 0x%x \n", rim_index_string, rim_entry.event_mask);
    ret = ecatCoeSdoUpload(instance->slave->info.dwSlaveId,rim_index,8,(uint8_t*)&rim_entry.event_status, 4, &len, 20000, 0);
    fprintf(fp, "%s.08 Event Status = 0x%x \n", rim_index_string, rim_entry.event_status);
    ret = ecatCoeSdoUpload(instance->slave->info.dwSlaveId,rim_index,10,(uint8_t*)&rim_entry.power_fan_status, 4, &len, 20000, 0);
    fprintf(fp, "%s.0A Power Fan Status = %f \n", rim_index_string, rim_entry.power_fan_status);
    ret = ecatCoeSdoUpload(instance->slave->info.dwSlaveId,rim_index,11,(uint8_t*)&rim_entry.slot_fan_status, 4, &len, 20000, 0);
    fprintf(fp, "%s.0B Slot Fan Status = %f \n", rim_index_string, rim_entry.slot_fan_status);
    ret = ecatCoeSdoUpload(instance->slave->info.dwSlaveId,rim_index,12,(uint8_t*)&rim_entry.power_fan_rpm, 4, &len, 20000, 0);
    fprintf(fp, "%s.0C Power Fan RPM = %f \n", rim_index_string, rim_entry.power_fan_rpm);
    ret = ecatCoeSdoUpload(instance->slave->info.dwSlaveId,rim_index,13,(uint8_t*)&rim_entry.slot_fan_rpm, 4, &len, 20000, 0);
    fprintf(fp, "%s.0D Slot Fan RPM = %f \n", rim_index_string, rim_entry.slot_fan_rpm);
    ret = fusion_sql_insert_rim(&db_handle, &rim_entry, fusion_db_rim_id[rim_count]);
    SQL_ASSERT_FAILURE(ret);
    ret = fusion_sql_insert_rim_bridge(&db_handle);
  }
  if ( fp )
    fclose(fp);
}

void ddi_sdk_fusion_set_aout (ddi_fusion_instance_t *instance, uint32_t index, uint16_t  value )
{
  uint16_t *mem_ptr = (uint16_t *)&instance->aout_desc[index].pd_output[instance->aout_desc[index].byte_offset];
  *mem_ptr = value;
}

uint16_t ddi_sdk_fusion_get_ain (ddi_fusion_instance_t *instance, uint32_t index )
{
  uint16_t *mem_ptr = (uint16_t *)&instance->ain_desc[index].pd_input[instance->ain_desc[index].byte_offset];
  return *mem_ptr;
}

void ddi_sdk_fusion_set_dout8 (ddi_fusion_instance_t *instance, uint32_t index, uint8_t  value )
{
  uint8_t *mem_ptr = (uint8_t *)&instance->dout_desc[index].pd_output[instance->dout_desc[index].byte_offset];
  *mem_ptr = (uint8_t)value;
}

void ddi_sdk_fusion_set_dout16 (ddi_fusion_instance_t *instance, uint32_t index, uint16_t  value )
{
  uint16_t *mem_ptr = (uint16_t *)&instance->dout_desc[index].pd_output[instance->dout_desc[index].byte_offset];
  *mem_ptr = (uint16_t)value;
}

uint8_t ddi_sdk_fusion_get_din8 (ddi_fusion_instance_t *instance, uint32_t index)
{
  uint8_t *mem_ptr = (uint8_t *)&instance->din_desc[index].pd_input[instance->din_desc[index].byte_offset];
  return *mem_ptr;
}

uint16_t ddi_sdk_fusion_get_din16 (ddi_fusion_instance_t *instance, uint32_t index)
{
  uint16_t *mem_ptr = (uint16_t *)&instance->din_desc[index].pd_input[instance->din_desc[index].byte_offset];
  return *mem_ptr;
}

uint16_t ddi_sdk_fusion_get_din (ddi_fusion_instance_t *instance, uint32_t index, uint16_t *length )
{
  if (instance->din_desc[index].size == 2)
  {
    uint16_t *mem_ptr = (uint16_t *)&instance->din_desc[index].pd_input[instance->din_desc[index].byte_offset];
    *length = 2;
    return *mem_ptr;
  }
  else
  {
    uint8_t *mem_ptr = (uint8_t *)&instance->din_desc[index].pd_input[instance->din_desc[index].byte_offset];
    *length = 1;
    return *mem_ptr;
  }
}

//sets up a process data descriptor from the process variable entry
void setup_pd_desc(EC_T_PROCESS_VAR_INFO_EX *entry, fusion_pd_desc_t *desc, uint8_t *pd_input, uint8_t *pd_output)
{
  //perform a one-time calculation to derive the byte offset, hi, lo, mask and size of the entry in bytes
  uint32_t offset = (uint32_t)entry->nBitOffs;
  uint32_t bitsize = (uint32_t)entry->nBitSize;
  uint16_t byte = ((offset + 8) / 8) - 1;
  uint32_t byten = (byte % 4) * 8;
  uint8_t lo = (offset - byten) % 32;
  uint8_t hi = (lo + bitsize - 1);
  uint32_t mask = BIT32_SHIFT_MASK(hi:lo);
  uint32_t size = BIT32_SIZE(hi:lo);
  size = (size + 7) >> 3;
  uint32_t reg;

  //setup descriptor
  desc->pd_input = pd_input;
  desc->pd_output = pd_output;
  desc->byte_offset = byte;
  desc->hi = hi;
  desc->lo = lo;
  desc->mask = mask;
  desc->size = size;
}

/* Open a fusion instance
 * This function will open an acontis instance and associate the acontis instance (the slave pointer)
 * with the fusion instance
 * In addition, this function will register the cyclic data callback given by the cyclic_function argument
 */
ddi_status_t ddi_sdk_fusion_open_base(ddi_fusion_instance_t **instance, cyclic_func_t *cyclic_function, AcontisCallback *cyclic_callback)
{
  ddi_fusion_instance_t *instance_ptr;
  int count;

  // validate arguments: if both cyclic_function and cyclic_callback are NULL
  // return ddi_status_param err
  if ( !cyclic_function && !cyclic_callback)
    return ddi_status_param_err;
  if ( !instance)
    return ddi_status_param_err;

  //find an unallocated instance
  for( count = 0; count < DDI_SDK_MAX_INSTANCES; count++ )
  {
    if( !fusion_instances[count].is_allocated )
    {
      DLOG(" Fusion instance found at 0x%x \n", count);
      //set the fusion interface to this fusion instance
      *instance = &fusion_instances[count];
      instance_ptr = *instance;
      //set the slave interface to the respective interface
      instance_ptr->slave = &slave[count];
      //setup cyclic data callback argument
      instance_ptr->slave->callback_args = *instance;
      //setup cyclic data callback address
      if (cyclic_function)
        instance_ptr->slave->cyclic_func = cyclic_function;
      else
        instance_ptr->slave->cyclic_object = cyclic_callback;

      //register the ecatslave interface
      if ( ddi_sdk_ecat_open(instance_ptr->slave) != ddi_status_ok )
      {
        ELOG(" error opening ecat interface \n");
        return ddi_status_err;
      }
      DLOG(" success opening ecat interface \n");
      //scan for this slave
      if ( ddi_sdk_ecat_scan_slave(instance_ptr) )
      {
        ELOG(" error scanning slave \n");
        return ddi_status_err;
      }
      if( instance_ptr->slave == NULL )
      {
        ELOG(" slave interface null \n");
        return ddi_status_err;
      }
      return ddi_status_ok;
    }
  }
  ELOG(" Fusion instance not found, all fusion instances are allocated \n");
  return ddi_status_err;
}

/**
 * Wrapper functions to ddi_sdk_fusion_open so that we do not have duplicate code.
 * @param instance
 * @param cyclic_function
 * @return
 */
ddi_status_t ddi_sdk_fusion_open(ddi_fusion_instance_t **instance, cyclic_func_t *cyclic_function)
{
    return ddi_sdk_fusion_open_base(instance, cyclic_function, nullptr);
}

ddi_status_t ddi_sdk_fusion_open(ddi_fusion_instance_t **instance, AcontisCallback *cyclic_callback)
{
    return ddi_sdk_fusion_open_base(instance, nullptr, cyclic_callback);
}

/* Close a fusion instance
 * This function will close an fusion-acontis mapping instance
 */
ddi_status_t ddi_sdk_fusion_close(ddi_fusion_instance_t *instance)
{
  int count;
  //validate arguments
  if(!instance)
  {
    return ddi_status_err;
  }

  instance->is_allocated = 0;
  //reset slave state
  instance->din_count  = 0;
  instance->dout_count = 0;
  instance->ain_count  = 0;
  instance->aout_count = 0;
  instance->dins       = 0;
  instance->douts      = 0;
  instance->ains       = 0;
  instance->aouts      = 0;
  instance->slave->allocated = 0;
  return ddi_status_ok;
}

//scan for Fusion.IO slaves
//need to modify this function to search for a specific slave station address
static uint32_t ddi_sdk_ecat_scan_slave(ddi_fusion_instance_t *instance)
{
  uint32_t status;
  uint16_t wFixedAddress = 0;
  uint16_t i, count = 0;
  uint16_t din_count = 0, dout_count = 0, ain_count = 0, aout_count = 0;
  int name_len, ret;

  //reset slave state
  instance->din_count  = 0;
  instance->dout_count = 0;
  instance->ain_count  = 0;
  instance->aout_count = 0;
  instance->dins       = 0;
  instance->douts      = 0;
  instance->ains       = 0;
  instance->aouts      = 0;

  // Initialize database if present
  ret = ddi_sdk_init_db(&db_handle);
  if ( ret != 1 )
  {
    ELOG("Warning: issue connecting to database or database not present\n");
    ddi_sdk_log_error("Warning: error connecting to DB = 0x%x \n", ret);
  }

  if (!ddi_sdk_get_slave_fixed_addr(instance->slave , 0x76b, 0x64, &wFixedAddress))
  {
    ELOG("Did not find a Fusion.IO slave\n");
    return 1;
  }

  DLOG("Found Digital Dynamics device: 0x%04x\n", wFixedAddress);

  /* now get the offset of this device in the process data buffer and some other infos */
  status = ecatGetCfgSlaveInfo(EC_TRUE, wFixedAddress, &instance->slave->info);
  if (status != 0)
  {
    ELOG("ERROR: ecatGetCfgSlaveInfo() returns with error: %s : 0x%x\n", ecatGetText(status), status);
    return status;
  }

  DLOG("VendorId:     0x%04x\n", instance->slave->info.dwVendorId);
  DLOG("ProductCode:  0x%x\n", instance->slave->info.dwProductCode);
  DLOG("DeviceName:   \"%s\"\n", instance->slave->info.abyDeviceName);
  DLOG("PDO input:    [%u - %u] (%u bits) 0x%x (%u) bytes\n", instance->slave->info.dwPdOffsIn, instance->slave->info.dwPdOffsIn + instance->slave->info.dwPdSizeIn, instance->slave->info.dwPdSizeIn, (instance->slave->info.dwPdSizeIn + 7) / 8, (instance->slave->info.dwPdSizeIn + 7) / 8);
  DLOG("PDO output:   [%u - %u] (%u bits) 0x%x (%u) bytes\n", instance->slave->info.dwPdOffsOut, instance->slave->info.dwPdOffsOut + instance->slave->info.dwPdSizeOut, instance->slave->info.dwPdSizeOut, (instance->slave->info.dwPdSizeOut + 7) / 8, (instance->slave->info.dwPdSizeOut + 7) / 8);
  DLOG("ENI input:    %d entries\n", instance->slave->info.wNumProcessVarsInp);
  DLOG("ENI output:   %d entries\n", instance->slave->info.wNumProcessVarsOutp);

  name_len = strlen(instance->slave->info.abyDeviceName);

  instance->slave->pd_input = ecatGetProcessImageInputPtr();
  instance->slave->pd_output = ecatGetProcessImageOutputPtr();

  count = instance->slave->info.wNumProcessVarsInp + instance->slave->info.wNumProcessVarsOutp;
  instance->slave->eni_entries = (EC_T_PROCESS_VAR_INFO_EX *)malloc(count * sizeof(EC_T_PROCESS_VAR_INFO_EX));
  if (instance->slave->eni_entries == EC_NULL)
  {
    ELOG("ERROR: failed to allocate memory for ENI entries\n");
    return EC_E_NOMEMORY;
  }

  status = ecatGetSlaveInpVarInfoEx(EC_TRUE, wFixedAddress, instance->slave->info.wNumProcessVarsInp, instance->slave->eni_entries, &i);
  if (status != EC_E_NOERROR)
  {
    ELOG("ERROR: 0x%x ecatGetSlaveInpVarInfoEx failed: %s \n", status, ecatGetText(status));
    return status;
  }

  status = ecatGetSlaveOutpVarInfoEx(EC_TRUE, wFixedAddress, instance->slave->info.wNumProcessVarsOutp, &instance->slave->eni_entries[i], &i);
  if (status != EC_E_NOERROR)
  {
    ELOG("ERROR: 0x%x ecatGetSlaveOutpVarInfoEx failed: %s\n", status, ecatGetText(status));
    return status;
  }

  instance->eni_count = count;

  uint16_t index = 0;
  EC_T_PROCESS_VAR_INFO_EX *entry = instance->slave->eni_entries;
  for (i = 0; i < count; i++)
  {
    int offset = name_len + (((entry->wIndex & 0xf000) == 0x6000) ? in_name_suffix_len : out_name_suffix_len);

    if (index != entry->wIndex)
    {
      index = entry->wIndex;
#ifdef DUMP_ENI
      DLOG("\n");
#endif
    }

#ifdef DUMP_ENI
    DLOG("%04x.%02x:%02x [%u:%u] \"%s\"\n",
         entry->wIndex, entry->wSubIndex, entry->nBitSize,
         entry->nBitOffs, (entry->nBitOffs + entry->nBitSize - 1),
//         entry->wDataType, type_to_string(entry->wDataType),
         entry->szName + offset);
#endif

    switch (entry->wIndex & 0xf00f)
    {
      case 0x6000:
      case 0x6001:
      case 0x6005:
      case 0x6006:
        if ( instance->pd_input == NULL)
        {
          instance->pd_input  = (uint8_t *)&instance->slave->pd_input[(entry->nBitOffs) / 8];
        }
        instance->din_count++;
        if (!instance->dins)
        {
          instance->dins  = (uint16_t *)&instance->slave->pd_input[(entry->nBitOffs) / 8];
        }
        //setup the process data descriptor for that din
        setup_pd_desc(entry,&instance->din_desc[din_count],instance->slave->pd_input,instance->slave->pd_output);
        din_count++;
        break;
      case 0x6002:
      case 0x6003:
      case 0x6004:
      case 0x6007:
      case 0x6008:
      case 0x6009:
        if ( instance->pd_input == NULL)
        {
          instance->pd_input  = (uint8_t *)&instance->slave->pd_input[(entry->nBitOffs) / 8];
        }
        instance->ain_count++;
        if (!instance->ains)
        {
          instance->ains  = (uint16_t *)&instance->slave->pd_input[(entry->nBitOffs) / 8];
        }
        setup_pd_desc(entry,&instance->ain_desc[ain_count],instance->slave->pd_input,instance->slave->pd_output);
        ain_count++;
        break;
      case 0x7000:
      case 0x7001:
        if ( instance->pd_output == NULL)
        {
          instance->pd_output  = (uint8_t *)&instance->slave->pd_output[(entry->nBitOffs) / 8];
        }
        instance->dout_count++;
        if (!instance->douts)
        {
          instance->douts = (uint16_t *)&instance->slave->pd_output[(entry->nBitOffs) / 8];
        }
        setup_pd_desc(entry,&instance->dout_desc[dout_count],instance->slave->pd_input,instance->slave->pd_output);
        dout_count++;
        break;
      case 0x7002:
      case 0x7003:
        if ( instance->pd_output == NULL)
        {
          instance->pd_output  = (uint8_t *)&instance->slave->pd_output[(entry->nBitOffs) / 8];
        }
        instance->aout_count++;
        if (!instance->aouts)
        {
          instance->aouts = (uint16_t *)&instance->slave->pd_output[(entry->nBitOffs) / 8];
        }
        setup_pd_desc(entry,&instance->aout_desc[aout_count],instance->slave->pd_input,instance->slave->pd_output);
        aout_count++;
        break;

      default:
        break;
    }

    entry++;
  }

  DLOG("\n");
  DLOG("%d\tDO %p\n", instance->dout_count, instance->douts);
  DLOG("%d\tAO %p\n", instance->aout_count, instance->aouts);
  DLOG("%d\tDI %p\n", instance->din_count,  instance->dins);
  DLOG("%d\tAI %p\n", instance->ain_count,  instance->ains);

  return status;
}
