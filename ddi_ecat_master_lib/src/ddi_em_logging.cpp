/**************************************************************************
(c) Copyright 2021 Digital Dynamics Inc. Scotts Valley CA USA.
Unpublished copyright. All rights reserved. Contains proprietary and
confidential trade secrets belonging to DDI. Disclosure or release without
prior written authorization of DDI is prohibited.
**************************************************************************/

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "ddi_em_api.h"
#include "ddi_em_logging.h"
#include "ddi_em_config.h"
#include "ddi_defines.h"

// Per-instance logging control
static ddi_em_logging_level g_logging_level[DDI_EM_MAX_MASTER_INSTANCES];

// Log file for each EtherCAT Master Instance
static FILE *g_logging_fd[DDI_EM_MAX_MASTER_INSTANCES];

// Initailize the global logging structure
ddi_em_result ddi_em_log_init(void)
{
  int instance;
  for ( instance = 0; instance < DDI_EM_MAX_MASTER_INSTANCES; instance++ )
  {
    g_logging_fd[instance] = NULL;
    g_logging_level[instance] = DDI_EM_LOG_LEVEL_WARNINGS; // Enable errors and warnings by default
  }
  return DDI_EM_STATUS_OK;
}

// Close any open logging handles for this master instance
ddi_em_result ddi_em_logging_deinit (ddi_em_handle em_handle)
{
  if ( g_logging_fd[em_handle] )
  {
    fclose(g_logging_fd[em_handle]);
    g_logging_fd[em_handle] = NULL;
  }
  return DDI_EM_STATUS_OK;
}

// Function to log a message to persistent storage
ddi_em_result ddi_em_log(ddi_em_handle em_handle, ddi_em_logging_level log_level, const char *const fmt, ...)
{
  if (fmt == NULL)
    return DDI_EM_STATUS_INVALID_ARG;

  if (g_logging_level[em_handle] >= log_level)
  {
    if ( g_logging_fd[em_handle] == NULL )
      return DDI_EM_STATUS_OK;

    // Borrowed from fusion.IO.sdk/common/ddi_softuart.c, thanks Johana
    // Fusion firmware logs and DDI ECAT Master SDK logs will have the same timestamp formatting
    // 'YYYY-MM-DD hh:mm:ss.nnn \0' which has 19 characters from strftime and 6 chars from snprintf
    char buf[64];
    int len;
    struct timeval tv;

    //get the current time of day, convert it to the logging format
    gettimeofday(&tv, NULL);

    memset(buf, 0, sizeof(buf));
    // the timestamp format will always be a fixed size of 20 bytes including the NULL termination:
    len = strftime(buf, 20, "%Y-%m-%d %H:%M:%S", localtime(&tv.tv_sec));
    len += snprintf(&buf[len], 6, ".%03ld ", tv.tv_usec / 1000);
    len = MIN(len, 64);
    fwrite(buf, len, sizeof(uint8_t), g_logging_fd[em_handle]);
    // log the variable arguments to file
    va_list args;
    va_start(args, fmt);
    vfprintf(g_logging_fd[em_handle], fmt, args);
    va_end(args);
    fflush(g_logging_fd[em_handle]);
  }
  return DDI_EM_STATUS_OK;
}

// Initialize the logging subsystem for this instance
ddi_em_result ddi_em_logging_init (ddi_em_handle em_handle)
{
  char logfile_name[256], *log_dir_name;
  struct stat st = {0};
  log_dir_name = getenv("DDI_EM_LOG_DIR");

  // If the DDI_EM_LOG_DIR directory variable does not exist, use the default directory
  if ( log_dir_name == NULL )
  {
    log_dir_name = (char *)DDI_EM_LOG_DIR;
  }

  // Check for logging directory and create if needed
  if (stat(log_dir_name, &st) == -1)
  {
    if ( mkdir(log_dir_name, 0755) )
    {
      return DDI_EM_STATUS_FILE_OPEN_ERR;
    }
  }

  // Format the log file name for this instance
  sprintf(logfile_name, "%s" "/" DDI_EM_LOG_FILE_PREFIX"_%d.log",log_dir_name,em_handle);
  g_logging_fd[em_handle] = fopen(logfile_name, "a+");
  if ( g_logging_fd[em_handle] == NULL )
    return DDI_EM_STATUS_FILE_OPEN_ERR;
  // Enable errors and warnings by default
  g_logging_level[em_handle] = DDI_EM_LOG_LEVEL_WARNINGS;
  return DDI_EM_STATUS_OK;
}

// Set the logging level for the master instance
EM_API ddi_em_result ddi_em_set_logging_level (ddi_em_handle em_handle, ddi_em_logging_level logging_level)
{
  VALIDATE_INSTANCE(em_handle); // Validate the instance argument
  g_logging_level[em_handle] = logging_level;
  return DDI_EM_STATUS_OK;
}
