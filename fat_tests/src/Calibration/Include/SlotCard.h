/**************************************************************************
(c) Copyright 2022 Digital Dynamics Inc. Scotts Valley CA USA.
Unpublished copyright. All rights reserved. Contains proprietary and
confidential trade secrets belonging to DDI. Disclosure or release without
prior written authorization of DDI is prohibited.
**************************************************************************/

#ifndef SLOTCARD_H
#define SLOTCARD_H

#include <fcntl.h>
#include <stdint.h>
#include "AcontisTestFixture.h"
#include "IOChannel.h"

class SlotCard
{
public:
  virtual IOChannel get_ioChannel(int index) = 0;
  virtual void set_ioChannel(int index, IOChannel arg) = 0;
  
};

#endif
