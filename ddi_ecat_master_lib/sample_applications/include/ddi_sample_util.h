/**************************************************************************
(c) Copyright 2021 Digital Dynamics Inc. Scotts Valley CA USA.
Unpublished copyright. All rights reserved. Contains proprietary and
confidential trade secrets belonging to DDI. Disclosure or release without
prior written authorization of DDI is prohibited.
**************************************************************************/

#ifndef __DDI_SAMPLE_UTIL_H
#define __DDI_SAMPLE_UTIL_H

#include <stdio.h>

// Utilities for sample applications

#define RED       "\033[0;91m"
#define GREEN     "\033[1;92m"
#define YELLOW    "\033[1;93m"
#define ORANGE    "\033[0;93m"
#define BLUE      "\033[1;94m"
#define VIOLET    "\033[0;95m"
#define CYAN      "\033[0;96m"
#define CLEAR     "\033[0m"
  
#define PRINTF_GOTO_XY(x,y) do{ printf("\033[%d;%dH", y, x); printf("\33[2K"); } while(0)

#define clrscr()  printf("\e[1;1H\e[2J")

#define LOG_PREFIX "DDI Sample App: "

#define ELOG(fmt, ...) do{ printf(LOG_PREFIX RED "ERROR: " fmt CLEAR, ##__VA_ARGS__ );  }while(0)
#define WLOG(fmt, ...) do{ printf(LOG_PREFIX YELLOW "WARNING: " fmt CLEAR, ##__VA_ARGS__ ); }while(0)
#define DLOG(...)      do{ printf(LOG_PREFIX __VA_ARGS__); }while(0)
#define VLOG(...)      do{ printf(LOG_PREFIX __VA_ARGS__); }while(0)

#endif
