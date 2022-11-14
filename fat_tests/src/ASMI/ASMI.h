#ifndef ASMI_H
#define ASMI_H

#include "EnvironmentProvider.h"

class ASMICommand : public DDICommand {
public:
    using DDICommand::DDICommand;
    virtual string GetTestFilter() { return "*ASMI*"; }
    virtual string GetDescription() { return "ASMI Development Platform"; }
};

#endif
