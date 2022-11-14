/**************************************************************************
(c) Copyright 2021 Digital Dynamics Inc. Scotts Valley CA USA.
Unpublished copyright. All rights reserved. Contains proprietary and
confidential trade secrets belonging to DDI. Disclosure or release without
prior written authorization of DDI is prohibited.
**************************************************************************/

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include "ddi_defines.h"
#include "ddi_em_common.h"
#include "ddi_debug.h"

// Return the keyboard input. Only the non-blocking version is implemented currently.
ddi_status_t ddi_get_keyboard_input(char *key, bool blocking)
{
  static bool initialized = false;
  static int fd;
  static const char *device = "/dev/tty";
  int bytes;
  if ( initialized == false)
  {
    // Open Keyboard
    fd = open(device, O_RDONLY | O_NONBLOCK);
    if(fd == -1)
    {
        ELOG("ERROR Opening %s\n", device);
        return ddi_status_err;
    }
    initialized = true;
  }
  // Read Keyboard Data
  bytes = read(fd, key, 1);
  if(bytes > 0)
  {
    return ddi_status_ok;
  }
  else
  {
    return ddi_status_err;
  }
}
