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
#include <AtEthercat.h>
#include "EcError.h"
#include "EcInterfaceCommon.h"
#include "ddi_em_api.h"
#include "ddi_defines.h"
#include "ddi_debug.h"
#include "ddi_em_logging.h"

// Provides a mapping from Acontis codes and strings to DDI codes and strings
typedef struct {
  uint32_t      acontis_code;
  const char*   acontis_string;
  uint32_t      ddi_code;
  const char*   ddi_string;
} acontis_ddi_err_mapping_obj;

// Provides a mapping from Acontis event code and strings to DDI codes and strings
typedef struct {
  uint32_t          acontis_code;
  ddi_em_event_type ddi_code;
  const char*       ddi_string;
} acontis_ddi_event_mapping_obj;

typedef struct {
  ddi_em_result   ddi_code;
  const char*     ddi_string;
} ddi_err_mapping_obj;

#define ACONTIS_ERR(d) EC_E_##d, EC_SZTXT_E_##d

// This table provides a mapping from Acontis error codes to DDI error codes
static acontis_ddi_err_mapping_obj g_acontis_ddi_err_table []  = {
  { ACONTIS_ERR(NOERROR),                          DDI_EM_STATUS_OK,               "No error present"},
  { ACONTIS_ERR(ERROR),                            DDI_EM_STATUS_NOT_FOUND,        "Generic Error"},
  { ACONTIS_ERR(NOTSUPPORTED),                     DDI_EM_STATUS_NOT_SUPPORTED,    "Operation not suppported"},
  { ACONTIS_ERR(INVALIDINDEX),                     DDI_EM_STATUS_INVALID_INDEX,    "Invalid Index Entered"},
  { ACONTIS_ERR(INVALIDOFFSET),                    DDI_EM_STATUS_INVALID_OFFSET,   "Invalid Offset Entered"},
  { ACONTIS_ERR(CANCEL),                           DDI_EM_STATUS_OP_CANCELLED,     "Operation Cancelled"},
  { ACONTIS_ERR(INVALIDSIZE),                      DDI_EM_STATUS_INVALID_SIZE,     "Invalid Size"},
  { ACONTIS_ERR(INVALIDDATA),                      DDI_EM_STATUS_INVALID_DATA,     "Invalid Data"},
  { ACONTIS_ERR(NOTREADY),                         DDI_EM_STATUS_NOT_READY,        "Not Ready"},
  { ACONTIS_ERR(BUSY),                             DDI_EM_STATUS_BUSY,             "Stack or Object Busy"},
  { ACONTIS_ERR(ACYC_FRM_FREEQ_EMPTY),             DDI_EM_ACYC_QUEUE_ERR,          "Acyclic Frame Queue Error"},
  { ACONTIS_ERR(NOMEMORY),                         DDI_EM_STATUS_NO_RESOURCES,     "No resources available"},
  { ACONTIS_ERR(INVALIDPARM),                      DDI_EM_STATUS_INVALID_ARG,      "Invalid Parmeter Entered"},
  { ACONTIS_ERR(NOTFOUND),                         DDI_EM_STATUS_NOT_FOUND,        "File or Object not found"},
  { ACONTIS_ERR(DUPLICATE),                        DDI_ES_DUP_DETECTED,            "Duplicate Detected"},
  { ACONTIS_ERR(INVALIDSTATE),                     DDI_EM_STATE_ERR,               "Invalid Master State"},
  { ACONTIS_ERR(TIMER_LIST_FULL),                  DDI_ES_TIMER_FULL,              "Slave timer list full"},
  { ACONTIS_ERR(TIMEOUT),                          DDI_EM_STATUS_TIMEOUT,          "Timeout occurred"},
  { ACONTIS_ERR(OPENFAILED),                       DDI_EM_STATUS_OPEN_FAILED,      "Open operation failed"},
  { ACONTIS_ERR(SENDFAILED),                       DDI_EM_LL_SEND_FAILED,          "Link Layer send failed"},
  { ACONTIS_ERR(INSERTMAILBOX),                    DDI_EM_MB_INSERT_FAILED,        "Config: Insert Mailbox failed"},
  { ACONTIS_ERR(INVALIDCMD),                       DDI_EM_MB_INVALID_CMD,          "Invalid mailbox command code"},
  { ACONTIS_ERR(UNKNOWN_MBX_PROTOCOL),             DDI_EM_MB_PROTOCOL,             "Invalid mailbox protocol"},
  { ACONTIS_ERR(ACCESSDENIED),                     DDI_EM_ACCESS_DENIED,           "Internal Master error"},
  { ACONTIS_ERR(IDENTIFICATIONFAILED),             DDI_EM_ENI_ID_FAILED,           "ENI Identitifcation failed"},
  { ACONTIS_ERR(LOCK_CREATE_FAILED),               DDI_EM_STATUS_OS_LOCK_FAILED,   "OS lock failed"},
  { ACONTIS_ERR(PRODKEY_INVALID),                  DDI_EM_STATUS_PROD_UNLICENSED,  "Product unlicensed"},
  { ACONTIS_ERR(WRONG_FORMAT),                     DDI_EM_ENI_FORMAT_ERR,          "ENI not formatted properly"},
  { ACONTIS_ERR(FEATURE_DISABLED),                 DDI_EM_STATUS_FEATURE_DISABLED, "Feature disabled in this version of the SDK"},
  { ACONTIS_ERR(SHADOW_MEMORY),                    DDI_EM_STATUS_SHADOW_MEM_ERR,   "Shadow memory requested in wrong mode"},
  { ACONTIS_ERR(BUSCONFIG_MISMATCH),               DDI_EM_BUS_CFG_ERR,             "Bus Configuration mismatch during scan"},
  { ACONTIS_ERR(CONFIGDATAREAD),                   DDI_EM_ENI_INVALID,             "Invalid ENI file"},
  { ACONTIS_ERR(ENI_NO_SAFEOP_OP_SUPPORT),         DDI_EM_ENI_NO_OP_SAFEOP,        "No OP or SAFEOP support in ENI, but OP or SAFEOP requested"},
  { ACONTIS_ERR(XML_CYCCMDS_MISSING),              DDI_EM_ENI_NO_CYC_CMD,          "No cyclic commands present in the ENI"},
  { ACONTIS_ERR(XML_ALSTATUS_READ_MISSING),        DDI_EM_ENI_NO_ALSTATUS,         "No Alstatus read present in ENI file"},
  { ACONTIS_ERR(MCSM_FATAL_ERROR),                 DDI_EM_INTERNAL_STATE,          "Master is in an invalid internal state"},
  { ACONTIS_ERR(SLAVE_ERROR),                      DDI_ES_ERR,                     "Slave error occurred during operation"},
  { ACONTIS_ERR(FRAME_LOST),                       DDI_EM_FRAME_LOST,              "Frame lost during operation - possible wiring issue"},
  { ACONTIS_ERR(CMD_MISSING),                      DDI_EM_CYC_CMD_MISSING,         "At least one cyclic command missing"},
  { ACONTIS_ERR(CYCCMD_WKC_ERROR),                 DDI_EM_CYC_WKC_ERR,             "WKC error in cyclic frame"},
  { ACONTIS_ERR(AI_ADDRESS),                       DDI_ES_AUTO_INC_INVALID,        "Slave auto-inc address invalid"},
  { ACONTIS_ERR(INVALID_SLAVE_STATE),              DDI_ES_INTERNAL_STATE,          "Invalid Slave state for requested operation"},
  { ACONTIS_ERR(SLAVE_NOT_ADDRESSABLE),            DDI_ES_NOT_ADDRESSABLE,         "Slave lost station address"},
  { ACONTIS_ERR(CYC_CMDS_OVERFLOW),                DDI_EM_ENI_CMD_CNT_ERR,         "Too many cyclic commands in the ENI for this master version"},
  { ACONTIS_ERR(LINK_DISCONNECTED),                DDI_ES_LINK_DISCONNECTED,       "Slave link disconnected"},
  { ACONTIS_ERR(COE_MBXSND_WKC_ERROR),             DDI_EM_COE_TX_WKC_ERR,          "Master COE transmit mailbox wkc error"},
  { ACONTIS_ERR(COE_MBXRCV_WKC_ERROR),             DDI_EM_COE_RX_WKC_ERR,          "Master COE receive mailbox wkc error"},
  { ACONTIS_ERR(NO_MBX_SUPPORT),                   DDI_ES_NO_MBX_SUPPORT,          "Slave does not have mailbox support"},
  { ACONTIS_ERR(NO_COE_SUPPORT),                   DDI_EM_ENI_COE_SUPPORT,         "ENI does not have COE support"},
  { ACONTIS_ERR(NO_EOE_SUPPORT),                   DDI_EM_ENI_EOE_SUPPORT,         "ENI does not have EOE support"},
  { ACONTIS_ERR(NO_FOE_SUPPORT),                   DDI_EM_ENI_FOE_SUPPORT,         "ENI does not have FOE support"},
  { ACONTIS_ERR(NO_SOE_SUPPORT),                   DDI_EM_ENI_SOE_SUPPORT,         "ENI does not have SOE support"},
  { ACONTIS_ERR(NO_VOE_SUPPORT),                   DDI_EM_ENI_VOE_SUPPORT,         "ENI does not have VOE support"},
  { ACONTIS_ERR(EVAL_EXPIRED),                     DDI_EM_STATUS_EVAL_EXPIRED,     "Evaluation expired"},
  { ACONTIS_ERR(LICENSE_MISSING),                  DDI_EM_STATUS_INVALID_LIC,      "Invalid license format or license file missing"},
// Abort code mapping sections
  { ACONTIS_ERR(SDO_ABORTCODE_TOGGLE),                 DDI_EM_ABORTCODE_TOGGLE,        "SDO Abort: Toggle bit did not toggle"},
  { ACONTIS_ERR(SDO_ABORTCODE_TIMEOUT),                DDI_EM_ABORTCODE_TIMEOUT,       "SDO Abort: Timeout occurred"},
  { ACONTIS_ERR(SDO_ABORTCODE_CCS_SCS),                DDI_EM_ABORTCODE_CCS_SCS,       "SDO Abort: Client/server specificer uknown"},
  { ACONTIS_ERR(SDO_ABORTCODE_BLK_SIZE),               DDI_EM_ABORTCODE_BLK_SIZE,      "SDO Abort: Invalid block size"},
  { ACONTIS_ERR(SDO_ABORTCODE_SEQNO),                  DDI_EM_ABORTCODE_SEQNO,         "SDO Abort: Invalid abort code"},
  { ACONTIS_ERR(SDO_ABORTCODE_CRC),                    DDI_EM_ABORTCODE_CRC,           "SDO Abort: CRC error"},
  { ACONTIS_ERR(SDO_ABORTCODE_MEMORY),                 DDI_EM_ABORTCODE_MEMORY,        "SDO Abort: Out of memory"},
  { ACONTIS_ERR(SDO_ABORTCODE_ACCESS),                 DDI_EM_ABORTCODE_ACCESS,        "SDO Abort: Unsupported access to an object"},
  { ACONTIS_ERR(SDO_ABORTCODE_WRITEONLY),              DDI_EM_ABORTCODE_WRITEONLY,     "SDO Abort: Object is write only"},
  { ACONTIS_ERR(SDO_ABORTCODE_READONLY),               DDI_EM_ABORTCODE_WRITEONLY,     "SDO Abort: Object is read only"},
  { ACONTIS_ERR(SDO_ABORTCODE_INDEX),                  DDI_EM_ABORTCODE_INDEX,         "SDO Abort: Object does not exist in the object dictionary"},
  { ACONTIS_ERR(SDO_ABORTCODE_PDO_MAP),                DDI_EM_ABORTCODE_PDO_MAP,       "SDO Abort: Object cannot be mapped to the PDO"},
  { ACONTIS_ERR(SDO_ABORTCODE_PDO_LEN),                DDI_EM_ABORTCODE_PDO_LEN ,      "SDO Abort: The number and length of the objects to be mapped would exceed PDO length"},
  { ACONTIS_ERR(SDO_ABORTCODE_I_INCOMP),               DDI_EM_ABORTCODE_P_INCOMP,      "SDO Abort: General parameter incompatibility reason"},
  { ACONTIS_ERR(SDO_ABORTCODE_HARDWARE),               DDI_EM_ABORTCODE_HARDWARE,      "SDO Abort: Access failed due to an hardware error"},
  { ACONTIS_ERR(SDO_ABORTCODE_DATA_LENGTH_NOT_MATCH),  DDI_EM_ABORTCODE_DATA_LENGTH_NOT_MATCH, "SDO Abort: Access failed due to an hardware error"},
  { ACONTIS_ERR(SDO_ABORTCODE_DATA_LENGTH_TOO_HIGH),   DDI_EM_ABORTCODE_DATA_LENGTH_TOO_HIGH,  "SDO Abort: Data type does not match, length of service parameter too high"},
  { ACONTIS_ERR(SDO_ABORTCODE_DATA_LENGTH_TOO_LOW),    DDI_EM_ABORTCODE_DATA_LENGTH_TOO_LOW,   "SDO Abort: Data type does not match, length of service parameter too low"},
  { ACONTIS_ERR(SDO_ABORTCODE_OFFSET),                 DDI_EM_ABORTCODE_OFFSET,        "SDO Abort: Sub-index does not exist"},
  { ACONTIS_ERR(SDO_ABORTCODE_VALUE_RANGE),            DDI_EM_ABORTCODE_VALUE_RANGE,   "SDO Abort: Value range of parameter exceeded (only for write access)"},
  { ACONTIS_ERR(SDO_ABORTCODE_VALUE_TOO_HIGH),         DDI_EM_ABORTCODE_VALUE_TOO_HIGH,"SDO Abort: Value of parameter written too high"},
  { ACONTIS_ERR(SDO_ABORTCODE_VALUE_TOO_LOW),          DDI_EM_ABORTCODE_VALUE_TOO_LOW, "SDO Abort: Value of parameter written too low "},
  { ACONTIS_ERR(SDO_ABORTCODE_MINMAX),                 DDI_EM_ABORTCODE_MINMAX,        "SDO Abort: Maximum value is less than minimum value"},
  { ACONTIS_ERR(SDO_ABORTCODE_GENERAL),                DDI_EM_ABORTCODE_GENERAL,       "SDO Abort: General Error"},
  { ACONTIS_ERR(SDO_ABORTCODE_TRANSFER),               DDI_EM_ABORTCODE_TRANSFER,      "SDO Abort: Data cannot be transferred or stored to the application"},
  { ACONTIS_ERR(SDO_ABORTCODE_TRANSFER_LOCAL_CONTROL), DDI_EM_ABORTCODE_TRANSFER_LOCAL_CONTROL, "SDO Abort: Data cannot be transferred or stored to the application because of local control"},
  { ACONTIS_ERR(SDO_ABORTCODE_TRANSFER_DEVICE_STATE),  DDI_EM_ABORTCODE_TRANSFER_DEVICE_STATE, "SDO Abort: Data cannot be transferred or stored to the application because of the present device state"},
  { ACONTIS_ERR(SDO_ABORTCODE_DICTIONARY),             DDI_EM_ABORTCODE_DICTIONARY,    "SDO Abort: Object dictionary dynamic generation"},
  { ACONTIS_ERR(SDO_ABORTCODE_MODULE_ID_LIST_NOT_MATCH), DDI_EM_ABORTCODE_MODULE_ID_LIST_NOT_MATCH, " Detected Module Ident List (0xF030) and Configured Module Ident list (0xF050) does not match"},
  { ACONTIS_ERR(SDO_ABORTCODE_SI_NOT_WRITTEN),         DDI_EM_ABORTCODE_SI_NOT_WRITTEN,   "Subindex cannot be written, SI0 must be 0 for write"},
  { ACONTIS_ERR(FOE_ERRCODE_NOTDEFINED),           DDI_ES_FOE_UNSPECIFIED,         "Unspecified FoE error"},
  { ACONTIS_ERR(FOE_ERRCODE_NOTFOUND),             DDI_ES_FOE_NOT_FOUND,           "FoE file not found"},
  { ACONTIS_ERR(FOE_ERRCODE_ACCESS),               DDI_ES_FOE_ACCESS_DENIED,       "FoE Access Denied"},
  { ACONTIS_ERR(FOE_ERRCODE_DISKFULL),             DDI_ES_FOE_DISK_FULL,           "FoE slave disk full"},
  { ACONTIS_ERR(FOE_ERRCODE_ILLEGAL),              DDI_ES_FOE_ILLEGAL_OP,          "FoE illegal operation"},
  { ACONTIS_ERR(FOE_ERRCODE_PACKENO),              DDI_ES_FOE_PACKET_NO,           "FoE incorrect packet number received"},
  { ACONTIS_ERR(FOE_ERRCODE_EXISTS),               DDI_ES_FOE_ALREADY_EXISTS,      "FoE file already exists"},
  { ACONTIS_ERR(FOE_ERRCODE_NOUSER),               DDI_ES_FOE_NO_USER,             "FoE user error"},
  { ACONTIS_ERR(FOE_ERRCODE_BOOTSTRAPONLY),        DDI_ES_FOE_BOOTSTRAP_ONLY,      "FoE operations must take place while slave is in BOOT mode"},
  { ACONTIS_ERR(FOE_ERRCODE_NOTINBOOTSTRAP),       DDI_ES_FOE_NOT_BOOTSTRAP,       "FoE Downloaded filename not valid in bootstrap"},
  { ACONTIS_ERR(FOE_ERRCODE_INVALIDPASSWORD),      DDI_ES_FOE_PASSWORD,            "FoE invalid password"},
  { ACONTIS_ERR(FOE_ERRCODE_PROGERROR),            DDI_ES_FOE_PROG_ERROR,          "FoE progress error"},
  { ACONTIS_ERR(FOE_ERRCODE_INVALID_CHECKSUM),     DDI_ES_FOE_CHECKSUM,            "FoE invalid checksum"},
  { ACONTIS_ERR(FOE_ERRCODE_INVALID_FIRMWARE),     DDI_ES_FOE_INVALID_FW,          "FoE invalid firmware"},
  { ACONTIS_ERR(FOE_ERRCODE_NO_FILE),              DDI_ES_FOE_READ_FILE,           "FoE no install file found"},
  { ACONTIS_ERR(CFGFILENOTFOUND),                  DDI_EM_ENI_INVALID,             "Invalid master ENI file"},
  { ACONTIS_ERR(EEPROMREADERROR),                  DDI_ES_EEPROM_RD_ERR,           "Error reading EEPROM"},
  { ACONTIS_ERR(EEPROMWRITEERROR),                 DDI_ES_EEPROM_WR_ERR,           "Error writing EEPROM"},
  { ACONTIS_ERR(XML_CYCCMDS_SIZEMISMATCH),         DDI_EM_ENI_CYC_CMD_SIZE,        "ENI: Cyclic command size mismatch"},
  { ACONTIS_ERR(XML_INVALID_INP_OFF),              DDI_EM_ENI_INPUT_OFFSET,        "ENI: Cyclic command input offset incorrect"},
  { ACONTIS_ERR(XML_INVALID_OUT_OFF),              DDI_EM_ENI_OUTPUT_OFFSET,       "ENI: Cyclic command output offset incorrect"},
  { ACONTIS_ERR(PORTCLOSE),                        DDI_EM_PORT_CLOSE_ERR,          "Error closing port"},
  { ACONTIS_ERR(PORTOPEN),                         DDI_EM_PORT_OPEN_ERR,           "Error opening port"},
  { ACONTIS_ERR(SLAVE_NOT_PRESENT),                DDI_ES_NOT_PRESENT,             "Slave not present on the network"},
  { ACONTIS_ERR(NO_FOE_SUPPORT_BS),                DDI_ES_FOE_NOT_SUPPORTED,       "FoE not supported although the slave is in BOOT mode"},
  { ACONTIS_ERR(EEPROMRELOADERROR),                DDI_ES_EEPROM_RELOAD_ERR,       "EEPROM reload error"},
  { ACONTIS_ERR(SLAVECTRLRESETERROR),              DDI_ES_RESET_ERR,               "Error occurred during slave reset"},
  { ACONTIS_ERR(SYSDRIVERMISSING),                 DDI_EM_SYSTEM_DRIVER_ERR,       "Error occurred while loading system driver"},
  { ACONTIS_ERR(BUSCONFIG_TOPOCHANGE),             DDI_EM_SCAN_BUS_CHANGE,         "Bus topology changed while scan in progress"},
  { ACONTIS_ERR(FOE_MBX_WKC_ERROR),                DDI_ES_FOE_WKC_ERROR,           "FoE WKC error"},
  { ACONTIS_ERR(EEPROMASSIGNERROR),                DDI_ES_EEPROM_ASSIGN_ERR,       "EEPROM assignment to slave or Ecat Master"},
  { ACONTIS_ERR(MBX_ERROR_TYPE),                   DDI_ES_MBX_ERR,                 "Slave Mailbox Unknown Error Code"},
  { ACONTIS_ERR(XML_PREV_PORT_MISSING),            DDI_EM_ENI_PREV_PORT,           "ENI: Prev port missing"},
  { ACONTIS_ERR(MBX_CMD_WKC_ERROR),                DDI_ES_MBX_WKC_ERR,             "Slave Mailbox Working Count Error"},
  { ACONTIS_ERR(MAX_BUS_SLAVES_EXCEEDED),          DDI_EM_MAX_SLAVES_ERR,          "Maximum number of slaves exceeded"},
  { ACONTIS_ERR(MBXERR_SYNTAX),                    DDI_ES_MBX_HEADER_ERR,          "Slave mailbox: Header incorrect"},
  { ACONTIS_ERR(MBXERR_UNSUPPORTEDPROTOCOL),       DDI_ES_MBX_PROTOCOL_ERR,        "Slave mailbox: Protocol error"},
  { ACONTIS_ERR(MBXERR_INVALIDCHANNEL),            DDI_ES_MBX_CHANNEL_ERR,         "Slave mailbox: Invalid channel"},
  { ACONTIS_ERR(MBXERR_SERVICENOTSUPPORTED),       DDI_ES_MBX_SERVICE_ERR,         "Slave mailbox: Service not supported"},
  { ACONTIS_ERR(MBXERR_INVALIDHEADER),             DDI_ES_MBX_PROTO_HEADER,        "Slave mailbox: The mailbox protocol header of the mailbox protocol is wrong"},
  { ACONTIS_ERR(MBXERR_SIZETOOSHORT),              DDI_ES_MBX_SHORT_SIZE,          "Slave mailbox: The mailbox data was too short"},
  { ACONTIS_ERR(MBXERR_NOMOREMEMORY),              DDI_ES_MBX_RESOURCES,           "Slave mailbox: The request cannot be processed due to limited resources"},
  { ACONTIS_ERR(MBXERR_INVALIDSIZE),               DDI_ES_MBX_SIZE_ERR,            "Slave mailbox: The length of the data is inconsistent"},
  { ACONTIS_ERR(FOE_ERRCODE_MAX_FILE_SIZE),        DDI_ES_FOE_FILE_SIZE_ERR,       "FoE: File size error"},
  { ACONTIS_ERR(FOE_ERRCODE_FILE_HEAD_MISSING),    DDI_ES_FOE_HEADER_MISSING,      "FoE: File header missing"},
  { ACONTIS_ERR(FOE_ERRCODE_FLASH_PROBLEM),        DDI_ES_FOE_FLASH_PROBLEM,       "FoE: Issue with hardware device flash"},
  { ACONTIS_ERR(FOE_ERRCODE_FILE_INCOMPATIBLE),    DDI_ES_FOE_FILE_INVALID,        "Foe: File Incompatible with current firmware"},
  { ACONTIS_ERR(FRAMELOSS_AFTER_SLAVE),            DDI_ES_LOST_COMM_ERR,           "Slave: Communication lost after port opened"},
  { ACONTIS_ERR(PDIWATCHDOG),                      DDI_ES_PDI_WATCHDOG,            "Slave: PDI watchdog triggered"},
  { ACONTIS_ERR(BAD_CONNECTION),                   DDI_ES_BAD_CONNECTION,          "Slave: Poor network connection detected"},
};

