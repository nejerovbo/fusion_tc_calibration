/******************************************************************************
 * (c) Copyright 2020-2021 Digital Dynamics Inc. Scotts Valley CA USA.
 * Unpublished copyright. All rights reserved. Contains proprietary and
 * confidential trade secrets belonging to DDI. Disclosure or release without
 * prior written authorization of DDI is prohibited.
 *****************************************************************************/

#include "ddi_sdk_ecat_sdo.h"
#include "ddi_sdk_ecat_master.h"
#include "EcObjDef.h"

//ddi ecat sdo dmsg
ddi_sdo_msg_t ddi_sdo_msg[] =
{
  //ddi_sdo_msg_t definition:
  //  Command ID                                       Index SubIndex RX/TX  TX amount, Rx amount, Tx string              us_timeout, datatype size in bytes  complete access
  //============================================================================================================================================================
  //get the device version
  { DDI_COE_GET_DEVICE_VERSION,                         0x1000, 0, DDI_RX, 0/*tx*/, 1/*rx*/,   {0},                        1000,      4,                    0},
  //get the device version
  { DDI_COE_GET_DEVICE_NAME,                            0x1008, 0, DDI_RX, 0/*tx*/, 1/*rx*/,   {0},                        1000,      256,                  0},
  //get the device version
  { DDI_COE_GET_HW_VERSION,                             0x1009, 0, DDI_RX, 0/*tx*/, 1/*rx*/,   {0},                        1000,      256,                  0},  
  //get the SW version
  { DDI_COE_GET_SW_VERSION,                             0x100A, 0, DDI_RX, 0/*tx*/, 1/*rx*/,   {0},                        1000,      256,                  0},
  //get the BL version
  { DDI_COE_GET_BL_VERSION,                             0x100B, 0, DDI_RX, 0/*tx*/, 1/*rx*/,   {0},                        1000,      256,                  0},
  //get module index distance
  { DDI_COE_GET_MODULE_PROFILE_IDX_DIST,                0xF000, 1, DDI_RX, 0/*tx*/, 1/*rx*/,   {0},                        1000,      2,                    0},
  //get max number of modules
  { DDI_COE_GET_MODULE_MAX_MODULES,                     0xF000, 2, DDI_RX, 0/*tx*/, 1/*rx*/,   {0},                        1000,      2,                    0},
  //get list of module idents
  { DDI_COE_GET_MODULE_PROFILE_LIST,                    0xF010, 1, DDI_RX, 0/*tx*/, 6/*rx*/,   {0},                        1000,      4,                    1},  
  //issue detect module command
  { DDI_COE_DETECT_MODULE_CMD,                          0xF002, 1, DDI_TX, 1/*tx*/, 0/*rx*/,   {'n','a','d','a'},          1000,      4,                    0},
  //get the number of configured modules
  { DDI_COE_GET_NUMBER_OF_CONFIG_MODULES,               0xF030, 0, DDI_RX, 0/*tx*/, 1/*rx*/,   {0},                        1000,      4,                    0},
  //get the actual configured modules
  { DDI_COE_GET_CONFIGURED_MODULES,                     0xF030, 1, DDI_RX, 0/*tx*/, 256/*rx*/, {0},                        1000,      4,                    1},  
  //get the number of detected modules
  { DDI_COE_GET_NUMBER_OF_DETECTED_MODULES,             0xF050, 0, DDI_RX, 0/*tx*/, 1/*rx*/,   {0},                        1000,      4,                    0},
  //get the actual detected modules
  { DDI_COE_GET_DETECTED_MODULES,                       0xF050, 1, DDI_RX, 0/*tx*/, 256/*rx*/, {0},                        1000,      4,                    1},
  //get the active exception status
  { DDI_COE_GET_ACTIVE_EXCEPTION_STATUS,                0xF380, 0, DDI_RX, 0/*tx*/, 1/*rx*/,   {0},                        1000,      2,                    0},
  //get the active device warning details
  { DDI_COE_GET_ACTIVE_DEVICE_WARNING_DETAILS,          0xF381, 1, DDI_RX, 0/*tx*/, 256/*rx*/, {0},                        1000,      4,                    1},
  //get the active manufacturer warning details
  { DDI_COE_GET_ACTIVE_MANF_WARNING_DETAILS,            0xF382, 1, DDI_RX, 0/*tx*/, 256/*rx*/, {0},                        1000,      4,                    1},
  //get the active device error details
  { DDI_COE_GET_ACTIVE_DEVICE_ERROR_DETAILS,            0xF383, 1, DDI_RX, 0/*tx*/, 256/*rx*/, {0},                        1000,      4,                    1},
  //get the active manufacturer error details
  { DDI_COE_GET_ACTIVE_MANF_ERROR_DETAILS,              0xF384, 1, DDI_RX, 0/*tx*/, 256/*rx*/, {0},                        1000,      4,                    1},
  //get the active global device warning details
  { DDI_COE_GET_ACTIVE_GLOBAL_DEV_WARNING_DETAILS,      0xF385, 1, DDI_RX, 0/*tx*/, 256/*rx*/, {0},                        1000,      4,                    1},
  //get the active global manufacturer warning details
  { DDI_COE_GET_ACTIVE_GLOBAL_MANF_WARNING_DETAILS,     0xF386, 1, DDI_RX, 0/*tx*/, 256/*rx*/, {0},                        1000,      4,                    1},
  //get the active global device error details
  { DDI_COE_GET_ACTIVE_GLOBAL_DEV_ERROR_DETAILS,        0xF387, 1, DDI_RX, 0/*tx*/, 256/*rx*/, {0},                        1000,      4,                    1},
  //get the active global manufacturer erorr status
  { DDI_COE_GET_ACTIVE_GLOBAL_MANF_ERROR_DETAILS,       0xF388, 1, DDI_RX, 0/*tx*/, 256/*rx*/, {0},                        1000,      4,                    1},
  //get the active exception status
  { DDI_COE_GET_LATCHED_EXCEPTION_STATUS,               0xF390, 0, DDI_RX, 0/*tx*/, 1/*rx*/,   {0},                        1000,      2,                    0},
  //get the active device warning details
  { DDI_COE_GET_LATCHED_DEVICE_WARNING_DETAILS,         0xF391, 1, DDI_RX, 0/*tx*/, 256/*rx*/, {0},                        1000,      4,                    1},
  //get the active manufacturer warning details
  { DDI_COE_GET_LATCHED_MANF_WARNING_DETAILS,           0xF392, 1, DDI_RX, 0/*tx*/, 256/*rx*/, {0},                        1000,      4,                    1},
  //get the active device error details
  { DDI_COE_GET_LATCHED_DEVICE_ERROR_DETAILS,           0xF393, 1, DDI_RX, 0/*tx*/, 256/*rx*/, {0},                        1000,      4,                    1},
  //get the active manufacturer error details
  { DDI_COE_GET_LATCHED_MANF_ERROR_DETAILS,             0xF394, 1, DDI_RX, 0/*tx*/, 256/*rx*/, {0},                        1000,      4,                    1},
  //get the active global device warning details
  { DDI_COE_GET_LATCHED_GLOBAL_DEV_WARNING_DETAILS,     0xF395, 1, DDI_RX, 0/*tx*/, 256/*rx*/, {0},                        1000,      4,                    1},
  //get the active global manufacturer warning details
  { DDI_COE_GET_LATCHED_GLOBAL_MANF_WARNING_DETAILS,    0xF396, 1, DDI_RX, 0/*tx*/, 256/*rx*/, {0},                        1000,      4,                    1},
  //get the active global device error details
  { DDI_COE_GET_LATCHED_GLOBAL_DEV_ERROR_DETAILS,       0xF397, 1, DDI_RX, 0/*tx*/, 256/*rx*/, {0},                        1000,      4,                    1},
  //get the active global manufacturer error status
  { DDI_COE_GET_LATCHED_GLOBAL_MANF_ERROR_DETAILS,      0xF398, 1, DDI_RX, 0/*tx*/, 256/*rx*/, {0},                        1000,      4,                    1},
  //set the device warning mask
  { DDI_COE_SET_DEVICE_WARNING_MASK,                    0xF3A1, 1, DDI_TX, 256/*tx*/, 0/*rx*/, {0},                        1000,      4,                    1},
  //set the manufacturer warning mask
  { DDI_COE_SET_MANF_WARNING_MASK,                      0xF3A2, 1, DDI_TX, 256/*tx*/, 0/*rx*/, {0},                        1000,      4,                    1},
  //set the device error mask
  { DDI_COE_SET_DEVICE_ERROR_MASK,                      0xF3A3, 1, DDI_TX, 256/*tx*/, 0/*rx*/, {0},                        1000,      4,                    1},
  //set the manufacturer error mask
  { DDI_COE_SET_MANF_ERROR_MASK,                        0xF3A4, 1, DDI_TX, 256/*tx*/, 0/*rx*/, {0},                        1000,      4,                    1},
  //set the device warning mask
  { DDI_COE_SET_GLOBAL_DEVICE_WARNING_MASK,             0xF3A5, 1, DDI_TX, 256/*tx*/, 0/*rx*/, {0},                        1000,      4,                    1},
  //set the manufacturer warning mask
  { DDI_COE_SET_GLOBAL_MANF_WARNING_MASK,               0xF3A6, 1, DDI_TX, 256/*tx*/, 0/*rx*/, {0},                        1000,      4,                    1},
  //set the device error mask
  { DDI_COE_SET_GLOBAL_DEVICE_ERROR_MASK,               0xF3A7, 1, DDI_TX, 256/*tx*/, 0/*rx*/, {0},                        1000,      4,                    1},
  //set the manufacturer error mask
  { DDI_COE_SET_GLOBAL_MANF_ERROR_MASK,                 0xF3A8, 1, DDI_TX, 256/*tx*/, 0/*rx*/, {0},                        1000,      4,                    1},
  //get the manufacturer serial number
  { DDI_COE_GET_MANF_SERIAL_NUMBER,                     0xF9F0, 0, DDI_RX, 0/*tx*/, 1/*rx*/,   {0},                        1000,      16,                   1},
  //get the CDP Functional Generation Number
  { DDI_COE_GET_CDP_FGN,                                0xF9F1, 0, DDI_RX, 0/*tx*/, 1/*rx*/,   {0},                        1000,      4,                    0},
  //get the SDP Functional Generation Number
  { DDI_COE_GET_SDP_FGN,                                0xF9F2, 0, DDI_RX, 0/*tx*/, 256/*rx*/, {0},                        1000,      4,                    0},
  //get the Vendor Name
  { DDI_COE_GET_VENDOR_NAME,                            0xF9F3, 0, DDI_RX, 0/*tx*/, 1/*rx*/,   {0},                        1000,      16,                   0},
  //get the SDP Name
  { DDI_COE_GET_SDP_NAME,                               0xF9F4, 0, DDI_RX, 0/*tx*/, 1/*rx*/,   {0},                        1000,      16,                   0},
  //get the Output Identifier
  { DDI_COE_GET_OUTPUT_IDENTIFIER,                      0xF9F5, 0, DDI_RX, 0/*tx*/, 256/*rx*/, {0},                        1000,      2,                    0},
  //get the time since poweron
  { DDI_COE_GET_TIME_SINCE_POWERON,                     0xF9F6, 0, DDI_RX, 0/*tx*/, 1/*rx*/,   {0},                        1000,      4,                    0},
  //get the Firmware update FGN
  { DDI_COE_GET_FW_UDPATE_FGN,                          0xF9F7, 0, DDI_RX, 0/*tx*/, 1/*rx*/,   {0},                        1000,      4,                    0},
  //reset device without restoring factory defaults
  { DDI_COE_SET_DEVICE_RESET,                           0xFBF0, 1, DDI_TX, 6/*tx*/, 0/*rx*/,   {'t','e','s','e','r','\0'}, 1000,      1,                    0},
  //reset device along with restoring factory defaults
  { DDI_COE_SET_FACTORY_DEVICE_RESET,                   0xFBF0, 1, DDI_TX, 6/*tx*/, 0/*rx*/,   {'t','e','s','e','r','f'},  1000,      1,                    0},
  //clears latched exception status
  { DDI_COE_SET_EXCEPTION_RESET,                        0xFBF1, 1, DDI_TX, 5/*tx*/, 0/*rx*/,   {'t','e','s','e','r'},      1000,      1,                    0},
  //store parameters command
  { DDI_COE_STORE_PARAMETERS_CMD,                       0xFBF2, 1, DDI_TX, 4/*tx*/, 0/*rx*/,   {'s','a','v','e'},          1000,      1,                    0},
  //calculate checksum using MD5
  { DDI_COE_START_CALCULATE_CHECKSUM_MD5,               0xFBF3, 1, DDI_TX, 4/*tx*/, 0/*rx*/,   {0x04,0,0,0},               1000,      1,                    0},
  //calculate checksum using the default method
  { DDI_COE_START_CALCULATE_CHECKSUM_DEFAULT,           0xFBF3, 1, DDI_TX, 4/*tx*/, 0/*rx*/,   {0x01,0,0,0},               1000,      1,                    0},
  //calculate checksum using the default method
  { DDI_COE_GET_CALCULATE_CHECKSUM_RESULT,              0xFBF3, 3, DDI_RX, 0/*tx*/, 18/*rx*/,  {0},                        1000,      1,                    0},
  //load parameters command
  { DDI_COE_LOAD_PARAMETERS_CMD,                        0xFBF4, 1, DDI_TX, 4/*tx*/, 0/*rx*/,   {'l','o','a','d'},          1000,      1,                    0}
};

