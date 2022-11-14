/*****************************************************************************
 * (c) Copyright 2021 Digital Dynamics Inc. Scotts Valley CA USA.
 * Unpublished copyright. All rights reserved. Contains proprietary and
 * confidential trade secrets belonging to DDI. Disclosure or release without
 * prior written authorization of DDI is prohibited.
 *****************************************************************************/

// Sample Test Application for Conan/Cmake

#include <iostream>
#include "ddi_debug.h"

int ddi_log_level = LOG_LEVEL_VERBOSE;

int main(void)
{
  std::cout << "Hello from conan land";
  DLOG("Hello World \n");
  return 0;
}
