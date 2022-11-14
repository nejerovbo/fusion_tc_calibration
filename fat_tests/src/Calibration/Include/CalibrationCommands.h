/*****************************************************************************
 * (c) Copyright 2022 Digital Dynamics Inc. Scotts Valley CA USA.
 * Unpublished copyright. All rights reserved. Contains proprietary and
 * confidential trade secrets belonging to DDI. Disclosure or release without
 * prior written authorization of DDI is prohibited.
 *****************************************************************************/

#ifndef CALIBRATION_COMMANDS_H
#define CALIBRATION_COMMANDS_H

#include "yarger.h"
#include "EnvironmentProvider.h"


class CovertMATLABModel : public DDICommand {
public:
    using DDICommand::DDICommand;
    virtual string GetTestFilter() { return "*CreateCalFromMATLAB*"; }
    virtual string GetDescription() { return "Create a calibration file from MATLAB"; }
};

class DMMResolutionTest: public DDICommand {
    using DDICommand::DDICommand;
    virtual string GetTestFilter() { return "*AOTestDMM*"; }
    virtual string GetDescription() { return "DMM Resolution test"; }    
};

class MultiAIOCalibration8 : public DDICommand {
public:
    using DDICommand::DDICommand;
    virtual string GetTestFilter() { return "*Multi_AIOCal8*"; }
    virtual string GetDescription() { return "Multiple cards cal"; }
};

class MultiAIOTest8 : public DDICommand {
public:
    using DDICommand::DDICommand;
    virtual string GetTestFilter() { return "*MultiAIOTest8*"; }
    virtual string GetDescription() { return "Multiple cards verification"; }
};

class MultiAIOCalibration16 : public DDICommand {
public:
    using DDICommand::DDICommand;
    virtual string GetTestFilter() { return "*Multi_AIOCal16*"; }
    virtual string GetDescription() { return "Multiple cards cal"; }
};

class MultiAIOTest16 : public DDICommand {
public:
    using DDICommand::DDICommand;
    virtual string GetTestFilter() { return "*MultiAIOTest16*"; }
    virtual string GetDescription() { return "Multiple cards verification"; }
};

class ReturnPartNumber : public DDICommand {
public:
    using DDICommand::DDICommand;
    virtual string GetTestFilter() { return "*ReturnPartNumber*"; }
    virtual string GetDescription() { return "Return the Part Number of Fusion"; }
};

class TCTest : public DDICommand {
public:
    using DDICommand::DDICommand;
    virtual string GetTestFilter() { return "*TCTest*"; }
    virtual string GetDescription() { return "Thermocouple Test card"; }
};

class TCVerify : public DDICommand {
public:
    using DDICommand::DDICommand;
    virtual string GetTestFilter() { return "*TCVerify*"; }
    virtual string GetDescription() { return "Thermocouple card verification"; }
};

class RTDVerify : public DDICommand {
public:
    using DDICommand::DDICommand;
    virtual string GetTestFilter() { return "*RTDVerify*"; }
    virtual string GetDescription() { return "RTD card verification"; }
};

#endif // CALIBRATION_COMMANDS_H