//load a custom manufacturer SDO
void *manufactuer_sdo_ptr = NULL;
uint32_t manufacturer_sdo_size;

//display the al status and al statuscode
void display_alstatus (slave_t *slave)
{
   uint32_t sdo_code;
   uint32_t length;
   int result;

   result = ecatCoeSdoUpload(slave->info.dwSlaveId, 0x130 , 0,
        (uint8_t*)&sdo_code, sizeof(uint32_t), &length, 15000, 1);

   if(result != 0 )  {
    ELOG("ddi_sdk_ecat_coe_upload to addr 0x%x did not succeed result %s = 0x%x\n", 0x130, ecatGetText(result),result);
    return;
   }

   printf(" ALSTATUS = 0x%x  \n", sdo_code);

   result = ecatCoeSdoUpload(slave->info.dwSlaveId, 0x134 , 0,
     (uint8_t*)&sdo_code, sizeof(uint32_t), &length, 15000, 1);

   if(result != 0 )  {
    ELOG("ddi_sdk_ecat_coe_upload to addr 0x%x did not succeed result %s = 0x%x\n", 0x134, ecatGetText(result),result);
    return;
   }

   printf(" ALSTATUS CODE = 0x%x \n", sdo_code);
}

//parses the ddi_sdo_msg_t structure and sends RX/TX messages accordingly.
ddi_status_t ddi_sdk_ecat_sdo_msg(slave_t *slave, uint32_t sdo_cmd, char *msg, uint32_t *msg_length )
{
  ddi_sdo_msg_t *msg_ptr = ddi_sdo_msg;
  int msg_valid = 0;
  int count;
  int max_entry = sizeof(ddi_sdo_msg)/sizeof(ddi_sdo_msg_t);
  uint32_t result;
  VLOG("max_entry %d \n", max_entry);
  if (slave == NULL)
  {
    ELOG(" slave null \n");
    return ddi_status_err;
  }
  for(count = 0; count < max_entry; count++)
  {
    if(msg_ptr->id == sdo_cmd)
    {
      VLOG(" SDO msg 0x%x found \n", msg_ptr->id);
      msg_valid = 1;
    }
    //master -> slave
    if (msg_valid && msg_ptr->direction == DDI_TX)
    {
      uint32_t length;
      VLOG(" sdo_address 0x%x subindex 0x%x \n", msg_ptr->address, msg_ptr->subindex);
      VLOG("%s \n", msg_ptr->buffer);

      result = ecatCoeSdoDownload(slave->info.dwSlaveId, msg_ptr->address, msg_ptr->subindex,
        (uint8_t*)msg_ptr->buffer, msg_ptr->tx_amount*msg_ptr->sizeof_data_type, msg_ptr->timeout_us, msg_ptr->complete_access);

      //for CDP commands, read the status and wait until completion, 0xFF is pending
      if ( ( msg_ptr->address == 0xF002 ) || ( msg_ptr->address == 0xFBF0 ) || ( msg_ptr->address == 0xFBF1 ) || ( msg_ptr->address == (0xFBF2) )|| \
           ( msg_ptr->address == 0xFBF3 ) || ( msg_ptr->address == 0xFBF4 ) )
      {
        msg_ptr->buffer[0] = 0xFF;
        while ( msg_ptr->buffer[0] == 0XFF)
        {
          result = ecatCoeSdoUpload(slave->info.dwSlaveId, msg_ptr->address, 2,
            (uint8_t*)msg_ptr->buffer, 2, &length, msg_ptr->timeout_us, 0);
          VLOG(" Command status: pending \n");
        }
      }
      VLOG(" Command status: 0x%x  \n",msg_ptr->buffer[0]);

      if(result != 0 ) {
        ELOG("ddi_sdk_ecat_coe_download to addr 0x%x did not succeed result %s = 0x%x\n", msg_ptr->address, ecatGetText(result), result);
        return ddi_status_err;
      }
      break;


    } //end DDI_TX
    //slave->master
    if (msg_valid && msg_ptr->direction == DDI_RX)
    {
      uint32_t length;
      VLOG(" sdo_address 0x%x subindex 0x%x rx_amount 0x%x data_type_size 0x%x \n", msg_ptr->address, msg_ptr->subindex, msg_ptr->rx_amount, msg_ptr->sizeof_data_type);

      result = ecatCoeSdoUpload(slave->info.dwSlaveId, msg_ptr->address, msg_ptr->subindex,
        (uint8_t*)msg_ptr->buffer, msg_ptr->rx_amount*msg_ptr->sizeof_data_type, &length, msg_ptr->timeout_us, msg_ptr->complete_access);

      if(result != 0 )  {
        ELOG("ddi_sdk_ecat_coe_upload to addr 0x%x did not succeed result %s = 0x%x\n", msg_ptr->address, ecatGetText(result),result);
        return ddi_status_err;
      }
      VLOG(" coe upload success length = 0x%x \n", length);

      if(msg) 
      {
        memcpy(msg,msg_ptr->buffer, length);
        *msg_length = length;
      } 
      else
      {
        return ddi_status_err;
      }
      break;
    }  //end DDI_RX
    msg_ptr++;
  } //end max entry count
  return ddi_status_ok;
}

