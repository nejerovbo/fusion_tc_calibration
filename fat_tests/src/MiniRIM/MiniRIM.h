#ifndef MINIRIM_H
#define MINIRIM_H

#include "EnvironmentProvider.h"

class MiniRIMCommand : public DDICommand {
public:
    using DDICommand::DDICommand;
    virtual string GetTestFilter() { return "*MiniRIM_Test*"; }
    virtual string GetDescription() { return "MiniRIM Development Platform"; }
};

#endif