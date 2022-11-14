/**************************************************************************
(c) Copyright 2021 Digital Dynamics Inc. Scotts Valley CA USA.
Unpublished copyright. All rights reserved. Contains proprietary and
confidential trade secrets belonging to DDI. Disclosure or release without
prior written authorization of DDI is prohibited.
**************************************************************************/

#ifndef DDI_EM_LOGGING_H
#define DDI_EM_LOGGING_H

/** ddi_em_log
 @brief Function to log a message to persistent storage
 @param instance The Master instance handle
 @param log_level The log level this message has
 @param fmt Variadic arguments
 @return ddi_em_result The result code of the operation, see ddi_em_result for details
 @see ddi_em_result
 */
ddi_em_result ddi_em_log(ddi_em_handle em_instance, ddi_em_logging_level log_level, const char *const fmt, ...);

// Initialize the logging subsytem
/** ddi_em_log_init
 @brief Function to log a message to persistent storage
 @return ddi_em_result The result code of the operation, see ddi_em_result for details
 @see ddi_em_result
 */
ddi_em_result ddi_em_log_init(void);

/** ddi_em_log_init
 @brief Initialize the logging subsytem for a master instance
 @param instance The EtherCAT master handle
 @return ddi_em_result The result code of the operation, see ddi_em_result for details
 @see ddi_em_result
 */
ddi_em_result ddi_em_logging_init (ddi_em_handle instance);

/** ddi_em_logging_deinit
 @brief De-initialize the logging subsytem for a master instance
 @param instance The EtherCAT master handle
 @return ddi_em_result The result code of the operation, see ddi_em_result for details
 @see ddi_em_result
 */
ddi_em_result ddi_em_logging_deinit (ddi_em_handle instance);

// Start all logs with a " [DDI EM SDK]: message"
#undef LOG_PREFIX
#define LOG_PREFIX "[DDI EM SDK]: "

// Undefine any current ELOG/DLOG/VLOGs and replace them with a master-specific log file
#undef ELOG
#undef WLOG
#undef DLOG
#undef VLOG
#define ELOG(instance, fmt, ...) do{ ddi_em_log(instance, DDI_EM_LOG_LEVEL_ERRORS, LOG_PREFIX RED "ERROR: " fmt CLEAR, ##__VA_ARGS__ );  }while(0)
#define WLOG(instance, fmt, ...) do{ ddi_em_log(instance, DDI_EM_LOG_LEVEL_WARNINGS, LOG_PREFIX YELLOW "WARNING: " fmt CLEAR, ##__VA_ARGS__ ); }while(0)
#define DLOG(instance, ...)      do{ ddi_em_log(instance, DDI_EM_LOG_LEVEL_DEBUG, LOG_PREFIX __VA_ARGS__); }while(0)
#define VLOG(instance, ...)      do{ ddi_em_log(instance, DDI_EM_LOG_LEVEL_VERBOSE, LOG_PREFIX __VA_ARGS__); }while(0)

#endif