#define ACONTIS_NOTIFY(d) EC_NOTIFY_##d

acontis_ddi_event_mapping_obj g_acontis_ddi_event_table []  = {
  { ACONTIS_NOTIFY(GENERIC),                       DDI_EM_EVENT_GENERIC,                 "Generic Notification Received"},
  { ACONTIS_NOTIFY(ERROR),                         DDI_EM_EVENT_ERR,                     "Error during notification"},
  { ACONTIS_NOTIFY(STATECHANGED),                  DDI_EM_EVENT_STATE_CHANGED,           "Master state changed"},
  { ACONTIS_NOTIFY(ETH_LINK_CONNECTED),            DDI_EM_EVENT_LINK_CONNECTED,          "EtherCAT link connected"},
  { ACONTIS_NOTIFY(SB_STATUS),                     DDI_EM_EVENT_SCAN_NETWORK_COMPLETE,   "Network scan completed"},
  { ACONTIS_NOTIFY(SLAVE_STATECHANGED),            DDI_ES_EVENT_STATE_CHANGED,           "A slave state was changed"},
  { ACONTIS_NOTIFY(SLAVES_STATECHANGED),           DDI_ES_EVENT_MULT_STATES_CHANGED,     "Multiple slave states were changed"},
  { ACONTIS_NOTIFY(RAWCMD_DONE),                   DDI_EM_EVENT_RAW_CMD_COMPLETE,        "Raw EtherCAT command completed"},
  { ACONTIS_NOTIFY(SLAVE_PRESENCE),                DDI_ES_EVENT_PRESENCE,                "New slave presence on the network was detected"},
  { ACONTIS_NOTIFY(SLAVES_PRESENCE),               DDI_ES_EVENT_MULTIPLE_PRESENCE,       "New multiple slaves presence on the network was detected"},
  { ACONTIS_NOTIFY(SLAVE_REGISTER_TRANSFER),       DDI_ES_EVENT_REG_TRANSFER_COMPLETE,   "Slave register transfer complete"},
  { ACONTIS_NOTIFY(EEPROM_OPERATION),              DDI_ES_EVENT_EEPROM_OP_COMPLETE,      "Slave EEPROM operation completed or timeout"},
  { ACONTIS_NOTIFY(PORT_OPERATION),                DDI_ES_EVENT_PORT_OP_COMPLETE,        "Slave port operation completed or timeout"},
  { ACONTIS_NOTIFY(SLAVE_IDENTIFICATION),          DDI_ES_EVENT_ID_CMD_COMPLETE,         "slave port id completed or timeout"},
  { ACONTIS_NOTIFY(COE_INIT_CMD),                  DDI_EM_EVENT_COE_INIT_CMD_COMPLETE,   "CoE init command completed during state transition"},
  { ACONTIS_NOTIFY(COE_TX_PDO),                    DDI_ES_EVENT_COE_TX_PDO,              "CoE Tx PDO notification received"},
  { ACONTIS_NOTIFY(RAWMBX_DONE),                   DDI_EM_EVENT_RAW_MBX_COMPLETE,        "Raw EtherCAT mailbox transfer completed"},
  { ACONTIS_NOTIFY(CYCCMD_WKC_ERROR),              DDI_EM_EVENT_ERR_CYC_WKC,             "Cyclic frame working count (WKC) error "},
  { ACONTIS_NOTIFY(MASTER_INITCMD_WKC_ERROR),      DDI_EM_EVENT_ERR_INIT_CMD,            "WKC error occurred during master initialization"},
  { ACONTIS_NOTIFY(SLAVE_INITCMD_WKC_ERROR),       DDI_ES_EVENT_ERR_INIT_CMD,            "WKC error occurred during slave initialization"},
  { ACONTIS_NOTIFY(COE_MBXSND_WKC_ERROR),          DDI_ES_EVENT_ERR_COE_WKC,             "WKC error occurred during CoE transfer"},
  { ACONTIS_NOTIFY(FOE_MBXSND_WKC_ERROR),          DDI_ES_EVENT_ERR_FOE_WKC,             "WKC error occurred during FoE transfer"},
  { ACONTIS_NOTIFY(FRAME_RESPONSE_ERROR),          DDI_ES_EVENT_ERR_FRAME_RESPONSE,      "Invalid frame response detected"},
  { ACONTIS_NOTIFY(SLAVE_INITCMD_RESPONSE_ERROR),  DDI_ES_EVENT_ERR_INIT_CMD_RESPONSE,   "Invalid slave response during initialization"},
  { ACONTIS_NOTIFY(MASTER_INITCMD_RESPONSE_ERROR), DDI_EM_EVENT_ERR_INIT_CMD_RESPONSE,   "Invalid master response during initialization"},
  { ACONTIS_NOTIFY(MBSLAVE_INITCMD_TIMEOUT),       DDI_ES_EVENT_ERR_INIT_CMD_TIMEOUT,    "Timeout occurred during slave initialization"},
  { ACONTIS_NOTIFY(NOT_ALL_DEVICES_OPERATIONAL),   DDI_EM_EVENT_ERR_NOT_ALL_SLAVES_IN_OP,"Not all slaves are in OP mode while process data is active"},
  { ACONTIS_NOTIFY(ETH_LINK_NOT_CONNECTED),        DDI_EM_EVENT_ERR_LINK_DISCONNECTED,   "EtherCAT link disconnected"},
  { ACONTIS_NOTIFY(STATUS_SLAVE_ERROR),            DDI_ES_EVENT_ERR_ERROR_STATE,         "Slave is in error state"},
// Additional slave info not available in v1.0
  { ACONTIS_NOTIFY(SLAVE_NOT_ADDRESSABLE),         DDI_ES_EVENT_ERR_ADDRESSABLE,         "Slave is not addressable"},
  { ACONTIS_NOTIFY(MBSLAVE_COE_SDO_ABORT),         DDI_ES_EVENT_ERR_COE_SDO_ABORT,       "Slave encountered an SDO abort during CoE operation"},
  { ACONTIS_NOTIFY(CLIENTREGISTRATION_DROPPED),    DDI_EM_EVENT_ERR_CLIENT_DROPPED,      "Client registration dropped due to re-initialization of the master"},
  { ACONTIS_NOTIFY(FOE_MBSLAVE_ERROR),             DDI_ES_EVENT_ERR_FOE_MBX,             "Mailbox returned error during FoE"},
  { ACONTIS_NOTIFY(MBXRCV_INVALID_DATA),           DDI_ES_EVENT_ERR_INVALID_MBX_DATA,    "Invalid mailbox data received"},
  { ACONTIS_NOTIFY(PDIWATCHDOG),                   DDI_ES_EVENT_ERR_PDI_WATCHDOG,        "PDI watchdog error occurred"},
  { ACONTIS_NOTIFY(SLAVE_NOTSUPPORTED),            DDI_EM_EVENT_ERR_SLAVE_NOT_SUPPORTED, "Unsupported master detected during bus scan"},
  { ACONTIS_NOTIFY(SLAVE_UNEXPECTED_STATE),        DDI_ES_EVENT_ERR_UNEXPECTED_STATE,    "Slave is in an unexpected state"},
  { ACONTIS_NOTIFY(ALL_DEVICES_OPERATIONAL),       DDI_EM_EVENT_ERR_ALL_SLAVES_IN_OP,    "All devices back in OP after DDI_EM_EVENT_ERR_NOT_ALL_SLAVES_IN_OP"},
  { ACONTIS_NOTIFY(EEPROM_CHECKSUM_ERROR),         DDI_ES_EVENT_ERR_EEPROM_CHECKSUM_ERROR,"Slave has EEPROM checksum error"},
  { ACONTIS_NOTIFY(LINE_CROSSED),                  DDI_ES_EVENT_ERR_LINES_CROSSED,       "EtherCAT lines are crosssed"},
  { ACONTIS_NOTIFY(SLAVES_UNEXPECTED_STATE),       DDI_ES_EVENT_ERR_MULT_UNEXPECTED_STATE,"Multiple Slaves are in an unexpected state"},
  { ACONTIS_NOTIFY(SLAVES_ERROR_STATUS),           DDI_ES_EVENT_ERR_MULT_ERROR_STATE,    "Multiple Slaves are in an error state"},
  { ACONTIS_NOTIFY(FRAMELOSS_AFTER_SLAVE),         DDI_ES_EVENT_ERR_FRAMELOSS_AFTER_OPEN,"Frameloss occurred due to port open"},
  { ACONTIS_NOTIFY(BAD_CONNECTION),                DDI_ES_EVENT_ERR_POOR_CONNECTION,     "Poor EtherCAT connection detected"},
  { ACONTIS_NOTIFY(SB_MISMATCH),                   DDI_EM_EVENT_ERR_SCAN_MISMATCH,       "Mismatch during network scan"},
#ifdef DDI_EM_SUPPORT_HC // Unsupported feature in v1.0
  { ACONTIS_NOTIFY(HC_SLAVE_PART),                 0, "txt"},
  { ACONTIS_NOTIFY(HOTCONNECT),                    0, "txt"},
  { ACONTIS_NOTIFY(HC_DETECTADDGROUPS),            0, "txt"},
  { ACONTIS_NOTIFY(HC_PROBEALLGROUPS),             0, "txt"},
  { ACONTIS_NOTIFY(HC_TOPOCHGDONE),                0, "txt"},
  { ACONTIS_NOTIFY(HC_SLAVE_JOIN),                 0, "txt"},
  { ACONTIS_NOTIFY(SB_DUPLICATE_HC_NODE),          0, "txt"},
  { ACONTIS_NOTIFY(SLAVE_DISAPPEARS),              0, "txt"},
  { ACONTIS_NOTIFY(SLAVE_APPEARS),                 0, "txt"},
#endif
#ifdef DDI_EM_SUPPORT_SOE // Unsupported feature in v1.0
  { ACONTIS_NOTIFY(SOE_MBXSND_WKC_ERROR),          0, "txt"},
  { ACONTIS_NOTIFY(SOE_WRITE_ERROR),               0, "txt"},
#endif
#ifdef DDI_EM_SUPPORT_VOE
  { ACONTIS_NOTIFY(VOE_MBXSND_WKC_ERROR),          0, "txt"},
#endif
#ifdef DDI_EM_SUPPORT_S2S // Slave-to-slave, Unsupported feature in v1.0
  { ACONTIS_NOTIFY(S2SMBX_ERROR),                  0, "txt"},
#endif
#if DDI_EM_SUPPORT_EOE // Unsupported feature in v1.0
//  { ACONTIS_NOTIFY(EOE_MBXSND_WKC_ERROR),          0, "txt"},
#endif
#if DDI_EM_SUPPORT_REDUNDANCY  // Unsupported feature in v1.0
  { ACONTIS_NOTIFY(REFCLOCK_PRESENCE),             0, "txt"},
  { ACONTIS_NOTIFY(MASTER_RED_STATECHANGED),       0, "txt"},
  { ACONTIS_NOTIFY(MASTER_RED_FOREIGN_SRC_MAC),    0, "txt"},
  { ACONTIS_NOTIFY(RED_LINEBRK),                   0, "txt"},
  { ACONTIS_NOTIFY(JUNCTION_RED_CHANGE),           0, "txt"},
  { ACONTIS_NOTIFY(RED_LINEFIXED),                 0, "txt"},
#endif
#ifdef DDI_EM_SUPPORT_DC // Unsupported feature in v1.0
  { ACONTIS_NOTIFY(DC_STATUS),                     0, "txt"},
  { ACONTIS_NOTIFY(DC_SLV_SYNC),                   0, "txt"},
  { ACONTIS_NOTIFY(DCL_STATUS),                    0, "txt"},
  { ACONTIS_NOTIFY(DCM_SYNC),                      0, "txt"},
  { ACONTIS_NOTIFY(DCX_SYNC),                      0, "txt"},
#endif
};

