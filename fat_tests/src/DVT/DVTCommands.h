//
// Created by ddodd on 9/18/2021.
//

#ifndef SAMPLE_TEST_APPS_DVTCOMMANDS_H
#define SAMPLE_TEST_APPS_DVTCOMMANDS_H

#include "yarger.h"
#include "EnvironmentProvider.h"

class AnalogInputLatencyBeckhoff : public DDICommand {
public:
    using DDICommand::DDICommand;
    virtual string GetTestFilter() { return "*DVT_AINLatency_Beckhoff*"; }
    virtual string GetDescription() { return "AIN latency using Beckhoff AOUT"; }
};

class AnalogInputLatency : public DDICommand {
public:
    using DDICommand::DDICommand;
    virtual string GetTestFilter() { return "*AINLatency*"; }
    virtual string GetDescription() { return "AnalogInputLatency"; }
};

class AnalogInputStepResponse : public DDICommand {
public:
    using DDICommand::DDICommand;
    virtual string GetTestFilter() { return "*AINStepResponse*"; };
    virtual string GetDescription() { return "AnalogInputStepResponse"; }
};

class AnalogInputFreqResponse : public DDICommand {
public:
    using DDICommand::DDICommand;
    virtual string GetTestFilter() { return "*DVT_AINFreqResponse_Beckhoff*"; };
    virtual string GetDescription() { return "AIN frequency response using Beckhoff AOUT"; }
};


class AnalogOutputLatency : public DDICommand {
public:
    using DDICommand::DDICommand;
    virtual string GetTestFilter() { return "*AOUTLatency*"; }
    virtual string GetDescription() { return "AnalogOutputLatency"; }
};


class DigitalInputSwitchingThreshold : public DDICommand {
public:
    using DDICommand::DDICommand;
    virtual string GetTestFilter() { return "*DINSwitchingThreshold*"; }
    virtual string GetDescription() { return "DigitalInputSwitchingThreshold"; }
};

class AIOCombinedAccuracy : public DDICommand {
public:
    using DDICommand::DDICommand;
    virtual string GetTestFilter() { return "*AIOCombinedAccuracy*"; }
    virtual string GetDescription() { return "AIOCombinedAccuracy"; }
};

class AIInputAccuracy : public DDICommand {
public:
    using DDICommand::DDICommand;
    virtual string GetTestFilter() { return "*AIInputAccuracy*"; }
    virtual string GetDescription() { return "AIInputAccuracy"; }
};

class AOOutputAccuracy : public DDICommand {
public:
    using DDICommand::DDICommand;
    virtual string GetTestFilter() { return "*AOOutputAccuracy*"; }
    virtual string GetDescription() { return "AOOutputAccuracy"; }
};

class AOOutputThermal : public DDICommand {
public:
    using DDICommand::DDICommand;
    virtual string GetTestFilter() { return "*AOOutputThermal*"; }
    virtual string GetDescription() { return "AOOutputThermal"; }
};

class AODCCoupling : public DDICommand {
public:
    using DDICommand::DDICommand;
    virtual string GetTestFilter() { return "*AODCCoupling*"; }
    virtual string GetDescription() { return "AODCCoupling"; }
};

class DigitalOutputLatency : public DDICommand {
public:
    using DDICommand::DDICommand;
    virtual string GetTestFilter() { return "*DOUTLatency*"; }
    virtual string GetDescription() { return "DigitalOutputLatency"; }
};

class DigitalInputLatency : public DDICommand {
public:
    using DDICommand::DDICommand;
    virtual string GetTestFilter() { return "*DINLatency*"; }
    virtual string GetDescription() { return "DigitalInputLatency"; }
};

class DigitalInputLatencyBeckhoff : public DDICommand {
public:
    using DDICommand::DDICommand;
    virtual string GetTestFilter() { return "*DVT_DINLatency_Beckhoff*"; }
    virtual string GetDescription() { return "DigitalInputLatency using Beckhoff AOUT"; }
};


class EtherCATDataTimestamp : public DDICommand {
public:
    using DDICommand::DDICommand;
    virtual string GetTestFilter() { return "*DVT_TimeStampTest*"; }
    virtual string GetDescription() { return "EtherCAT Data Timestamp"; }
};

#endif //SAMPLE_TEST_APPS_DVTCOMMANDS_H
