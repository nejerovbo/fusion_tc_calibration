
#ifndef DDI_EM_TEST_COMMANDS_H
#define DDI_EM_TEST_COMMANDS_H

#include "gtest/gtest.h"
#include "yarger.h"
#include "DDIEMEnvironment.h"
#include "AcontisEnvironment.h"

// Basic EtherCAT Master tests
class DDIEMBasicTest : public DDICommand {
public:
  using DDICommand::DDICommand;
  virtual string GetTestFilter() { return "DDIEM_Basic*"; }
  virtual string GetDescription() { return "DDIEM basic tests"; }
  virtual ::testing::Environment* GetEnvironment() {
    return new DDIEMEnvironment(m_parser->get_command().get_options_map());
  }
};

// E-84 loopback tests using the DDI ECAT Master SDK
class DDIEM_E84LoopbackTest: public DDICommand {
  using DDICommand::DDICommand;
  virtual string GetTestFilter() { return "*DDIEM_E84_loopback*"; }
  virtual string GetDescription() { return "E-84 loopback test"; }
  virtual ::testing::Environment* GetEnvironment() {
    return new DDIEMEnvironment(m_parser->get_command().get_options_map());
  }
};

// Partial mode test using the DDI ECAT Master SDK
class DDIEM_PartialModeTest: public DDICommand {
  using DDICommand::DDICommand;
  virtual string GetTestFilter() { return "*DDIEM_Partial*"; }
  virtual string GetDescription() { return "DDIEM Partial Mode Test"; }
  virtual ::testing::Environment* GetEnvironment() {
    return new DDIEMEnvironment(m_parser->get_command().get_options_map());
  }
};

// Partial mode test using the DDI ECAT Master SDK
class DDIEM_Echuck_Test: public DDICommand {
  using DDICommand::DDICommand;
  virtual string GetTestFilter() { return "*DDIEM_Echuck*"; }
  virtual string GetDescription() { return "DDIEM Echuck OP Test"; }
  virtual ::testing::Environment* GetEnvironment() {
    return new DDIEMEnvironment(m_parser->get_command().get_options_map());
  }
};


class EChuckCommand : public DDICommand 
{
public:
  using DDICommand::DDICommand;
  virtual string GetTestFilter() { return "*EChuck*"; }
  virtual string GetDescription() { return "EChuck Sample App"; }
  virtual ::testing::Environment* GetEnvironment() {
    return new DDIEMEnvironment(m_parser->get_command().get_options_map());
  }
};

#endif