// This structure maps DDI errors that are not covered in the DDI->Acontis table
static ddi_err_mapping_obj g_ddi_err_table []  = {
  { DDI_EM_SDK_NOT_INITIALIZED,     "The SDK is not initalized"},
  { DDI_EM_STATUS_NO_RESOURCES,     "Out of memory"},
  { DDI_EM_STATUS_INVALID_ARG,      "An invalid argument was passed to the SDK"},
  { DDI_EM_STATUS_FILE_OPEN_ERR,    "A file open error occurred"},
  { DDI_EM_STATUS_LOG_OPEN_ERR,     "A log file open error occurred \n"},
  { DDI_EM_STATUS_LOG_OPEN_ERR,     "A log file open error occurred \n"},
  { DDI_EM_STATUS_NOT_SUPPORTED,    "This operation was not supported \n"},
  { DDI_EM_STATUS_INVALID_INDEX,    "An invalid EtherCAT index was passed to the SDK\n"},
  { DDI_EM_STATUS_INVALID_OFFSET,   "An invalid EtherCAT offset was passed to the SDK\n"},
  { DDI_EM_STATUS_LOG_DIR_FAILED,   "Log directory creation failed \n"},
  { DDI_EM_STATUS_NULL_ARGUMENT,    "A NULL argument was passed to the SDK\n"},
  { DDI_EM_STATUS_INVALID_NOTIFY,   "An unsupported notification event occurred \n"},
  { DDI_EM_STATUS_CPU_AFFINITY_ERR, "An an issue with CPU affinity selection occurred \n"},
  { DDI_EM_STATUS_CYC_THREAD_STOP,  "An error stopping the cyclic thread occurred \n"},
  { DDI_EM_STATUS_NON_ROOT_USER,    "This SDK was initialized using a non-administrative user\n"},
  { DDI_ES_EEPROM_BUSY,             "The slave ECAT EEPROM is busy, operation failed\n"},
  { DDI_ES_MAX_NUM_OF_FUSIONS_ERR,  "The maximum number of Fusion slaves has been exceeded \n"}
};