//wrapper providing functionality to transfer data from slave to master
ddi_status_t ddi_sdk_ecat_coe_read(slave_t *slave, uint32_t sdo_address, uint32_t subindex, char *msg,
                                   uint32_t msg_length, uint32_t *return_length, uint32_t timeout_us, uint32_t complete_access)
{
  uint32_t result;
  result = ecatCoeSdoUpload(slave->info.dwSlaveId, sdo_address, subindex,
    (uint8_t*)msg, msg_length, return_length, timeout_us, complete_access);

  if(result != 0 ) {
    ELOG("ddi_sdk_ecat_coe_read to addr 0x%x did not succeed \n", sdo_address);
  }
  return ddi_status_ok;
}

//wrapper providing functionality to transfer data from slave to master
ddi_status_t ddi_sdk_ecat_coe_write(slave_t *slave, uint32_t sdo_address, uint32_t subindex, char *msg,
                                    uint32_t length, uint32_t timeout_us, uint32_t complete_access)
{
  uint32_t result;
  result = ecatCoeSdoDownload(slave->info.dwSlaveId, sdo_address, subindex,
    (uint8_t*)msg, length, timeout_us, complete_access);

  if(result != 0 ) {
    ELOG("ddi_sdk_ecat_coe_write to addr 0x%x did not succeed \n", sdo_address);
  }
  return ddi_status_ok;
}

