/**************************************************************************
(c) Copyright 2021-2022 Digital Dynamics Inc. Scotts Valley CA USA.
Unpublished copyright. All rights reserved. Contains proprietary and
confidential trade secrets belonging to DDI. Disclosure or release without
prior written authorization of DDI is prohibited.
**************************************************************************/

#ifndef DDI_EM_API_H
#define DDI_EM_API_H

/// @file ddi_em_api.h

#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include "ddi_em_platform.h"

// The ddi_em_platform.h file adds support for platform-specific features
// The ddi_em_platform.h file should be soft-linked to one of the files in the hw_platforms directory
// The ddi_em_platform.h is linked to the ddi_em_supermicro_e300-8d.h (SYS-E300-8D) file by default

#ifdef __cplusplus
extern "C" {
#endif

/*! @var DDI_EM_VERSION
  @brief DDI EtherCAT Master Version
*/
#define DDI_EM_VERSION "1.3.0"

/*! @var DDI_EM_TRUE
  @brief Defines true value for the DDI EtherCAT Master SDK
*/
#define DDI_EM_TRUE 1

/*! @var DDI_EM_FALSE
  @brief Defines false value for the DDI EtherCAT Master SDK
*/
#define DDI_EM_FALSE 0

/*! @enum ddi_em_result
  @brief Result enumeration

  This enumeration provides the basic return types for the DDI ECAT Master SDK.
  The error codes are broken up into several sections:
  0-0xFFFF         : General purpose SDK error codes not related to the SDK
  0x10000-0x1FFFF  : Master error codes
  0x20000-0x2FFFF  : Slave error codes
*/
typedef enum {
  DDI_EM_STATUS_OK                = 0,       /**< @brief Operation successful */
  // Common errors to both Master and Slaves
  DDI_EM_SDK_NOT_INITIALIZED      = 0x00001, /**< @brief SDK not initialized  */
  DDI_EM_STATUS_NO_RESOURCES      = 0x00002, /**< @brief Lack of system resources, e.g. out of memory */
  DDI_EM_STATUS_INVALID_ARG       = 0x00003, /**< @brief Invalid argument passed to the SDK */
  DDI_EM_STATUS_FILE_OPEN_ERR     = 0x00004, /**< @brief Error opening a file */
  DDI_EM_STATUS_LOG_OPEN_ERR      = 0x00005, /**< @brief Error opening a log file */
  DDI_EM_STATUS_NOT_SUPPORTED     = 0x00006, /**< @brief This operation is not supported */
  DDI_EM_STATUS_INVALID_INDEX     = 0x00007, /**< @brief Invalid EtherCAT index */
  DDI_EM_STATUS_INVALID_OFFSET    = 0x00008, /**< @brief Invalid EtherCAT offset */
  DDI_EM_STATUS_OP_CANCELLED      = 0x00009, /**< @brief Operation cancelled */
  DDI_EM_STATUS_INVALID_SIZE      = 0x0000A, /**< @brief Invalid size */
  DDI_EM_STATUS_INVALID_DATA      = 0x0000B, /**< @brief Invalid data */
  DDI_EM_STATUS_NOT_READY         = 0x0000C, /**< @brief System not ready */
  DDI_EM_STATUS_BUSY              = 0x0000D, /**< @brief System busy */
  DDI_EM_STATUS_NOT_FOUND         = 0x0000E, /**< @brief Resource not found */
  DDI_EM_STATUS_TIMEOUT           = 0x0000F, /**< @brief Timeout occurred */
  DDI_EM_STATUS_OPEN_FAILED       = 0x00010, /**< @brief Open failed */
  DDI_EM_STATUS_LOG_DIR_FAILED    = 0x00011, /**< @brief Creation of the log directory failed */
  DDI_EM_STATUS_OS_LOCK_FAILED    = 0x00012, /**< @brief Operating system locking failed */
  DDI_EM_STATUS_PROD_UNLICENSED   = 0x00013, /**< @brief Product unlicensed */
  DDI_EM_STATUS_FEATURE_DISABLED  = 0x00014, /**< @brief Requested feature is disabled */
  DDI_EM_STATUS_SHADOW_MEM_ERR    = 0x00015, /**< @brief Shadow memory error */
  DDI_EM_STATUS_EVAL_EXPIRED      = 0x00016, /**< @brief Evaluation expired */
  DDI_EM_STATUS_INVALID_LIC       = 0x00017, /**< @brief Invalid license file */
  DDI_EM_STATUS_INVALID_INSTANCE  = 0x00018, /**< @brief Invalid instance received */
  DDI_EM_STATUS_NULL_ARGUMENT     = 0x00019, /**< @brief Required operation argument was null */
  DDI_EM_STATUS_INVALID_NOTIFY    = 0x0001A, /**< @brief Invalid event event */
  DDI_EM_STATUS_CPU_AFFINITY_ERR  = 0x0001B, /**< @brief Setting CPU affinity failed */
  DDI_EM_STATUS_CYC_THREAD_STOP   = 0x0001C, /**< @brief Error stopping cyclic thread */
  DDI_EM_STATUS_NON_ROOT_USER     = 0x0001D, /**< @brief User initialized this SDK as a non-root user */
  // Master-specific errors
  DDI_EM_INIT_ERR                 = 0x10000, /**< @brief Error occurred during master initialization */
  DDI_EM_STATE_ERR                = 0x10001, /**< @brief Error occurred during state transition */
  DDI_EM_EVENT_REG_ERR            = 0x10002, /**< @brief Error occurred during event registration */
  DDI_EM_NIC_ALREADY_REG          = 0x10003, /**< @brief Network interface card already registered to another instance */
  DDI_EM_CONFIG_ERR               = 0x10004, /**< @brief Error occurred during master configuration */
  DDI_EM_ACYC_QUEUE_ERR           = 0x10005, /**< @brief Error occurred with the acyclic queue */
  DDI_EM_SCAN_REV_MISMATCH        = 0x10006, /**< @brief Revision mismatch during network scan */
  DDI_EM_LL_SEND_FAILED           = 0x10007, /**< @brief Link layer send failed */
  DDI_EM_MB_INSERT_FAILED         = 0x10008, /**< @brief Insertion of mailbox failed */
  DDI_EM_MB_INVALID_CMD           = 0x10009, /**< @brief Invalid Mailbox command */
  DDI_EM_MB_PROTOCOL              = 0x1000A, /**< @brief Invalid Mailbox protocol */
  DDI_EM_ACCESS_DENIED            = 0x1000B, /**< @brief Access was denied */
  DDI_EM_ENI_ID_FAILED            = 0x1000C, /**< @brief Identification failed */
  DDI_EM_ENI_FORMAT_ERR           = 0x1000D, /**< @brief ENI has a formatting error */
  DDI_EM_ENI_INVALID              = 0x1000E, /**< @brief ENI does not exist */
  DDI_EM_ENI_NO_OP_SAFEOP         = 0x1000F, /**< @brief OP requested, but does not exist in ENI */
  DDI_EM_ENI_NO_CYC_CMD           = 0x10010, /**< @brief No cyclic commands specified in the ENI */
  DDI_EM_ENI_NO_ALSTATUS          = 0x10011, /**< @brief No ALSTATUS specified in the ENI */
  DDI_EM_ENI_CMD_CNT_ERR          = 0x10012, /**< @brief Command count in the ENI is incorrect */
  DDI_EM_ENI_COE_SUPPORT          = 0x10013, /**< @brief No COE support specified in the ENI */
  DDI_EM_ENI_EOE_SUPPORT          = 0x10014, /**< @brief No EOE support specified in the ENI */
  DDI_EM_ENI_FOE_SUPPORT          = 0x10015, /**< @brief No FOE support specified in the ENI */
  DDI_EM_ENI_SOE_SUPPORT          = 0x10016, /**< @brief No SOE support specified in the ENI */
  DDI_EM_ENI_VOE_SUPPORT          = 0x10017, /**< @brief No VOE support specified in the ENI */
  DDI_EM_ENI_CYC_CMD_SIZE         = 0x10018, /**< @brief Cyclic command size incorrect in the ENI */
  DDI_EM_ENI_INPUT_OFFSET         = 0x10019, /**< @brief Invalid input offset in the ENI */
  DDI_EM_ENI_OUTPUT_OFFSET        = 0x1001A, /**< @brief Invalid output offset in the ENI */
  DDI_EM_ENI_PREV_PORT            = 0x1001B, /**< @brief Prev_port not listed in the ENI where required */
  DDI_EM_BUS_CFG_ERR              = 0x1001C, /**< @brief Bus configuration error occurred */
  DDI_EM_INTERNAL_STATE           = 0x1001D, /**< @brief Internal processing error occurred */
  DDI_EM_FRAME_LOST               = 0x1001E, /**< @brief Lost frame detected during cyclic processing */
  DDI_EM_CYC_CMD_MISSING          = 0x1001F, /**< @brief Cyclic commands missing */
  DDI_EM_CYC_WKC_ERR              = 0x10020, /**< @brief WKC count error occuring during cyclic processing */
  DDI_EM_COE_TX_WKC_ERR           = 0x10021, /**< @brief WKC count error occuring during CoE transmit */
  DDI_EM_COE_RX_WKC_ERR           = 0x10022, /**< @brief WKC count error occuring during CoE receieve */
  DDI_EM_PORT_CLOSE_ERR           = 0x10023, /**< @brief Error closing port */
  DDI_EM_PORT_OPEN_ERR            = 0x10024, /**< @brief Error opening port */
  DDI_EM_SYSTEM_DRIVER_ERR        = 0x10025, /**< @brief Issue occurred with system processing */
  DDI_EM_SCAN_BUS_CHANGE          = 0x10026, /**< @brief Networking changed during system scan */
  DDI_EM_MAX_SLAVES_ERR           = 0x10027, /**< @brief Maximum number of EtherCAT slaves exceeded */
  DDI_EM_SCAN_NO_SLV_FOUND        = 0x10028, /**< @brief No slaves found during master scan */
  DDI_EM_NIC_MAC_ACCESS_ERR       = 0x10029, /**< @brief Error accessing the NIC card MAC address */
  DDI_EM_REMOTE_ACCESS_INIT_ERR   = 0x1002A, /**< @brief Remote access initialization error */
  DDI_EM_REMOTE_ACCESS_DEINIT_ERR = 0x1002B, /**< @brief Remote access deinitialziation error */
  // Slave-specific errors
  DDI_ES_ERR                      = 0x20000, /**< @brief Unspecified slave error occurred */
  DDI_ES_COE_ERR                  = 0x20001, /**< @brief Error occurred duirng CoE operation */
  DDI_ES_FOE_WRITE_ERR            = 0x20002, /**< @brief Error occurred duirng FoE write */
  DDI_ES_FOE_READ_ERR             = 0x20003, /**< @brief Error occurred duirng FoE read */
  DDI_ES_SET_STATE_ERR            = 0x20004, /**< @brief Error occurred setting state change */
  DDI_ES_GET_STATE_ERR            = 0x20005, /**< @brief Error occurred getting state change information */
  DDI_ES_DUP_DETECTED             = 0x20006, /**< @brief Duplicate slave detected during network scan */
  DDI_ES_TIMER_FULL               = 0x20007, /**< @brief All available timers are taken required for this slave */
  DDI_ES_AUTO_INC_INVALID         = 0x20008, /**< @brief Auto-increment address is invalid */
  DDI_ES_INTERNAL_STATE           = 0x20009, /**< @brief Internal slave error occurred */
  DDI_ES_NOT_ADDRESSABLE          = 0x2000A, /**< @brief Slave is no longer addressable */
  DDI_ES_LINK_DISCONNECTED        = 0x2000B, /**< @brief Link was disconnected during operation */
  DDI_ES_NO_MBX_SUPPORT           = 0x2000C, /**< @brief No valid mailbox support is provided by this slave */
  DDI_ES_FOE_UNSPECIFIED          = 0x2000D, /**< @brief Unspecified FoE error occurred */
  DDI_ES_FOE_NOT_FOUND            = 0x2000E, /**< @brief FoE file not found */
  DDI_ES_FOE_ACCESS_DENIED        = 0x2000F, /**< @brief FoE access was denied */
  DDI_ES_FOE_DISK_FULL            = 0x20010, /**< @brief Disk full error occurred during FoE process */
  DDI_ES_FOE_ILLEGAL_OP           = 0x20011, /**< @brief Disk Illegal operation occurred during FoE */
  DDI_ES_FOE_PACKET_NO            = 0x20012, /**< @brief Invalid packet numbering occurred during FoE */
  DDI_ES_FOE_ALREADY_EXISTS       = 0x20013, /**< @brief File already exists after FoE */
  DDI_ES_FOE_NO_USER              = 0x20014, /**< @brief No valid user exists during FoE */
  DDI_ES_FOE_BOOTSTRAP_ONLY       = 0x20015, /**< @brief FoE only allowed during BOOT mode */
  DDI_ES_FOE_NOT_BOOTSTRAP        = 0x20016, /**< @brief FoE filename is not valid during BOOT mode */
  DDI_ES_FOE_PASSWORD             = 0x20017, /**< @brief FoE password is not valid */
  DDI_ES_FOE_PROG_ERROR           = 0x20018, /**< @brief FoE programming failed */
  DDI_ES_FOE_CHECKSUM             = 0x20019, /**< @brief FoE checksum invalid */
  DDI_ES_FOE_INVALID_FW           = 0x2001A, /**< @brief Invalid firmware for this device */
  DDI_ES_FOE_READ_FILE            = 0x2001B, /**< @brief FoE requested file invalid */
  DDI_ES_FOE_NOT_SUPPORTED        = 0x2001C, /**< @brief FoE is not supported by this device */
  DDI_ES_FOE_WKC_ERROR            = 0x2001D, /**< @brief FoE WKC error detected */
  DDI_ES_EEPROM_RD_ERR            = 0x2001E, /**< @brief Error occurred during EEPROM read */
  DDI_ES_EEPROM_WR_ERR            = 0x2001F, /**< @brief Error occurred during EEPROM write */
  DDI_ES_NOT_PRESENT              = 0x20020, /**< @brief Requested slave is not present on the network */
  DDI_ES_EEPROM_RELOAD_ERR        = 0x20021, /**< @brief Error occurred during EEPROM reloading */
  DDI_ES_RESET_ERR                = 0x20022, /**< @brief Error occurred during EEPROM reset */
  DDI_ES_EEPROM_ASSIGN_ERR        = 0x20023, /**< @brief Error occurred during EEPROM assignment to Master or PDI */
  DDI_ES_MBX_ERR                  = 0x20024, /**< @brief Slave mailbox error occurred */
  DDI_ES_MBX_WKC_ERR              = 0x20025, /**< @brief Slave mailbox WKC error occurred */
  DDI_ES_MBX_HEADER_ERR           = 0x20026, /**< @brief Slave mailbox header error occurred */
  DDI_ES_MBX_PROTOCOL_ERR         = 0x20027, /**< @brief Slave mailbox protocol error occurred */
  DDI_ES_MBX_CHANNEL_ERR          = 0x20028, /**< @brief Slave mailbox channel error occurred */
  DDI_ES_MBX_SERVICE_ERR          = 0x20029, /**< @brief Slave mailbox service error occurred */
  DDI_ES_MBX_PROTO_HEADER         = 0x2002A, /**< @brief Slave mailbox protcol header invalid */
  DDI_ES_MBX_SHORT_SIZE           = 0x2002B, /**< @brief Slave mailbox size is too short */
  DDI_ES_MBX_RESOURCES            = 0x2002C, /**< @brief Slave mailbox out of memory */
  DDI_ES_MBX_SIZE_ERR             = 0x2002D, /**< @brief Slave mailbox size mismatch */
  DDI_ES_FOE_FILE_SIZE_ERR        = 0x2002E, /**< @brief FoE file size error */
  DDI_ES_FOE_HEADER_MISSING       = 0x2002F, /**< @brief FoE file header missing */
  DDI_ES_FOE_FLASH_PROBLEM        = 0x20030, /**< @brief Error flashing FoE image */
  DDI_ES_FOE_FILE_INVALID         = 0x20031, /**< @brief FoE file is invalid */
  DDI_ES_LOST_COMM_ERR            = 0x20032, /**< @brief Communication was lost with the slave */
  DDI_ES_PDI_WATCHDOG             = 0x20033, /**< @brief PDI watchdog error occurred */
  DDI_ES_BAD_CONNECTION           = 0x20034, /**< @brief Bad EtherCAT connection was detected */
  DDI_ES_STATE_ERR                = 0x20035, /**< @brief Error occurred during slave state change */
  DDI_ES_SCAN_REV_ERR             = 0x20036, /**< @brief Revision did not match during bus scan */
  DDI_ES_SCAN_SERIAL_ERR          = 0x20037, /**< @brief Serial number did not match during bus scan */
  DDI_ES_MAX_NUM_OF_FUSIONS_ERR   = 0x20038, /**< @brief Maximum number of Fusion instances exceeeded */
  DDI_ES_EEPROM_BUSY              = 0x20039, /**< @brief Slave eeprom is busy */
  // SDO Abort codes
  DDI_EM_ABORTCODE_TOGGLE                   = 0x05030000,  /**< @brief Toggle bit did not toggle */
  DDI_EM_ABORTCODE_TIMEOUT                  = 0x05040000,  /**< @brief SDO protocol time out */
  DDI_EM_ABORTCODE_CCS_SCS                  = 0x05040001,  /**< @brief Client/server command specifier not valid or unknown */
  DDI_EM_ABORTCODE_BLK_SIZE                 = 0x05040002,  /**< @brief Invalid block size */
  DDI_EM_ABORTCODE_SEQNO                    = 0x05040003,  /**< @brief Invalid sequence number */
  DDI_EM_ABORTCODE_CRC                      = 0x05040004,  /**< @brief CRC error */
  DDI_EM_ABORTCODE_MEMORY                   = 0x05040005,  /**< @brief Out of memory */
  DDI_EM_ABORTCODE_ACCESS                   = 0x06010000,  /**< @brief Unsupported access to an object */
  DDI_EM_ABORTCODE_WRITEONLY                = 0x06010001,  /**< @brief Attempt to read a write only object */
  DDI_EM_ABORTCODE_READONLY                 = 0x06010002,  /**< @brief Attempt to write a read only object */
  DDI_EM_ABORTCODE_SI_NOT_WRITTEN           = 0x06010003,  /**< @brief Subindex cannot be written, SI0 must be 0 for write access */
  DDI_EM_ABORTCODE_CA_TYPE_MISMATCH         = 0x06010004,  /**< @brief Complete access not supported for objects of variable length such as ENUM object types */
  DDI_EM_ABORTCODE_OBJ_TOO_BIG              = 0x06010005,  /**< @brief Object length exceeds mailbox size */
  DDI_EM_ABORTCODE_PDO_MAPPED               = 0x06010006,  /**< @brief Object mapped to RxPDO, SDO Download blocked */
  DDI_EM_ABORTCODE_INDEX                    = 0x06020000,  /**< @brief Object does not exist in the object dictionary */
  DDI_EM_ABORTCODE_PDO_MAP                  = 0x06040041,  /**< @brief Object cannot be mapped to the PDO */
  DDI_EM_ABORTCODE_PDO_LEN                  = 0x06040042,  /**< @brief The number and length of the objects to be mapped would exceed PDO length */
  DDI_EM_ABORTCODE_P_INCOMP                 = 0x06040043,  /**< @brief General parameter incompatibility reason */
  DDI_EM_ABORTCODE_I_INCOMP                 = 0x06040047,  /**< @brief General internal incompatibility in the device */
  DDI_EM_ABORTCODE_HARDWARE                 = 0x06060000,  /**< @brief Access failed due to an hardware error */
  DDI_EM_ABORTCODE_DATA_LENGTH_NOT_MATCH    = 0x06070010,  /**< @brief Data type does not match, length of service parameter does not match */
  DDI_EM_ABORTCODE_DATA_LENGTH_TOO_HIGH     = 0x06070012,  /**< @brief Data type does not match, length of service parameter too high */
  DDI_EM_ABORTCODE_DATA_LENGTH_TOO_LOW      = 0x06070013,  /**< @brief Data type does not match, length of service parameter too low */
  DDI_EM_ABORTCODE_OFFSET                   = 0x06090011,  /**< @brief Sub-index does not exist */
  DDI_EM_ABORTCODE_VALUE_RANGE              = 0x06090030,  /**< @brief Value range of parameter exceeded (only for write access) */
  DDI_EM_ABORTCODE_VALUE_TOO_HIGH           = 0x06090031,  /**< @brief Value of parameter written too high */
  DDI_EM_ABORTCODE_VALUE_TOO_LOW            = 0x06090032,  /**< @brief Value of parameter written too low */
  DDI_EM_ABORTCODE_MODULE_ID_LIST_NOT_MATCH = 0x06090033,  /**< @brief Detected Module Ident List (0xF030) and Configured Module Ident list (0xF050) does not match */
  DDI_EM_ABORTCODE_MINMAX                   = 0x06090036,  /**< @brief Maximum value is less than minimum value */
  DDI_EM_ABORTCODE_GENERAL                  = 0x08000000,  /**< @brief General error */
  DDI_EM_ABORTCODE_TRANSFER                 = 0x08000020,  /**< @brief Data cannot be transferred or stored to the application */
  DDI_EM_ABORTCODE_TRANSFER_LOCAL_CONTROL   = 0x08000021,  /**< @brief Data cannot be transferred or stored to the application because of local control */
  DDI_EM_ABORTCODE_TRANSFER_DEVICE_STATE    = 0x08000022,  /**< @brief Data cannot be transferred or stored to the application because of the present device state */
  DDI_EM_ABORTCODE_DICTIONARY               = 0x08000023,  /**< @brief Object dictionary dynamic generation fails or no object dictionary is present */
} ddi_em_result;

/*! @enum ddi_em_logging_level
  @brief Configureable per-instance logging level
*/
typedef enum {
  DDI_EM_LOG_LEVEL_ERRORS      = 0, /**< @brief Log only error messages to the EtherCAT master interface persistent log file*/
  DDI_EM_LOG_LEVEL_WARNINGS    = 1, /**< @brief Log error and warning messages to the EtherCAT master interface persistent log file*/
  DDI_EM_LOG_LEVEL_DIAG        = 2, /**< @brief Log error, warning and diag messages to the EtherCAT master interface persistent log file*/
  DDI_EM_LOG_LEVEL_DEBUG       = 3, /**< @brief Log error, warning, diag and debug messages to the EtherCAT master interface persistent log file*/
  DDI_EM_LOG_LEVEL_VERBOSE     = 4  /**< @brief Log all messages to the EtherCAT master interface persistent log file*/
} ddi_em_logging_level;

/*! @enum ddi_em_network_control
  @brief A enumeration that represents the network control options.
  These settings allow for partial EtherCAT Master network operation and are used in the network_control_flags field in ddi_em_init_params

  DDI_EM_NETWORK_MASTER_STATE_CHECK_ENABLE enforces that all slaves reach the master state. For example, if the master state is set to OP mode
  and a slave is unable to reach OP mode, the entire network will not reach the requested master state. This is the default behavior of the
  DDI ECAT Master SDK.

  DDI_EM_NETWORK_MASTER_STATE_CHECK_DISABLE disables the master state enforcement. This allows for partial EtherCAT network support.

  DDI_EM_NETWORK_ENABLE_WKC enables the working counter check for an EtherCAT network instance. This is the default behavior of the DDI ECAT Master SDK.

  DDI_EM_NETWORK_DISABLE_WKC disables the working counter check for an EtherCAT network instance. This allows for partial EtherCAT network support.
*/
typedef enum {
  DDI_EM_NETWORK_MASTER_STATE_CHECK_ENABLE  = (0 << 0), /**< Default operation - all slaves must reach master state */
  DDI_EM_NETWORK_MASTER_STATE_CHECK_DISABLE = (1 << 0), /**< Partial network mode - not all slaves must reach master state */
  DDI_EM_NETWORK_ENABLE_WKC                 = (0 << 1), /**< Default operation - EtherCAT WKC check enabled for the EtherCAT Master instance*/
  DDI_EM_NETWORK_DISABLE_WKC                = (1 << 1), /**< Partial network mode - WKC not enforced for the EtherCAT Master instance */
} ddi_em_network_control;

// Initialization parameter defaults
/*! @var DDI_EM_CYCLIC_THREAD_PRI_HIGH
  @brief Highest cyclic thread priority supported
*/
#define DDI_EM_CYCLIC_THREAD_PRI_HIGH    100

/*! @var DDI_EM_CYCLIC_THREAD_PRI_LOW
  @brief Cyclic thread priority low limit
*/
#define DDI_EM_CYCLIC_THREAD_PRI_LOW     0

/*! @var DDI_EM_CYCLIC_THREAD_PRI_DEFAULT
  @brief Cyclic thread priority default
*/
#define DDI_EM_CYCLIC_THREAD_PRI_DEFAULT DDI_EM_CYCLIC_THREAD_PRI_HIGH

/*! @var DDI_EM_REMOTE_ENABLED
  @brief Remote access enabled
*/
#define DDI_EM_REMOTE_ENABLED            1

/*! @var DDI_EM_REMOTE_DISABLED
  @brief Remote access disabled
*/
#define DDI_EM_REMOTE_DISABLED           0

/*! @var DDI_EM_REMOTE_PD_SET_DISABLED
  @brief Remote setting of process data disabled
*/
#define DDI_EM_REMOTE_PD_SET_DISABLED    0

/*! @var DDI_EM_DEFAULT_CYCLIC_RATE
  @brief Default cyclic rate in microseconds
*/
#define DDI_EM_DEFAULT_CYCLIC_RATE       1000

/** @struct ddi_em_init_params
 *  @brief This is the EtherCAT Master initialization structure
 */
typedef struct {
  // Cyclic thread management
  uint32_t                polling_thread_priority; /**< Cyclic Thread priority. 0 = lowest, 100 = highest */
  uint32_t                enable_cyclic_thread;    /**< Control cyclic thread spawning. 0 = disable cyclic thread spawning (default), 1 = enable cyclic thread */
  // Remote access parameters
  const char *            remote_ip_addr;          /**< IP Address of the remote client, if enabled @deprecated since 1.1 */
  uint32_t                remote_client_enable;    /**< Enable or disable the remote client (default = disabled) @deprecated since 1.1 */
  uint32_t                remote_enable_set_pd;    /**< 0 = No ability to set pd from remote, 1 = remote setting of pd allowed @deprecated since 1.1 */
  // Master Configuration
  ddi_em_interface_select network_adapter;         /**< Network adapter this master interface listens on */
  uint32_t                scan_rate_us;            /**< Master scan rate in microseconds */
  uint32_t                enable_cpu_affinity;     /**< Enable CPU affinity selection, 0 = disable CPU affinity, 1 = use the value in cyclic_cpu_select */
  ddi_em_cpu_select       cyclic_cpu_select;       /**< CPU affinity selection */
  uint32_t                network_control_flags;   /**< Network control options, @see ddi_em_network_control */
} ddi_em_init_params;

/*! @var DDI_EM_MAX_MASTER_INSTANCES
  @brief Max number of supported master instances
*/
#define DDI_EM_MAX_MASTER_INSTANCES      6

/** @struct ddi_em_remote_access_init_params
 *  @brief This is the EtherCAT Master remote access initialization structure
 */
typedef struct {
  // Remote access parameters
  uint32_t                remote_port;                /**< TCP/IP Port to listen on */
  ddi_em_cpu_select       remote_cpu_select;          /**< CPU affinity selection for the remote access server */
  uint32_t                remote_enable_cpu_affinity; /**< Enable CPU affinity selection, 0 = disable CPU affinity, 1 = use the value in remote_cpu_select */
} ddi_em_remote_access_init_params;

#define DDI_EM_MAX_REMOTE_INSTANCES   6

/** @var typedef int32_t ddi_em_handle
  @brief This typedef represents the value of the DDI EtherCAT Master (master) handle.
*/
typedef int32_t ddi_em_handle;

/** @var typedef int32_t ddi_em_ra_handle
  @brief This typedef represents the value of the DDI EtherCAT Master (master) Remote Access handle.
*/
typedef int32_t ddi_ra_handle;

/** @var typedef int32_t ddi_es_handle
  @brief This typedef represents the value of the DDI EtherCAT Master slave handle.
*/
typedef int32_t ddi_es_handle;

/** @var DDI_EM_INVALID_HANDLE
  @brief This value signifies an invalid handle for both master and slave.
*/
#define DDI_EM_INVALID_HANDLE             -1

/** @enum ddi_em_state
    @brief An enum that contains possible state values for an EtherCAT master.
 */
typedef enum {
  DDI_EM_INVALID_STATE = -1, /**< @brief Invalid State */
  DDI_EM_STATE_INIT    =  1, /**< @brief INIT Mode */
  DDI_EM_STATE_PREOP   =  2, /**< @brief PRE-OP Mode */
  DDI_EM_STATE_SAFEOP  =  4, /**< @brief SAFE-OP mode */
  DDI_EM_STATE_OP      =  8  /**< @brief OP mode */
} ddi_em_state;

/** @enum ddi_es_state
  @brief An enum that contains possible state values for an EtherCAT slave.

  A structure that contains possible state values for an EtherCAT slave.  Note that the EtherCAT slave
  value has a BOOT mode option, whereas ddi_em_state does not.
*/
typedef enum {
  DDI_ES_INVALID_STATE  = -1, /**< @brief Invalid State */
  DDI_ES_STATE_INIT     =  1, /**< @brief INIT Mode */
  DDI_ES_STATE_PREOP    =  2, /**< @brief PRE-OP mode */
  DDI_ES_STATE_BOOT     =  3, /**< @brief BOOT Mode */
  DDI_ES_STATE_SAFEOP   =  4, /**< @brief SAFE-OP mode */
  DDI_ES_STATE_OP       =  8  /**< @brief OP mode */
} ddi_es_state;

/*! @struct ddi_em_slave_config
  \brief A structure that contains configuration information about the slave

  A structure that contains EtherCAT master statistics.  This structure information will be populated with a call
  to ddi_em_get_slave_config().
*/
typedef struct {
  uint32_t pd_input_offset;  /**< @brief Process data input offset of the slave.  This value is in bytes */
  uint32_t pd_output_offset; /**< @brief Process data output offset of the slave.  This value is in bytes */
  uint32_t pd_input_size;    /**< @brief The input process data size of the slave. This value in in bytes */
  uint32_t pd_output_size;   /**< @brief The output process data size of the slave. This value in in bytes */
  uint32_t slave_present;    /**< @brief Is the slave present on the network? 0 = Slave is not present. 1 = Slave is present*/
  uint32_t vendor_id;        /**< @brief Vendor ID of the slave (EEPROM offset 0x0008) */
  uint32_t product_code;     /**< @brief Product code (EEPROM offset 0x000A) */
  uint32_t revision;         /**< @brief Revision of the slave (EEPROM offset 0x000C) */
  uint32_t serial_number;    /**< @brief Serial number of the slave (EEPROM offset 0x000E) */
  uint32_t station_address;  /**< @brief Station address of the slave */
} ddi_em_slave_config;

/*! @struct ddi_em_master_stats
  \brief A structure that contains EtherCAT master statistics.

  A structure that contains EtherCAT master statistics.  This structure information will be populated with a call
  to ddi_em_get_master_stats().
*/
typedef struct {
  uint32_t cyclic_err_frame_count;               /**< @brief How many cyclic frames have had errors */
  uint32_t cur_consecutive_err_frame_count;      /**< @brief How many consecutive frames have lost frames currently */
  uint32_t max_consecutive_err_frame_count;      /**< @brief Maximum consecutive frames that have been lost since initialization */
  uint64_t cyclic_frames_with_no_errors;         /**< @brief Cyclic frames without errors */
  uint32_t max_cyclic_timestamp_diff_ns;         /**< @brief Maximum delta of consecutive cyclic frames, in nanoseconds */
  uint32_t min_cyclic_timestamp_diff_ns;         /**< @brief Minimum delta of consecutive cyclic frames, in nanoseconds */
  uint32_t average_cyclic_timestamp_diff_ns;     /**< @brief Average delta of consecutive cyclic frames, in nanoseconds */
} ddi_em_master_stats;

/*! @var DDI_EM_DISABLE_REV_DURING_OPEN
    @brief This value will disable the revision check during the ddi_em_open_by_station_address() call
*/
#define DDI_EM_DISABLE_REV_DURING_OPEN     0 // Disable revision check during ddi_em_open_slave_by_id call

/*! @var DDI_EM_DISABLE_SERIAL_DURING_OPEN
    @brief This value will disable the serial number check during the ddi_em_open_by_station_address() call
*/
#define DDI_EM_DISABLE_SERIAL_DURING_OPEN  0 // Disable serial number check during ddi_em_open_slave_by_id call

// Event section ----------------------------------------------------

/*! @enum ddi_em_event_type
  @brief A enumeration that represents the possible supported events in the DDI EtherCAT Master SDK

  The events listed below will be sent to the event callback handler registered by ddi_em_set_event_handler().
  The application must handle the callback for the appropriate events or the event will be lost.
  This enumeration contains the currently supported events in version 1.0.0 of the DDI ECAT Master SDK

  The event types are broken up into several sections:
  0-0xFFFF         : General purpose SDK event codes that can belong to the Master or Slave
  0x10000-0x1FFFF  : Master event notifications (not error-related)
  0x20000-0x2FFFF  : Master events that are error related
  0x30000-0x3FFFF  : Slave event notifications (not error-related)
  0x40000-0x4FFFF  : Slave events that are error related
*/
typedef enum {
  // 0-0xFFFF Generic Events
  DDI_EM_EVENT_ERR                          = -1,       /**< @brief Error during event */
  DDI_EM_EVENT_GENERIC                      = 0x00001,  /**< @brief Generic event received */
  DDI_EM_EVENT_APPLICATION                  = 0x00002,  /**< @brief Application triggered event */
  // 0x10000-0x1FFFF Master Events
  DDI_EM_EVENT_STATE_CHANGED                = 0x10000,  /**< @brief EtherCAT Master state changed */
  DDI_EM_EVENT_LINK_CONNECTED               = 0x10001,  /**< @brief EtherCAT link connected */
  DDI_EM_EVENT_MBX_TRANSFER_COMPLETE        = 0x10002,  /**< @brief EtherCAT link connected */
  DDI_EM_EVENT_SCAN_NETWORK_COMPLETE        = 0x10003,  /**< @brief Network Scan completed */
  DDI_EM_EVENT_SCAN_NETWORK_MISMATCH        = 0x10004,  /**< @brief One or more slaves mismatched during network scan */
  DDI_EM_EVENT_SCAN_NETWORK_DUP             = 0x10005,  /**< @brief Duplicate slave detected on the EtherCAT network */
  DDI_EM_EVENT_FRAMELOSS                    = 0x10006,  /**< @brief Frameloss detected on the EtherCAT network */
  DDI_EM_EVENT_RAW_CMD_COMPLETE             = 0x10007,  /**< @brief Raw EtherCAT command completed */
  DDI_EM_EVENT_RAW_MBX_COMPLETE             = 0x10008,  /**< @brief Raw EtherCAT mailbox transfer completed */
  DDI_EM_EVENT_COE_INIT_CMD_COMPLETE        = 0x10009,  /**< @brief COE init command completed during state transition */
  // 0x20000-0x2FFFF Master Error Events
  DDI_EM_EVENT_ERR_CYC_WKC                  = 0x20000,  /**< @brief Cyclic frame working count (WKC) error */
  DDI_EM_EVENT_ERR_INIT_CMD                 = 0x20001,  /**< @brief Error occurred during master initialization */
  DDI_EM_EVENT_ERR_INIT_CMD_RESPONSE        = 0x20002,  /**< @brief Invalid master response during initialization */
  DDI_EM_EVENT_ERR_NOT_ALL_SLAVES_IN_OP     = 0x20003,  /**< @brief Not all slaves are in OP mode while process data is active*/
  DDI_EM_EVENT_ERR_LINK_DISCONNECTED        = 0x20004,  /**< @brief EtherCAT link disconnected */
  DDI_EM_EVENT_ERR_CLIENT_DROPPED           = 0x20005,  /**< @brief Client registration dropped due to re-initialization of the master */
  DDI_EM_EVENT_ERR_SLAVE_NOT_SUPPORTED      = 0x20006,  /**< @brief Unsupported master detected during bus scan */
  DDI_EM_EVENT_ERR_ALL_SLAVES_IN_OP         = 0x20007,  /**< @brief All devices back in OP after DDI_EM_EVENT_ERR_NOT_ALL_SLAVES_IN_OP */
  DDI_EM_EVENT_ERR_SCAN_MISMATCH            = 0x20008,  /**< @brief Mismatch during network scan  */
  // 0x30000-0x3FFFF Slave Events
  DDI_ES_EVENT_PRESENCE                     = 0x30000,  /**< @brief New slave presence on the network detected */
  DDI_ES_EVENT_MULTIPLE_PRESENCE            = 0x30001,  /**< @brief New multiple slaves presence on the network detected */
  DDI_ES_EVENT_LINK_LINES_CROSSED           = 0x30002,  /**< @brief EtherCAT network lines were crossed */
  DDI_ES_EVENT_WKC_ERROR                    = 0x30003,  /**< @brief WKC error occurred during CoE or FoE */
  DDI_ES_EVENT_STATE_CHANGED                = 0x30004,  /**< @brief EtherCAT slave state changed */
  DDI_ES_EVENT_MULT_STATES_CHANGED          = 0x30005,  /**< @brief EtherCAT multiple slave state changed */
  DDI_ES_EVENT_REG_TRANSFER_COMPLETE        = 0x30006,  /**< @brief Slave ESC register access complete */
  DDI_ES_EVENT_EEPROM_OP_COMPLETE           = 0x30007,  /**< @brief Slave EEPROM operation complete or timeout */
  DDI_ES_EVENT_PORT_OP_COMPLETE             = 0x30008,  /**< @brief Port operation (close or open) complete */
  DDI_ES_EVENT_ID_CMD_COMPLETE              = 0x30009,  /**< @brief ID operation complete */
  DDI_ES_EVENT_COE_TX_PDO                   = 0x3000A,  /**< @brief CoE Tx PDO complete */
  // 0x40000-0x4FFFF Slave Error Events
  DDI_ES_EVENT_ERR_INIT_CMD                 = 0x40000,  /**< @brief WKC error occurred during slave initialization */
  DDI_ES_EVENT_ERR_COE_WKC                  = 0x40001,  /**< @brief WKC error occurred during CoE transfer */
  DDI_ES_EVENT_ERR_FOE_WKC                  = 0x40002,  /**< @brief WKC error occurred during FoE transfer */
  DDI_ES_EVENT_ERR_FRAME_RESPONSE           = 0x40003,  /**< @brief Invalid frame response detected */
  DDI_ES_EVENT_ERR_INIT_CMD_RESPONSE        = 0x40004,  /**< @brief Invalid slave response during initialization */
  DDI_ES_EVENT_ERR_INIT_CMD_TIMEOUT         = 0x40005,  /**< @brief Timeout occurred during slave initialization */
  DDI_ES_EVENT_ERR_ERROR_STATE              = 0x40006,  /**< @brief Slave entered error-state */
  DDI_ES_EVENT_ERR_ADDRESSABLE              = 0x40007,  /**< @brief Slave is not addressable */
  DDI_ES_EVENT_ERR_COE_SDO_ABORT            = 0x40008,  /**< @brief Slave encountered an SDO abort during CoE operation */
  DDI_ES_EVENT_ERR_FOE_MBX                  = 0x40009,  /**< @brief Mailbox returned error during FoE */
  DDI_ES_EVENT_ERR_INVALID_MBX_DATA         = 0x4000A,  /**< @brief Invalid mailbox data received */
  DDI_ES_EVENT_ERR_PDI_WATCHDOG             = 0x4000B,  /**< @brief PDI watchdog error occurred */
  DDI_ES_EVENT_ERR_UNEXPECTED_STATE         = 0x4000C,  /**< @brief Slave is in an unexpected state */
  DDI_ES_EVENT_ERR_EEPROM_CHECKSUM_ERROR    = 0x4000D,  /**< @brief Error with EEPROM checksum */
  DDI_ES_EVENT_ERR_LINES_CROSSED            = 0x4000E,  /**< @brief EtherCAT lines are crosssed */
  DDI_ES_EVENT_ERR_MULT_UNEXPECTED_STATE    = 0x4000F,  /**< @brief Multiple Slaves are in an unexpected state  */
  DDI_ES_EVENT_ERR_MULT_ERROR_STATE         = 0x40010,  /**< @brief Multiple Slaves entered error-state */
  DDI_ES_EVENT_ERR_FRAMELOSS_AFTER_OPEN     = 0x40011,  /**< @brief Frameloss occurred due to port open */
  DDI_ES_EVENT_ERR_POOR_CONNECTION          = 0x40012,  /**< @brief Poor EtherCAT connection detected */
} ddi_em_event_type;

/*! @var DDI_EM_EVENT_MAX_STR_SIZE
    @brief This define is the maximum size of a string reported through the ddi_em_event structure
*/
#define DDI_EM_EVENT_MAX_STR_SIZE   256

// Represents event event status
/*! @struct ddi_em_event
  \brief Represents event event status

  A structure that contains EtherCAT master statistics.  This structure information will be populated with a call
  to ddi_em_get_master_stats().
*/
typedef struct {
  ddi_em_handle      master_handle;           /**< Master Handle the event ocurred on */
  ddi_es_handle      es_handle;               /**< Slave Handle the event occurred on */
  ddi_em_event_type  event_code;              /**< Event code @see ddi_em_event_type */
  const char         *event_str;              /**< Event details in text format */
} ddi_em_event;

/*! @enum ddi_em_event_control
  @brief A enumeration that represents the event control options for the ddi_em_set_notify_control() function
*/
typedef enum {
  DDI_EM_EVENT_DISABLE_EVENT  = 0, /**< Disable the corresponding event */
  DDI_EM_EVENT_ENABLE_EVENT   = 1  /**< Enable the corresponding event */
} ddi_em_event_control;

/*!
@brief The client callback function prototype for system event notifications.
The client application registers their callback function with ddi_em_set_event_handler to receive system event notifications.

During each callback:
- The event_code will contain the numeric code of the event that occurred @see ddi_em_event_type
- The event_str will contain a text-event string of the event that occurred. This string can be used for logging and display purposes.
Example Usage:
<PRE>
typedef struct {
  ddi_em_handle em_handle;
  // add other user data here: e.g.
  user_txpdo_data input_pd;
  user_rxpdo_data output_pd;
} app_event_data_type;
app_event_data_type app_data;
void user_app_event_handler (app_event_data_type *data)
{
  static int frame_response_count = 0;
  switch ( event->event_code )
  {
    case DDI_EM_EVENT_STATE_CHANGED:
      printf(GREEN "Master state changed " CLEAR "\n");
      break;
    case DDI_ES_EVENT_STATE_CHANGED:
      printf(GREEN "Slave state changed " CLEAR "\n");
      break;
    case DDI_EM_EVENT_FRAMELOSS:
      printf(RED "Frameloss detected " CLEAR "\n");
      break;
    case DDI_ES_EVENT_PRESENCE:
      printf(YELLOW "Slave presence changed " CLEAR "\n");
      break;
    case DDI_ES_EVENT_ERR_FRAME_RESPONSE:
      printf(RED "Frame response error " CLEAR "\n");
      frame_response_count++;
      printf("Frame response error count %d \n", frame_response_count);
      break;
    default:
      ELOG("Unsupported notification code 0x%x \n", event->event_code);
      break;
  }
  return 0;
}
Register callback from main app entrypoint:
ddi_em_set_event_handler(em_handle, (ddi_em_event_func *)user_app_event_handler, (void *)&app_data);
</PRE>
@param event pointer to the users client data which was registered with the callback function.
*/
typedef uint32_t (ddi_em_event_func)(ddi_em_event* event);

/*!
@brief The client callback function prototype for cyclic process data event notifications.
The client application registers their callback function with ddi_em_register_cyclic_callback to receive cyclic event notifications.
Then the client application may access input and output process data from their callback.
During each callback:
- The most recent input process data (TxPDO) received is available for the client application to read.
- Any output process data (RxPDO) written by the client application is transmitted the following cyclic frame.

Example Usage:
<PRE>
typedef struct {
  ddi_em_handle em_handle;
  ddi_em_slave_config *es_cfg;
  // add other user data here: e.g.
  user_txpdo_data input_pd;
  user_rxpdo_data output_pd;
} app_data_type;
app_data_type app_data;
#define INPUT_PD 0
#define OUTPUT_PD 1
void user_app_process_data_callback (app_data_type *data)
{
  ddi_em_handle em_handle = data->em_handle;
  // access input and output process data here
  uint32_t txpdo_offset = data->es_cfg->pd_input_offset;
  uint32_t rxpdo_offset = data->es_cfg->pd_output_offset;
  // read inputs
  ddi_em_get_process_data(em_handle, txpdo_offset, &data->input_pd, sizeof(data->input_pd), INPUT_PD);
  // write outputs
  ddi_em_set_process_data(em_handle, rxpdo_offset, &data->output_pd, sizeof(data->output_pd));
}
Register callback from main app entrypoint:
ddi_em_register_cyclic_callback(em_handle, (ddi_em_cyclic_func *)user_app_process_data_callback, (void *)&app_data);
</PRE>
@param user_data pointer to the users client data which was registered with the callback function.
*/

typedef void (ddi_em_cyclic_func)(void *user_data);

// Global Management ----------------------------------------------------
/** ddi_em_init
 @brief Initializes the EtherCAT Master SDK
 This function must be called first prior to calling other functions
 @return ddi_em_result The result code of the operation @see ddi_em_result
 */
ddi_em_result ddi_em_sdk_init(void);

/** ddi_em_deinit
 @brief De-initializes the EtherCAT Master SDK
 @return ddi_em_result The result code of the operation @see ddi_em_result
 */
ddi_em_result ddi_em_sdk_deinit(void);

/** ddi_em_get_version
 @brief Return the version number as a string for DDI EtherCAT Master SDK
 @return The version number of the EtherCAT Master as a string
 */
const char* ddi_em_get_version(void);

/** ddi_em_init
 @brief Allocates and Initializes an EtherCAT Master Instance
 @param init_params The DDI ECAT master initialization parameters @see ddi_em_init_params
 @param em_handle The EtherCAT master instance allocated by the initialization routine
 @return ddi_em_result The result code of the operation @see ddi_em_result
 */
ddi_em_result ddi_em_init(ddi_em_init_params *init_params, ddi_em_handle *em_handle);

/** ddi_em_remote_access_init
 @brief Initializes the remote access server for the EtherCAT master instance
 @param init_params The remote access parameters @see ddi_em_remote_access_init_params
 @param remote_handle The remote access handle allocated by the ddi_em_remote_access_init call
 @return ddi_em_result The result code of the operation @see ddi_em_result
 @see ddi_em_result
 */
ddi_em_result ddi_em_remote_access_init (ddi_em_remote_access_init_params *init_params, ddi_ra_handle *remote_handle);

/** ddi_em_remote_access_deinit
 @brief Deinitialize the remote access server for the EtherCAT master instance
 @param remote_handle The remote access handle
 @return ddi_em_result The result code of the operation @see ddi_em_result
 @see ddi_em_result
 */
ddi_em_result ddi_em_remote_access_deinit (ddi_ra_handle remote_handle);

/** ddi_em_deinit
 @brief De-initializes the given master instance.
 This function will close and free any resources allocated by ddi_em_init()
 @param em_handle The EtherCAT Master instance handle
 @return ddi_em_result The result code of the operation @see ddi_em_result
 */
ddi_em_result ddi_em_deinit(ddi_em_handle em_handle);

/** ddi_em_configure_master
 @brief Configures an EtherCAT Master Instance with a given ENI file and place the master into INIT mode
 @param em_handle The EtherCAT Master instance handle
 @param eni_filename The full path of the ENI location. The ENI file may reside in any valid location on the filesystem
 @return ddi_em_result The result code of the operation @see ddi_em_result
 */
ddi_em_result ddi_em_configure_master(ddi_em_handle em_handle, const char* eni_filename);

/** ddi_em_set_cycle_rate
 @brief Sets the cycle rate for the given Master instance
 @param em_handle The EtherCAT Master instance handle
 @param cycle_rate_us The cycle rate in microseconds
 @return ddi_em_result The result code of the operation @see ddi_em_result
 */
ddi_em_result ddi_em_set_cycle_rate(ddi_em_handle em_handle, uint32_t cycle_rate_us);

// Slave Management -------------------------------------------------------
/** ddi_em_open_by_station_address
 @brief Returns a slave handle for a slave whose station address matches for the given EtherCAT Master Instance
 @param em_handle The EtherCAT Master instance handle
 @param station_address The station address of the slave to open
 @param es_handle The slave handle of the found device
 @return ddi_em_result The result code of the operation @see ddi_em_result
 */
ddi_em_result ddi_em_open_by_station_address(ddi_em_handle em_handle, uint32_t station_address, ddi_es_handle *es_handle);

/** ddi_em_open_slave_by_position
 @brief Returns a slave handle for the slave position given by auto_inc_address for the given EtherCAT Master Instance
 @param em_handle The EtherCAT Master instance handle
 @param auto_inc_address The slave position to open a handle to
 @param es_handle The slave handle of the found device
 @return ddi_em_result The result code of the operation @see ddi_em_result
 */
ddi_em_result  ddi_em_open_slave_by_position(ddi_em_handle em_handle, uint32_t auto_inc_address, ddi_es_handle *es_handle);

/** ddi_em_open_slave_by_id
 @brief Returns a slave handle for the slave matching the given vendor_id and product_id for the given Master Instance
 @param em_handle  The EtherCAT Master instance handle
 @param vendor_id The EtherCAT Vendor ID of the slave (EEPROM offset 0x0008)
 @param product_id The EtherCAT Product ID of the slave (EEPROM offset 0x000A)
 @param revision The EtherCAT Revision code of the slave (EEPROM offset 0x000C). This is an optional argument. Pass 0 to disable this check
 @param serial_number The EtherCAT Serial Number code of the slave (EEPROM offset 0x000E). This is an optional argument. Pass 0 to disable this check
 @param es_handle Pointer to a ddi_es_handle. On success, receives a handle to the found slave
 @return ddi_em_result DDI_EM_STATUS_OK on success; DDI_EM_SCAN_NO_SLV_FOUND if the slave was not found @see ddi_em_result
 */
ddi_em_result ddi_em_open_slave_by_id(ddi_em_handle em_handle, uint32_t vendor_id, uint32_t product_id, uint32_t revision, uint32_t serial_number, ddi_es_handle *es_handle);

/** ddi_em_close_slave
 @brief Closes a slave handle for Master instance
 @param em_handle The EtherCAT Master instance handle
 @param es_handle The EtherCAT Slave handle
 @return ddi_em_result The result code of the operation @see ddi_em_result
 */
ddi_em_result ddi_em_close_slave(ddi_em_handle em_handle, ddi_es_handle es_handle);

/** ddi_em_get_slave_config
 @brief Gets EtherCAT configuration information about a slave
 @param em_handle The EtherCAT Master instance handle
 @param es_handle The EtherCAT Slave handle
 @param cfg_info The EtherCAT slave configuration information
 @return ddi_em_result The result code of the operation @see ddi_em_result
 @see ddi_em_slave_config
 */
ddi_em_result ddi_em_get_slave_config(ddi_em_handle em_handle, ddi_es_handle es_handle, ddi_em_slave_config *cfg_info);

// EtherCAT Statistics -------------------------------------------------
/** ddi_em_get_master_stats
 @brief Gets statistic information about a master instance
 @param em_handle The EtherCAT Master instance handle
 @param master_stats The master statistic structure to be written to @see ddi_em_master_stats
 @return ddi_em_result The result code of the operation @see ddi_em_result
 */
ddi_em_result ddi_em_get_master_stats(ddi_em_handle em_handle, ddi_em_master_stats *master_stats);

// Process Data Management -------------------------------------------------
// Instance versions
/** ddi_em_set_process_data
 @brief Sets process data for a given Master instance. This function operates on byte amounts.
 This function will copy length bytes of the data argument buffer to the master output process data byte offset specified by pd_offset 
 @param em_handle The Master instance handle to update
 @param pd_offset The Master output process data offset to update in bytes
 @param data The data to update
 @param length The data length to update in bytes
 @return ddi_em_result The result code of the operation @see ddi_em_result
 */
ddi_em_result ddi_em_set_process_data(ddi_em_handle em_handle, uint32_t pd_offset, uint8_t *data, uint32_t length);

/** ddi_em_set_process_data_bits
 @brief Sets process data for a give Master instance. This function operates on bit amounts
 This function will copy bit_length bits of the data argument buffer starting at offset dest_bit_offset to the master output process 
 data bit offset specified by pd_bit_offset
 @param em_handle The Master instance handle to update
 @param pd_bit_offset The Master output process bit offset
 @param dest_bit_offset The destination bit offset
 @param data The data to update
 @param bit_length The data length to update in bytes
 @return ddi_em_result The result code of the operation @see ddi_em_result
 */
ddi_em_result ddi_em_set_process_data_bits(ddi_em_handle em_handle, uint32_t pd_bit_offset, uint32_t dest_bit_offset, uint8_t *data, uint32_t bit_length);

/** ddi_em_get_process_data
 @brief Retrieves process data for a given Master instance. This function operates on byte amounts
 This function will copy length bytes from the Master input process data section starting at the byte offset specified by pd_offset to
 the buffer pointed to by data
 @param em_handle The Master instance handle to retreive data from
 @param pd_offset The Master process data offset to retreive in bytes
 @param data The buffer to store retrieved data
 @param length The data length to retreive in bytes
 @param is_output Is this access a request to the output or input process data. 0 = input process data, 1 = output process data
 @return ddi_em_result The result code of the operation @see ddi_em_result
 */
ddi_em_result ddi_em_get_process_data(uint32_t em_handle, uint32_t pd_offset, uint8_t *data, uint length, uint8_t is_output);

/** ddi_em_get_process_data_bits
 @brief Retrieves process data for a given Master instance. This function operates on bits amounts
 This function will copy bit_length bits from the Master input process data section starting at offset location pd_bit_offset to
 the buffer pointed to by data at offset dest_bit_offset
 @param em_handle The Master instance handle to retreive data from
 @param pd_bit_offset The Master process data source bit offset in bits
 @param dest_bit_offset The destination bit offset in bits
 @param data The buffer to store retrieved data
 @param bit_length The data length to retreive in bits
 @param is_output Is this access a request to the output or input process data. 0 = input process data, 1 = output process data
 @return ddi_em_result The result code of the operation @see ddi_em_result
 */
ddi_em_result ddi_em_get_process_data_bits(ddi_em_handle em_handle, uint32_t pd_bit_offset, uint32_t dest_bit_offset, uint8_t *data, uint32_t bit_length, uint8_t is_output=false );

// State Control -----------------------------------------------------------
/** ddi_em_set_master_state
 @brief Sets the Master state for the given Master instance
 @param em_handle The Master instance handle
 @param master_state The Master state to set the master instance to @see ddi_em_state
 @param timeout The timeout in milliseconds
 @return ddi_em_result The result code of the operation @see ddi_em_result
 */
ddi_em_result ddi_em_set_master_state(ddi_em_handle em_handle, ddi_em_state master_state, uint32_t timeout);

/** ddi_em_get_master_state
 @brief Retrieve the master state for the given Master instance
 @param em_handle The Master instance handle
 @param state The Master state @see ddi_em_state
 @return ddi_em_result The result code of the operation @see ddi_em_result
 */
ddi_em_result ddi_em_get_master_state(ddi_em_handle em_handle, ddi_em_state *state);

/** ddi_em_set_slave_state
 @brief Sets the slave state for the given Master instance
 @param em_handle The Master instance handle
 @param es_handle The slave handle
 @param slave_state The Slave state to set the slave represented by es_handle to @see ddi_es_state
 @param timeout The timeout in milliseconds
 @return ddi_em_result The result code of the operation @see ddi_em_result
 */
ddi_em_result ddi_em_set_slave_state(ddi_em_handle em_handle, ddi_es_handle es_handle, ddi_es_state slave_state, uint32_t timeout);

/** ddi_em_get_slave_state
 @brief Retrieve the slave state for the given master instance
 @param em_handle The Master instance handle
 @param es_handle The Slave handle
 @param slave_state The returned slave state @see ddi_es_state
 @return ddi_em_result The retrieved slave state @see ddi_em_result
 */
ddi_em_result ddi_em_get_slave_state(ddi_em_handle em_handle, ddi_es_handle es_handle, ddi_es_state *slave_state);

// Cyclic Callback ---------------------------------------------------------
/** ddi_em_register_cyclic_callback
 @brief Sets the callback function for the given master instance. This function must be called in PRE-OP mode.
 @param em_handle The Master instance
 @param callback The cyclic data callback
 @param user_data The user-specified callback arguments, may be NULL.
 @return ddi_em_result The result of the operation @see ddi_em_result
 */
ddi_em_result ddi_em_register_cyclic_callback(ddi_em_handle em_handle, ddi_em_cyclic_func *callback, void *user_data);

/** ddi_em_cyclic_task_start
 @brief Start the cyclic task.  This function is used if the cyclic thread is managed outside of the DDI ECAT SDK.
 This function will start the cyclic task. This function is used if the cyclic thread is managed outside of the DDI ECAT SDK
 This function blocks and will not return until ddi_em_cyclic_task_stop() is called
 @param em_handle The Master instance to start the cyclic task on
 @return ddi_em_result The result of the operation @see ddi_em_result
 */
ddi_em_result ddi_em_cyclic_task_start(ddi_em_handle em_handle);

/** ddi_em_cyclic_task_stop
 @brief Stop the cyclic task.  This function is used to stop the cyclic task started by the ddi_em_cyclic_task_start() call
 @param em_handle The Master instance to stop the cyclic task on
 @return ddi_em_result DDI_EM_STATUS_OK if successful, DDI_EM_STATUS_CYC_THREAD_STOP if operation fails @see ddi_em_result
 */
ddi_em_result ddi_em_cyclic_task_stop(ddi_em_handle em_handle);

// Notifications -------------------------------------------------------------
/** ddi_em_register_notify
 @brief Registers the event callback mechanism for the given Master instance
 @param em_handle The EtherCAT Master instance handle
 @param callback The callback to be executed when the event that matches the mask is received @see ddi_em_event_func
 @return ddi_em_result The result of the register operation @see ddi_em_result
 */
ddi_em_result ddi_em_set_event_handler(ddi_em_handle em_handle, ddi_em_event_func *callback);

/** ddi_em_enable_event_handler
 @brief Registers the events specified in mask for the given Master instance with a optional callback function
 @param em_handle The EtherCAT Master instance handle
 @param notify_code The event value to be enabled or disabled @see ddi_em_event_type
 @param enable_event Enable or disable the event for the value specified in notify_code @see ddi_em_event_control
 @return ddi_em_result The result of the event enable operation @see ddi_em_result
 */
ddi_em_result ddi_em_enable_event_handler(ddi_em_handle em_handle, ddi_em_event_type notify_code, ddi_em_event_control enable_event);

// CANopen over EtherCAT (CoE) Interface -------------------------------------
// Instance Version
/** ddi_em_coe_write
 @brief Write data to a COE index/subindex for the given Master instance. This operation is from Master->Slave
 @param em_handle The Master instance handle
 @param es_handle The slave handle
 @param index The slave index to update
 @param subindex the slave subindex to update
 @param data The data to update
 @param len The data length to update in bytes
 @param timeout Timeout in milliseconds
 @param flags Additional COE Flags. 1 = Complete access
 @return ddi_em_result The result of the CoE write operation @see ddi_em_result
 */
ddi_em_result ddi_em_coe_write(ddi_em_handle em_handle, ddi_es_handle es_handle, uint16_t index, uint16_t subindex, uint8_t *data,
 uint32_t len, uint32_t timeout, uint32_t flags );

/** ddi_em_coe_read
 @brief Read data from a COE index/subindex for Master instance 0. This operation is from Slave->Master
 @param em_handle the Master instance handle
 @param es_handle The slave handle
 @param index The slave index to update
 @param subindex the slave subindex to update
 @param data The data to update
 @param len The maximum data length to update in bytes
 @param out_len The data length read
 @param timeout Timeout in milliseconds
 @param flags Additional COE Flags. 1 = Complete access
 @return ddi_em_result The result of the CoE read operation @see ddi_em_result
 */
ddi_em_result ddi_em_coe_read(ddi_em_handle em_handle, ddi_es_handle es_handle, uint16_t index, uint16_t subindex, uint8_t *data,
 uint32_t len, uint32_t *out_len, uint32_t timeout, uint32_t flags );

// File over EtherCAT (FoE) Interface ----------------------------------------
/** ddi_em_foe_write
 @brief Send data over File over EtherCAT (FoE) for the given Master instance
 The filename and data buffer must contain a valid address for this operation to complete successfully
 @param em_handle The master instance handle
 @param es_handle The slave handle
 @param filename The filename of the firmware file to be sent to the remove device
 @param file_length The length of the filename in bytes
 @param data The contents to be written over FOE
 @param data_length The data length for the buffer in bytes
 @param password The FoE password
 @param timeout Timeout in milliseconds
 @return ddi_em_result The result of the FoE write operation @see ddi_em_result
 */
ddi_em_result ddi_em_foe_write(ddi_em_handle em_handle, ddi_es_handle es_handle, const char *filename, uint32_t file_length,
  uint8_t *data, uint32_t data_length, uint32_t password, uint32_t timeout );

/** ddi_em_foe_read
 @brief Upload data over File over EtherCAT (FoE) for the given Master instance
 The filename and data buffer must contain a valid address for this operation to complete successfully
 @param em_handle The master instance handle
 @param es_handle The slave handle
 @param filename The filename of the input data to be read over FoE
 @param file_length The length of the filename in bytes
 @param data A data buffer to be populated over FoE
 @param data_length The maximum data length for the buffer in bytes
 @param read_len The amount of data read by the FoE read operation
 @param password The FoE password
 @param timeout Timeout in milliseconds
 @return ddi_em_result The result of the FoE read operation @see ddi_em_result
 */
ddi_em_result ddi_em_foe_read(ddi_em_handle em_handle, ddi_es_handle es_handle, char *filename, uint32_t file_length,
  uint8_t *data, uint32_t data_length, uint32_t* read_len, uint32_t password, uint32_t timeout );

/** ddi_em_read_eeprom
 @brief Read the ESC eeprom registers
 @param[in] em_handle The master instance handle
 @param[in] es_handle The slave handle
 @param[in] offset The offset to start the ESC eeprom read at, in bytes
 @param[in] read_length The length of eeprom data to read, in words
 @param[out] out_len The length of eeprom data read, in words
 @param[in] timeout The Eeprom read timeout in milliseconds
 @return ddi_em_result The result of the Eeprom read operation @see ddi_em_result
 */
ddi_em_result ddi_em_read_eeprom(ddi_em_handle em_handle, ddi_es_handle es_handle, uint16_t offset, uint16_t *data, uint32_t read_len, uint32_t *out_len, uint32_t timeout);

/** ddi_em_write_eeprom
 @brief Write the ESC eeprom registers
 @param[in] em_handle The master instance handle
 @param[in] es_handle The slave handle
 @param[in] offset The offset to start the ESC eeprom write at, in bytes
 @param[in] data The length of eeprom data to read, in words
 @param[out] out_len The length of eeprom data read, in words
 @param[in] timeout The Eeprom write timeout in milliseconds
 @return ddi_em_result The result of the Eeprom read operation @see ddi_em_result
 */
ddi_em_result ddi_em_write_eeprom(ddi_em_handle em_handle, ddi_es_handle es_handle, uint16_t offset, uint16_t *data, uint32_t write_len, uint32_t timeout);

/** ddi_em_write_esc_reg
 @brief Write the ESC hardware registers
 @param[in] em_handle The master instance handle
 @param[in] es_handle The slave handle
 @param[in] offset The offset to start the ESC register write at, in bytes
 @param[out] data The ESC register data to write
 @param[in] write_len The length of eeprom data to write, in bytes
 @param[in] timeout The CoE timeout in milliseconds
 @return ddi_em_result The result of the ESC register write operation @see ddi_em_result
 */
ddi_em_result ddi_em_write_esc_reg(ddi_em_handle em_handle, ddi_es_handle es_handle, uint16_t offset, uint8_t *data, uint32_t write_len, uint32_t timeout);

/** ddi_em_read_esc_reg
 @brief Read the ESC hardware registers
 @param[in] em_handle The master instance handle
 @param[in] es_handle The slave handle
 @param[in] offset The offset to start the ESC register read at, in bytes
 @param[out] data The ESC register data to read
 @param[in] read_len The length of ESC register data to read, in bytes
 @param[in] timeout The CoE timeout in milliseconds
 @return ddi_em_result The result of the ESC register read operation @see ddi_em_result
 */
ddi_em_result ddi_em_read_esc_reg(ddi_em_handle em_handle, ddi_es_handle es_handle, uint16_t offset, uint8_t *data, uint32_t read_len, uint32_t timeout);

// Error and logging control -------------------------------------------------
/** ddi_em_get_alstatus_inst
 @brief Return the ALSTATUS (ESC Register 0x130) and ALSTATUS code (ESC Register 0x134) of a slave
 @param em_handle The EtherCAT master handle
 @param es_handle The EtherCAT master handle
 @param alstatus The ALSTATUS register (ESC register 0x130)
 @param alstatus_code the ALSTATUS_CODE (ESC register 0x134)
 @param timeout The timeout in milliseconds
 @return ddi_em_result The result of the alstatus read operation @see ddi_em_result
 */
ddi_em_result ddi_em_get_alstatus(ddi_em_handle em_handle, ddi_es_handle es_handle, uint16_t *alstatus, uint16_t *alstatus_code, uint32_t timeout);

/** ddi_em_get_error_string
 @brief Return an error string for the particular ddi_em_result code
  This function returns the string equivalent for the particular ddi_em_result code
  This function does not take a handle as a argument as it only operates on a 32-bit result code
 @param ddi_em_result_code The result code to be stringified
 @return The error string as a const char *
 */
const char * ddi_em_get_error_string (ddi_em_result ddi_em_result_code);

/** ddi_em_set_logging_level
 @brief Set the logging level for a particular master instance
 @param em_handle The master handle
 @param logging_level The logging level to be set @see ddi_em_logging_level
 @return ddi_em_result The string result for the ddi_em_result code @see ddi_em_result
 */
ddi_em_result ddi_em_set_logging_level (ddi_em_handle em_handle, ddi_em_logging_level logging_level);

#ifdef __cplusplus
}
#endif

#endif // DDI_EM_API