// Translate from Acontis -> DDI error codes
// Log the notification code and corresponding string in the persistent log
ddi_em_result translate_ddi_acontis_err_code (ddi_em_handle em_handle, uint32_t acontis_code)
{
  int count;
  int array_count = ARRAY_ELEMENTS(g_acontis_ddi_err_table);
  for (count = 0; count < array_count; count++)
  {
    if (g_acontis_ddi_err_table[count].acontis_code == acontis_code)
    {
      ELOG(em_handle, "Master[%d] error: 0x%04x = (%s) \n", em_handle,g_acontis_ddi_err_table[count].ddi_code,g_acontis_ddi_err_table[count].ddi_string);
      return (ddi_em_result)g_acontis_ddi_err_table[count].ddi_code;
    }
  }
  return DDI_EM_STATUS_INVALID_ARG;
}

// Translate from Acontis -> DDI notification codes
// Optionally log the notification code and corresponding string in the persistent log
ddi_em_result translate_acontis_ddi_event_code (ddi_em_handle em_handle, ddi_es_handle es_handle , uint32_t acontis_code, ddi_em_event *event)
{
  int count;
  int array_count = ARRAY_ELEMENTS(g_acontis_ddi_event_table);
  // Search the notification table for a match for the acontis_code argument
  for ( count = 0; count < array_count; count++)
  {
    if ( g_acontis_ddi_event_table[count].acontis_code == acontis_code )
    {
      // Set the event response information back to the application
      event->master_handle = em_handle;
      event->es_handle = es_handle;
      event->event_code = g_acontis_ddi_event_table[count].ddi_code;
      // Set the notification string pointer to the table entry, force non-constant to map to a pointer
      event->event_str = g_acontis_ddi_event_table[count].ddi_string;
      DLOG(em_handle, "Master[%d] notification: 0x%04x = (%s) \n", em_handle,\
        g_acontis_ddi_event_table[count].ddi_code,g_acontis_ddi_event_table[count].ddi_string);
      return DDI_EM_STATUS_OK;
    }
  }
  // No matching notification detected
  return DDI_EM_STATUS_INVALID_NOTIFY;
}