/********************************************************************************/
/** \brief  Wait for mailbox transfer completion, log error
*
* \return N/A
*/
EC_T_VOID HandleMbxTferReqError
    (char*           szErrMsg          /**< [in] error message */
    ,uint32_t           dwErrorCode       /**< [in] basic error code */
    ,EC_T_MBXTFER*        pMbxTfer)         /**< [in] mbx transfer object */
{
    /* wait for MbxTfer completion, but let application finish if master never returns MbxTfer object (may happen using RAS) */
    {
        uint32_t dwWorstCaseTimeout = 10;

        /*
         * Wait if MbxTfer still running and MbxTfer object owned by master.
         * Cannot re-use MbxTfer object while state is eMbxTferStatus_Pend.
         */
        for (dwWorstCaseTimeout = 10; (eMbxTferStatus_Pend == pMbxTfer->eTferStatus) && (dwWorstCaseTimeout > 0); dwWorstCaseTimeout--)
        {
            ELOG( "%s: waiting for mailbox transfer response\n", szErrMsg);
            OsSleep(2000);
        }

        if (eMbxTferStatus_Pend == pMbxTfer->eTferStatus)
        {
            ELOG( "%s: timeout waiting for mailbox transfer response\n", szErrMsg);
            goto Exit;
        }
    }

    if (EC_E_NOERROR != dwErrorCode)
    {
        ELOG( "%s: MbxTferReqError: %s (0x%x)\n", szErrMsg, ecatGetText(dwErrorCode), dwErrorCode);
    }

Exit:
    return;
}

extern uint32_t client_id;

/*-DEFINES-------------------------------------------------------------------*/
#ifndef BIT2BYTE
    #define BIT2BYTE(x) \
        (((x)+7)>>3)
#endif

#define COE_PREFIX "[OBJECT_DICTIONARY]"
#define COE_LOG(fmt, ...) do{  fprintf(coe_od_fp, COE_PREFIX fmt , ##__VA_ARGS__ ); fflush(coe_od_fp); }while(0)

