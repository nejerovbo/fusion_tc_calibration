/*****************************************************************************
 * (c) Copyright 2018 Digital Dynamics Inc. Scotts Valley CA USA.
 * Unpublished copyright. All rights reserved. Contains proprietary and
 * confidential trade secrets belonging to DDI. Disclosure or release without
 * prior written authorization of DDI is prohibited.
 *****************************************************************************/
/*  ddi_debug.h
 *  Created by Johana Lehrer on 2018-10-02
 */

#ifndef DDI_DEBUG_H
#define DDI_DEBUG_H

#include <stdint.h>
#include <stdarg.h>

#define LOG_LEVEL_SILENT    0
#define LOG_LEVEL_ERRORS    1
#define LOG_LEVEL_DEBUG     2
#define LOG_LEVEL_VERBOSE   3

#define RED       "\033[0;91m"
#define GREEN     "\033[1;92m"
#define YELLOW    "\033[1;93m"
#define ORANGE    "\033[0;93m"
#define BLUE      "\033[1;94m"
#define VIOLET    "\033[0;95m"
#define CYAN      "\033[0;96m"
#define CLEAR     "\033[0m"

#ifndef LOG_PREFIX
#define LOG_PREFIX
#endif // LOG_PREFIX

#ifdef DEBUG

extern int ddi_log_level; // <<-- must be instanciated somewhere
#ifdef DDI_DEBUG_PRINTF
extern int ddi_log_printf(const char *const fmt, ...);
#else
#define ddi_log_printf printf
#endif

#define DDI_SET_LOG_LEVEL(level) do {ddi_log_level = level;} while(0)

#define ELOG(fmt, ...) do{ if (ddi_log_level >= LOG_LEVEL_ERRORS) {ddi_log_printf(LOG_PREFIX RED "ERROR: " fmt CLEAR, ##__VA_ARGS__ ); fflush(0);} }while(0)
#define DLOG(...)      do{ if (ddi_log_level >= LOG_LEVEL_DEBUG)  {ddi_log_printf(LOG_PREFIX __VA_ARGS__); fflush(0);} }while(0)
#define VLOG(...)      do{ if (ddi_log_level >= LOG_LEVEL_VERBOSE) {ddi_log_printf(LOG_PREFIX __VA_ARGS__); fflush(0);} }while(0)

#else

#define DDI_SET_LOG_LEVEL(level)

#define ELOG(...)
#define DLOG(...)
#define VLOG(...)

#endif

#endif // DDI_DEBUG_H