// Translate from DDI -> Acontis notification codes
ddi_em_result translate_ddi_acontis_event_code (ddi_em_event_type ddi_event_code, uint32_t *acontis_code)
{
  int count;
  int array_count = ARRAY_ELEMENTS(g_acontis_ddi_event_table);
  if ( acontis_code == NULL )
  {
    return DDI_EM_STATUS_NULL_ARGUMENT;
  }
  // Search the error table for a match for the acontis_code argument
  for ( count = 0; count < array_count; count++)
  {
    if ( g_acontis_ddi_event_table[count].ddi_code == ddi_event_code )
    {
      // Set the acontis code that matches the given DDI code and return DDI_EM_STATUS_OK
      *acontis_code = g_acontis_ddi_event_table[count].acontis_code;
      return DDI_EM_STATUS_OK;
    }
  }
  // No matching notification detected
  return DDI_EM_STATUS_INVALID_NOTIFY;
}

// Translate from a DDI error to a DDI string
// Optionally log the notification code and corresponding string in the persistent log
const char* ddi_em_get_error_string (ddi_em_result ddi_em_result)
{
  int count;
  int array_count = ARRAY_ELEMENTS(g_acontis_ddi_err_table);
  // First search through the Acontis->DDI error mappings
  for ( count = 0; count < array_count; count++)
  {
    if ( g_acontis_ddi_err_table[count].ddi_code == ddi_em_result )
    {
      // Log the notification as an error message
      return g_acontis_ddi_err_table[count].ddi_string;
    }
  }
  // Then search through any DDI-specific error messages
  array_count = ARRAY_ELEMENTS(g_ddi_err_table);
  for ( count = 0; count < array_count; count++)
  {
    if ( g_ddi_err_table[count].ddi_code == ddi_em_result )
    {
      // Log the notification as an error message
      return g_ddi_err_table[count].ddi_string;
    }
  }  
  return "Unsupported error code passed as an argument"; // Unsupported error Code
}