/********************************************************************************/
/** \brief  Read object dictionary.
*
* This function reads the CoE object dictionary.
*
* \return EC_E_NOERROR on success, error code otherwise.
*/
ddi_status_t ddi_sdk_ecat_read_obj_dictionary
(
    slave_t            *slave,            /**< [in]   Slave instance */
    char               *output_filename,  /**< [in]   output_filename */
    uint8_t            live_update,       /**< [in]   EC_TRUE: do SDO Upload */
    uint32_t           timeout_us         /**< [in]   Individual call timeout */
)
{
    /* buffer sizes */
#define CROD_ODLTFER_SIZE       ((uint32_t)0x1200)
#define CROD_OBDESC_SIZE        ((uint32_t)100)
#define CROD_ENTRYDESC_SIZE     ((uint32_t)100)
#define CROD_MAXSISDO_SIZE      ((uint32_t)0x200)
#define MAX_OBNAME_LEN          ((uint32_t)100)
#define LOG_BUFFER_SIZE         ((uint32_t)2048)

    /* variables */
    uint32_t            ec_result                   = EC_E_ERROR;   /* tmp return value for API calls */
    char*               szLogBuffer             = EC_NULL;      /* log buffer for formatted string*/
    uint8_t*            pbyODLTferBuffer        = EC_NULL;      /* OD List */
    uint8_t*            pbyOBDescTferBuffer     = EC_NULL;      /* OB Desc */
    uint8_t*            pbyGetEntryTferBuffer   = EC_NULL;      /* Entry Desc */
    EC_T_MBXTFER_DESC   MbxTferDesc             = {0};          /* mailbox transfer descriptor */
    EC_T_MBXTFER*       pMbxGetODLTfer          = EC_NULL;      /* mailbox transfer object for OD list upload */
    EC_T_MBXTFER*       pMbxGetObDescTfer       = EC_NULL;      /* mailbox transfer object for Object description upload */
    EC_T_MBXTFER*       pMbxGetEntryDescTfer    = EC_NULL;      /* mailbox transfer object for Entry description upload */

    uint16_t*           pwODList                = EC_NULL;      /* is going to carry ALL list of OD */
    uint16_t            wODListLen              = 0;            /* used entries in pwODList */
    uint16_t            wIndex                  = 0;            /* index to parse OD List */

    uint8_t             byValueInfoType         = 0;
    uint32_t            dwUniqueTransferId      = 0;
    EC_T_BOOL           bReadingMasterOD        = EC_FALSE;
    uint32_t            dwNodeId                = slave->info.dwSlaveId;
    FILE                *coe_od_fp;
    ddi_status_t        result                  = ddi_status_ok;

    /* Open Obj dictionary log file */
    if ( output_filename == NULL )
    {
        ELOG(" output filename is NULL \n");
        return -1;
    }

    /* Don't append, overwrite the existing file */
    coe_od_fp = fopen(output_filename, "w");
    if ( !coe_od_fp)
    {
        ELOG(" output_filename %s did not open ", output_filename);
        return -1;
    }

    /* Create Memory */
    pbyODLTferBuffer        = (uint8_t*)malloc(CROD_ODLTFER_SIZE);
    pbyOBDescTferBuffer     = (uint8_t*)malloc(CROD_OBDESC_SIZE);
    pbyGetEntryTferBuffer   = (uint8_t*)malloc(CROD_ENTRYDESC_SIZE);
    szLogBuffer             = (char*)malloc(LOG_BUFFER_SIZE);

    szLogBuffer[0] = '\0';
    szLogBuffer[LOG_BUFFER_SIZE - 1] = '\0';

    /* check if alloc was ok */
    if ((EC_NULL == pbyODLTferBuffer)
     || (EC_NULL == pbyOBDescTferBuffer)
     || (EC_NULL == pbyGetEntryTferBuffer))
    {
        result = ddi_status_no_resources;
        goto Exit;
    }

    memset(pbyODLTferBuffer,      0, CROD_ODLTFER_SIZE);
    memset(pbyOBDescTferBuffer,   0, CROD_OBDESC_SIZE);
    memset(pbyGetEntryTferBuffer, 0, CROD_ENTRYDESC_SIZE);

    /* create required MBX Transfer Objects */
    /* mailbox transfer object for OD list upload */
    MbxTferDesc.dwMaxDataLen        = CROD_ODLTFER_SIZE;
    MbxTferDesc.pbyMbxTferDescData  = pbyODLTferBuffer;

    pMbxGetODLTfer = emMbxTferCreate(INSTANCE_MASTER_DEFAULT, &MbxTferDesc);
    if (EC_NULL == pMbxGetODLTfer)
    {
        result = ddi_status_no_resources;
        goto Exit;
    }

    /* mailbox transfer object for Object description upload */
    MbxTferDesc.dwMaxDataLen        = CROD_OBDESC_SIZE;
    MbxTferDesc.pbyMbxTferDescData  = pbyOBDescTferBuffer;

    pMbxGetObDescTfer = emMbxTferCreate(INSTANCE_MASTER_DEFAULT, &MbxTferDesc);
    if (EC_NULL == pMbxGetObDescTfer)
    {
        result = ddi_status_no_resources;
        goto Exit;
    }

    /* mailbox transfer object for Entry description upload */
    MbxTferDesc.dwMaxDataLen        = CROD_ENTRYDESC_SIZE;
    MbxTferDesc.pbyMbxTferDescData  = pbyGetEntryTferBuffer;

    pMbxGetEntryDescTfer = emMbxTferCreate(INSTANCE_MASTER_DEFAULT, &MbxTferDesc);
    if (EC_NULL == pMbxGetEntryDescTfer)
    {
        result = ddi_status_no_resources;
        goto Exit;
    }

    /* Get OD List Type: ALL */
    pMbxGetODLTfer->dwClntId    = client_id;
    pMbxGetODLTfer->dwTferId    = dwUniqueTransferId++;
    pMbxGetODLTfer->dwDataLen   = pMbxGetODLTfer->MbxTferDesc.dwMaxDataLen;

    /* get list of object indexes */
    ec_result = emCoeGetODList(INSTANCE_MASTER_DEFAULT, pMbxGetODLTfer, dwNodeId, eODListType_ALL, timeout_us);
    if (EC_E_SLAVE_NOT_PRESENT == ec_result)
    {
        result = ddi_status_not_found;
        goto Exit;
    }

    /* wait until transfer object is available incl. logging error */
    HandleMbxTferReqError( (char*)"CoeReadObjectDictionary: Error in emCoeGetODList(ALL)", ec_result, pMbxGetODLTfer);
    if (EC_E_NOERROR != ec_result)
    {
        result = ddi_status_err;
        goto Exit;
    }

    /* OD Tfer object now shall contain complete list of OD Objects, store it for more processing */
    pwODList = (uint16_t*)malloc(sizeof(uint16_t) * pMbxGetODLTfer->MbxData.CoE_ODList.wLen);
    if (EC_NULL == pwODList)
    {
        result = ddi_status_no_resources;
        goto Exit;
    }
    memset(pwODList, 0, sizeof(uint16_t) * pMbxGetODLTfer->MbxData.CoE_ODList.wLen);

    /* reading master OD */
    if (MASTER_SLAVE_ID == dwNodeId)
    {
        bReadingMasterOD = EC_TRUE;
    }

    COE_LOG("Complete OD list:\n");

    /* iterate through all entries in list */
    for (wODListLen = 0, wIndex = 0; wIndex < (pMbxGetODLTfer->MbxData.CoE_ODList.wLen); wIndex++)
    {
        /* store next index */
        pwODList[wODListLen] = pMbxGetODLTfer->MbxData.CoE_ODList.pwOdList[wIndex];

        COE_LOG("%04X \n", pwODList[wODListLen]);

        /* to store only non empty index entries, increment List Length only if non zero entry */
        if (0 != pwODList[wODListLen])
        {
            wODListLen++;
        }
    }

    /* MbxGetODLTfer done */
    pMbxGetODLTfer->eTferStatus = eMbxTferStatus_Idle;

    /* Get OD List Type: RX PDO Map */
    pMbxGetODLTfer->dwClntId    = client_id;
    pMbxGetODLTfer->dwTferId    = dwUniqueTransferId++;
    pMbxGetODLTfer->dwDataLen   = pMbxGetODLTfer->MbxTferDesc.dwMaxDataLen;

    ec_result = emCoeGetODList(INSTANCE_MASTER_DEFAULT, pMbxGetODLTfer, dwNodeId, eODListType_RxPdoMap, timeout_us);
    if (EC_E_SLAVE_NOT_PRESENT == ec_result)
    {
        result = ddi_status_not_found;
        goto Exit;
    }

    /* wait until transfer object is available incl. logging error */
    HandleMbxTferReqError((char*)"CoeReadObjectDictionary: Error in emCoeGetODList(RxPdoMap)", ec_result, pMbxGetODLTfer);
    if (EC_E_NOERROR != ec_result)
    {
        result = ddi_status_err;
        goto Exit;
    }

    COE_LOG("RX PDO Mappable Objects:\n");
    /* iterate through all entries in list */
    for (wIndex = 0; wIndex < (pMbxGetODLTfer->MbxData.CoE_ODList.wLen); wIndex++)
    {
        COE_LOG( "%04X ", pMbxGetODLTfer->MbxData.CoE_ODList.pwOdList[wIndex]);
    }


    /* MbxGetODLTfer done */
    pMbxGetODLTfer->eTferStatus = eMbxTferStatus_Idle;

    /* Get OD List Type: TX PDO Map */
    pMbxGetODLTfer->dwClntId    = client_id;
    pMbxGetODLTfer->dwTferId    = dwUniqueTransferId++;
    pMbxGetODLTfer->dwDataLen   = pMbxGetODLTfer->MbxTferDesc.dwMaxDataLen;

    ec_result = emCoeGetODList(INSTANCE_MASTER_DEFAULT, pMbxGetODLTfer, dwNodeId, eODListType_TxPdoMap, timeout_us);
    if (EC_E_SLAVE_NOT_PRESENT == ec_result)
    {
        result = ddi_status_not_found;
        goto Exit;
    }

    /* wait until transfer object is available incl. logging error */
    HandleMbxTferReqError( (char*)"CoeReadObjectDictionary: Error in emCoeGetODList(TxPdoMap)", ec_result, pMbxGetODLTfer);
    if (EC_E_NOERROR != ec_result)
    {
        result = ddi_status_err;
        goto Exit;
    }

    /* now display Entries of ODList */

    COE_LOG("TX PDO Mappable Objects:\n");
    /* iterate through all entries in list */
    for( wIndex = 0; wIndex < (pMbxGetODLTfer->MbxData.CoE_ODList.wLen); wIndex++ )
    {
        COE_LOG("%04X ", pMbxGetODLTfer->MbxData.CoE_ODList.pwOdList[wIndex]);
    }

    /* MbxGetODLTfer done */
    pMbxGetODLTfer->eTferStatus = eMbxTferStatus_Idle;

    /* now iterate through Index list, to get closer info, sub indexes and values */

    /* get object description for all objects */
    COE_LOG("\n");
    COE_LOG("*************************************************************\n");
    COE_LOG("****                  OBJECT DESCRIPTION                 ****\n");
    COE_LOG("*************************************************************\n");

    /* init value info type */
    byValueInfoType = EC_COE_ENTRY_ObjAccess
                    | EC_COE_ENTRY_ObjCategory
                    | EC_COE_ENTRY_PdoMapping
                    | EC_COE_ENTRY_UnitType
                    | EC_COE_ENTRY_DefaultValue
                    | EC_COE_ENTRY_MinValue
                    | EC_COE_ENTRY_MaxValue;

    for (wIndex = 0; (wIndex < wODListLen); wIndex++)
    {
        uint16_t wSubIndexLimit = 0x100; /* SubIndex range: 0x00 ... 0xff */
        uint16_t wSubIndex      = 0;

        /* get Object Description */
        pMbxGetObDescTfer->dwClntId     = client_id;
        pMbxGetObDescTfer->dwDataLen    = pMbxGetObDescTfer->MbxTferDesc.dwMaxDataLen;
        pMbxGetObDescTfer->dwTferId     = dwUniqueTransferId++;

        /* get object description */
        ec_result = emCoeGetObjectDesc(INSTANCE_MASTER_DEFAULT, pMbxGetObDescTfer, dwNodeId, pwODList[wIndex], timeout_us);
        if (EC_E_SLAVE_NOT_PRESENT == ec_result)
        {
            result = ddi_status_not_found;
            goto Exit;
        }

        /* wait until transfer object is available incl. logging error */
        HandleMbxTferReqError((char*)"CoeReadObjectDictionary: Error in emCoeGetObjectDesc", ec_result, pMbxGetODLTfer);
        if (EC_E_NOERROR != ec_result)
        {
            result = ddi_status_err;
            goto Exit;
        }

        /* display ObjectDesc */

        uint16_t   wNameLen                    = 0;
        char   szObName[MAX_OBNAME_LEN]    = {0};

        wNameLen = pMbxGetObDescTfer->MbxData.CoE_ObDesc.wObNameLen;
        wNameLen = (uint16_t)EC_MIN(wNameLen, MAX_OBNAME_LEN - 1);

        strncpy(szObName, pMbxGetObDescTfer->MbxData.CoE_ObDesc.pchObName, (int32_t)wNameLen);
        szObName[wNameLen] = '\0';

        COE_LOG( "%04x %s: type 0x%04x, code=0x%02x, %s, SubIds=%d\n",
            pMbxGetObDescTfer->MbxData.CoE_ObDesc.wObIndex,
            szObName,
            pMbxGetObDescTfer->MbxData.CoE_ObDesc.wDataType,
            pMbxGetObDescTfer->MbxData.CoE_ObDesc.byObjCode,
            ((pMbxGetObDescTfer->MbxData.CoE_ObDesc.byObjCategory == 0) ? "optional" : "mandatory"),
            pMbxGetObDescTfer->MbxData.CoE_ObDesc.byMaxNumSubIndex);

        /* give logging task a chance to flush */
        if (bReadingMasterOD)
            OsSleep(2);

        /* if Object is Single Variable, only subindex 0 is defined */
        if (OBJCODE_VAR == pMbxGetObDescTfer->MbxData.CoE_ObDesc.byObjCode)
        {
            wSubIndexLimit = 1;
        }
        else
        {
            wSubIndexLimit = 0x100;
        }

        /* iterate through sub-indexes */
        for (wSubIndex = 0; wSubIndex < wSubIndexLimit; wSubIndex++)
        {
            /* Get Entry Description */
            pMbxGetEntryDescTfer->dwClntId     = client_id;
            pMbxGetEntryDescTfer->dwDataLen    = pMbxGetEntryDescTfer->MbxTferDesc.dwMaxDataLen;
            pMbxGetEntryDescTfer->dwTferId     = dwUniqueTransferId++;

            ec_result = emCoeGetEntryDesc(INSTANCE_MASTER_DEFAULT, pMbxGetEntryDescTfer, dwNodeId, pwODList[wIndex], EC_LOBYTE(wSubIndex), byValueInfoType, timeout_us);
            if (EC_E_SLAVE_NOT_PRESENT == ec_result)
            {
                result = ddi_status_not_found;
                goto Exit;
            }

            /* break after last index */
            if ((EC_E_INVALIDDATA == ec_result) || (EC_E_SDO_ABORTCODE_OFFSET == ec_result))
            {
                break;
            }

            /* handle MBX Tfer errors and wait until tfer object is available */
            HandleMbxTferReqError((char*)"CoeReadObjectDictionary: Error in emCoeGetEntryDesc", ec_result, pMbxGetEntryDescTfer);

            /* display EntryDesc */

            char   szAccess[50]        = {0};
            int32_t    nAccessIdx          = 0;
            char   szPdoMapInfo[50]    = {0};
            int32_t    nDataIdx            = 0;
            char   szUnitType[50]      = {0};
            char   szDefaultValue[10]  = {0};
            char   szMinValue[10]      = {0};
            char   szMaxValue[10]      = {0};
            char   szDescription[50]   = {0};

            uint32_t  dwUnitType          = 0;
            uint8_t*  pbyDefaultValue     = EC_NULL;
            uint8_t*  pbyMinValue         = EC_NULL;
            uint8_t*  pbyMaxValue         = EC_NULL;

            memset(szAccess, 0, sizeof(szAccess));
            memset(szPdoMapInfo, 0, sizeof(szPdoMapInfo));
            memset(szUnitType, 0, sizeof(szUnitType));
            memset(szDefaultValue, 0, sizeof(szDefaultValue));
            memset(szMinValue, 0, sizeof(szMinValue));
            memset(szMaxValue, 0, sizeof(szMaxValue));
            memset(szDescription, 0, sizeof(szDescription));

            /* build Access Right String */
            if (pMbxGetEntryDescTfer->MbxData.CoE_EntryDesc.byObAccess & EC_COE_ENTRY_Access_R_PREOP)
                szAccess[nAccessIdx++] = 'R';
            else
                szAccess[nAccessIdx++] = ' ';
            if (pMbxGetEntryDescTfer->MbxData.CoE_EntryDesc.byObAccess & EC_COE_ENTRY_Access_W_PREOP)
                szAccess[nAccessIdx++] = 'W';
            else
                szAccess[nAccessIdx++] = ' ';

            szAccess[nAccessIdx++] = '.';

            if (pMbxGetEntryDescTfer->MbxData.CoE_EntryDesc.byObAccess & EC_COE_ENTRY_Access_R_SAFEOP)
                szAccess[nAccessIdx++] = 'R';
            else
                szAccess[nAccessIdx++] = ' ';
            if (pMbxGetEntryDescTfer->MbxData.CoE_EntryDesc.byObAccess & EC_COE_ENTRY_Access_W_SAFEOP)
                szAccess[nAccessIdx++] = 'W';
            else
                szAccess[nAccessIdx++] = ' ';

            szAccess[nAccessIdx++] = '.';

            if (pMbxGetEntryDescTfer->MbxData.CoE_EntryDesc.byObAccess & EC_COE_ENTRY_Access_R_OP)
                szAccess[nAccessIdx++] = 'R';
            else
                szAccess[nAccessIdx++] = ' ';
            if (pMbxGetEntryDescTfer->MbxData.CoE_EntryDesc.byObAccess & EC_COE_ENTRY_Access_W_OP)
                szAccess[nAccessIdx++] = 'W';
            else
                szAccess[nAccessIdx++] = ' ';

            if (pMbxGetEntryDescTfer->MbxData.CoE_EntryDesc.bRxPdoMapping)
            {
                strncpy(szPdoMapInfo, "-RxPDO", sizeof(szPdoMapInfo) - 1);
                if (pMbxGetEntryDescTfer->MbxData.CoE_EntryDesc.bTxPdoMapping)
                {
                    strncpy(&(szPdoMapInfo[OsStrlen(szPdoMapInfo)]), "+TxPDO", sizeof(szPdoMapInfo) - OsStrlen(szPdoMapInfo) - 1);
                }
            }

            if (pMbxGetEntryDescTfer->MbxData.CoE_EntryDesc.byValueInfo & EC_COE_ENTRY_UnitType)
            {
                dwUnitType = EC_GET_FRM_DWORD(pMbxGetEntryDescTfer->MbxData.CoE_EntryDesc.pbyData);
                snprintf(szUnitType, sizeof(szUnitType) - 1, ", UnitType 0x%08X", dwUnitType);
                nDataIdx += 4;
            }

            if (pMbxGetEntryDescTfer->MbxData.CoE_EntryDesc.byValueInfo & EC_COE_ENTRY_DefaultValue)
            {
                strncpy(szDefaultValue, ", Default", sizeof(szDefaultValue) - 1);
                pbyDefaultValue = &pMbxGetEntryDescTfer->MbxData.CoE_EntryDesc.pbyData[nDataIdx];
                nDataIdx += BIT2BYTE(pMbxGetEntryDescTfer->MbxData.CoE_EntryDesc.wBitLen);
            }

            if (pMbxGetEntryDescTfer->MbxData.CoE_EntryDesc.byValueInfo & EC_COE_ENTRY_MinValue)
            {
                strncpy(szMinValue, ", Min", sizeof(szMinValue) - 1);
                pbyMinValue = &pMbxGetEntryDescTfer->MbxData.CoE_EntryDesc.pbyData[nDataIdx];
                nDataIdx += BIT2BYTE(pMbxGetEntryDescTfer->MbxData.CoE_EntryDesc.wBitLen);
            }

            if (pMbxGetEntryDescTfer->MbxData.CoE_EntryDesc.byValueInfo & EC_COE_ENTRY_MaxValue)
            {
                strncpy(szMaxValue, ", Max", sizeof(szMaxValue) - 1);
                pbyMaxValue = &pMbxGetEntryDescTfer->MbxData.CoE_EntryDesc.pbyData[nDataIdx];
                nDataIdx += BIT2BYTE(pMbxGetEntryDescTfer->MbxData.CoE_EntryDesc.wBitLen);
            }

            if (nDataIdx + 1 <= pMbxGetEntryDescTfer->MbxData.CoE_EntryDesc.wDataLen)
            {
                snprintf(szDescription, EC_MIN((int32_t)(pMbxGetEntryDescTfer->MbxData.CoE_EntryDesc.wDataLen - nDataIdx + 1), (int32_t)(sizeof(szDescription) - 1)),
                    "%s", &pMbxGetEntryDescTfer->MbxData.CoE_EntryDesc.pbyData[nDataIdx]);
            }

            COE_LOG( "%04x:%d %s: data type=0x%04x, bit len=%02d, %s%s%s%s%s%s \n",
                pMbxGetEntryDescTfer->MbxData.CoE_EntryDesc.wObIndex,
                pMbxGetEntryDescTfer->MbxData.CoE_EntryDesc.byObSubIndex,
                szDescription,
                pMbxGetEntryDescTfer->MbxData.CoE_EntryDesc.wDataType,
                pMbxGetEntryDescTfer->MbxData.CoE_EntryDesc.wBitLen,
                szAccess, szPdoMapInfo, szUnitType, szDefaultValue, szMinValue, szMaxValue);

            EC_UNREFPARM(pbyDefaultValue);
            EC_UNREFPARM(pbyMinValue);
            EC_UNREFPARM(pbyMaxValue);

            /* give logging task a chance to flush */
            if (bReadingMasterOD)
                OsSleep(2);

            if (0 == pMbxGetEntryDescTfer->MbxData.CoE_EntryDesc.wDataType)
            {
                /* unknown datatype */
                continue;
            }

            /* SDO Upload */
            if (live_update)
            {
                uint8_t   abySDOValue[CROD_MAXSISDO_SIZE] = {0};
                uint32_t  dwUploadBytes                   = 0;

                /* get object's value */
                ec_result = emCoeSdoUpload(
                    INSTANCE_MASTER_DEFAULT, dwNodeId, pwODList[wIndex], EC_LOBYTE(wSubIndex),
                    abySDOValue, EC_MIN( (uint32_t)(sizeof(abySDOValue)), (uint32_t)(((pMbxGetEntryDescTfer->MbxData.CoE_EntryDesc.wBitLen)+7)/8) ),
                    &dwUploadBytes, timeout_us, 0
                                        );
                if (EC_E_SLAVE_NOT_PRESENT == ec_result)
                {
                    result = ddi_status_not_found;
                    goto Exit;
                }
                else if (EC_E_NOERROR != ec_result)
                {
                    /* Upload error */
                    ELOG( "CoeReadObjectDictionary: Error in ecatCoeSdoUpload: %s (0x%x)\n", ecatGetText(ec_result), ec_result);
                    result = ddi_status_err;
                    continue;
                }

                if (((OBJCODE_REC == pMbxGetObDescTfer->MbxData.CoE_ObDesc.byObjCode) && (0 == wSubIndex))
                    || ((OBJCODE_ARR == pMbxGetObDescTfer->MbxData.CoE_ObDesc.byObjCode) && (0 == wSubIndex)))
                {
                    wSubIndexLimit = (uint16_t)(((uint16_t)abySDOValue[0]) + 1);
                }

                    switch (pMbxGetEntryDescTfer->MbxData.CoE_EntryDesc.wDataType)
                    {
                    case DEFTYPE_BOOLEAN:
                    case DEFTYPE_BIT1:
                    case DEFTYPE_BIT2:
                    case DEFTYPE_BIT3:
                    case DEFTYPE_BIT4:

                    case DEFTYPE_BIT5:
                    case DEFTYPE_BIT6:
                    case DEFTYPE_BIT7:
                    case DEFTYPE_BIT8:
                    case DEFTYPE_INTEGER8:
                    case DEFTYPE_UNSIGNED8:
                        {
                            COE_LOG( "%04x:%d BYTE: 0x%02X\n",
                                pwODList[wIndex], EC_LOBYTE(wSubIndex), abySDOValue[0]);
                        } break;
                    case DEFTYPE_INTEGER16:
                        {
                            COE_LOG( "%04x:%d SI16: %04d\n",
                                pwODList[wIndex], EC_LOBYTE(wSubIndex), EC_GET_FRM_WORD(&abySDOValue[0]));
                        } break;
                    case DEFTYPE_UNSIGNED16:
                        {
                            COE_LOG( "%04x:%d WORD: 0x%04X\n",
                                pwODList[wIndex], EC_LOBYTE(wSubIndex), EC_GET_FRM_WORD(&abySDOValue[0]));
                        } break;
                    case DEFTYPE_INTEGER32:
                        {
                            COE_LOG( "%04x:%d SI32: %08d\n",
                                pwODList[wIndex], EC_LOBYTE(wSubIndex), EC_GET_FRM_DWORD(&abySDOValue[0]));
                        } break;
                    case DEFTYPE_UNSIGNED32:
                        {
                            COE_LOG( "%04x:%d DWRD: 0x%08X\n",
                                pwODList[wIndex], EC_LOBYTE(wSubIndex), EC_GET_FRM_DWORD(&abySDOValue[0]));
                        } break;
                    case DEFTYPE_VISIBLESTRING:
                        {

                            COE_LOG( "%04x:%d STRG: %s\n",
                                pwODList[wIndex], EC_LOBYTE(wSubIndex), (char*)abySDOValue);
                        } break;
                    case DEFTYPE_OCTETSTRING:
                        {
    #if (defined INCLUDE_MASTER_OBD)
                            if( (COEOBJID_HISTORY_OBJECT == (pwODList[wIndex])) && EC_LOBYTE(wSubIndex) > 5 )
                            {
                                /* Diag Entry */
                                EC_T_OBJ10F3_DIAGMSG*   pDiag = (EC_T_OBJ10F3_DIAGMSG*)abySDOValue;
                                uint32_t dwTimeStampHi = EC_HIDWORD(pDiag->qwTimeStamp);
                                uint32_t dwTimeStampLo = EC_LODWORD(pDiag->qwTimeStamp);
                                COE_LOG( "%04x:%d DIAG # 0x%x type <%s> Text 0x%x Time: 0x%x.%x",
                                    pwODList[wIndex], EC_LOBYTE(wSubIndex),
                                    pDiag->dwDiagNumber,
                                    (((pDiag->wFlags & 0x0F) == DIAGFLAGERROR) ? "ERR" : (((pDiag->wFlags & 0x0F) == DIAGFLAGWARN) ? "WARN" : (((pDiag->wFlags & 0x0F) == DIAGFLAGINFO) ? "INF" : "UNK"))),
                                    pDiag->wTextId, dwTimeStampHi, dwTimeStampLo);
                                //ParseDiagMsg(pDemoThreadParam, pDiag);
                            }
                            else
    #endif
                            {
                                COE_LOG( "%04x:%d OCTS: %s",
                                    pwODList[wIndex], EC_LOBYTE(wSubIndex), (char*)abySDOValue);
                            }
                        } break;
                    case DEFTYPE_UNSIGNED48:
                        {
                            COE_LOG( "%04x:%d US48: %02X:%02X:%02X:%02X:%02X:%02X\n",
                                pwODList[wIndex], EC_LOBYTE(wSubIndex),
                                abySDOValue[0],
                                abySDOValue[1],
                                abySDOValue[2],
                                abySDOValue[3],
                                abySDOValue[4],
                                abySDOValue[5]);
                        } break;
                    case DEFTYPE_UNSIGNED64:
                        {
                            COE_LOG( "%04x:%d QWRD: 0x%08X.%08X\n",
                                pwODList[wIndex], EC_LOBYTE(wSubIndex),
                                EC_HIDWORD(EC_GET_FRM_QWORD(&abySDOValue[0])),
                                EC_LODWORD(EC_GET_FRM_QWORD(&abySDOValue[0])));
                        } break;
                    default:
                        {
                            uint32_t  dwIdx = 0;

                            COE_LOG( "%04x:%d DFLT: \n", pwODList[wIndex], EC_LOBYTE(wSubIndex));
                            for (dwIdx = 0; dwIdx < dwUploadBytes; dwIdx++)
                            {
                                snprintf(&szLogBuffer[OsStrlen(szLogBuffer)], (int32_t)(LOG_BUFFER_SIZE - OsStrlen(szLogBuffer) - 1), "%02x ", abySDOValue[dwIdx]);
                                if ((0 != dwIdx) && (0 == (dwIdx%8)))
                                {
                                    snprintf(&szLogBuffer[OsStrlen(szLogBuffer)], (int32_t)(LOG_BUFFER_SIZE - OsStrlen(szLogBuffer) - 1), "%s", " ");
                                }
                            }
                        } break;
                    } /* switch (pMbxGetEntryDescTfer->MbxData.CoE_EntryDesc.wDataType) */

    #if (defined INCLUDE_MASTER_OBD)
                    if (COEOBJID_SLAVECFGINFOBASE <= pwODList[wIndex] && 1 == EC_LOBYTE(wSubIndex))
                    {
                        EC_T_BOOL bEntryValid   = EC_FALSE;
                        bEntryValid = EC_GET_FRM_BOOL(abySDOValue);

                        /* do not show unused Slave Entries */
                        if (!bEntryValid)
                        {
                            break;
                        }
                    }
    #endif
                    /* give logging task a chance to flush */
                    if (bReadingMasterOD)
                        OsSleep(2);
            } /* bPerformUpload */

            /* MbxGetObDescTfer done */
            pMbxGetObDescTfer->eTferStatus = eMbxTferStatus_Idle;
        } /* for (wSubIndex = 0; wSubIndex < wSubIndexLimit; wSubIndex++) */
    } /* for (wIndex = 0; (wIndex < wODListLen) && !*pbStopReading; wIndex++) */

    result = ddi_status_ok;
Exit:
    /* Delete MBX Transfer objects */
    if (EC_NULL != pMbxGetODLTfer)
    {
        emMbxTferDelete(INSTANCE_MASTER_DEFAULT, pMbxGetODLTfer);
        pMbxGetODLTfer = EC_NULL;
    }
    if (EC_NULL != pMbxGetObDescTfer)
    {
        emMbxTferDelete(INSTANCE_MASTER_DEFAULT, pMbxGetObDescTfer);
        pMbxGetObDescTfer = EC_NULL;
    }
    if (EC_NULL != pMbxGetEntryDescTfer)
    {
        emMbxTferDelete(INSTANCE_MASTER_DEFAULT, pMbxGetEntryDescTfer);
        pMbxGetEntryDescTfer = EC_NULL;
    }

    /* Free allocated memory */
    free(pwODList);
    free(pbyODLTferBuffer);
    free(pbyOBDescTferBuffer);
    free(pbyGetEntryTferBuffer);
    free(szLogBuffer);

    fclose(coe_od_fp);

    return result;
}
