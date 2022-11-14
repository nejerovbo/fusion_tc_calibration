#ifndef UCSC_H
#define UCSC_H

#include "EnvironmentProvider.h"

class UCSCCommand : public DDICommand 
{
public:
  using DDICommand::DDICommand;
  virtual string GetTestFilter() { return "*UCSC*"; }
  virtual string GetDescription() { return "UCSC Sample App"; }
};

#endif
