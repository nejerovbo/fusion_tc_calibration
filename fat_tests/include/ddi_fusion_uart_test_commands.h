#ifndef DDI_FUSION_UART_TEST_COMMANDS_H
#define DDI_FUSION_UART_TEST_COMMANDS_H

#include "gtest/gtest.h"
#include "yarger.h"
#include "DDIEMEnvironment.h"
#include "AcontisEnvironment.h"

class Fusion_UART_param_test: public DDICommand 
{
public:
  using DDICommand::DDICommand;
  virtual string GetTestFilter() { return "*DDIEM_UART_param*"; }
  virtual string GetDescription() { return "Tests for fusion_UART_api support in ddi_ecat_master_lib"; }
  virtual ::testing::Environment* GetEnvironment() {
    return new DDIEMEnvironment(m_parser->get_command().get_options_map());
  }
};

class Fusion_UART_232_loopback_test : public DDICommand
{
public:
  using DDICommand::DDICommand;
  virtual string GetTestFilter() { return "*DDIEM_UART_232_loopback*"; }
  virtual string GetDescription() { return "RS-232 Loopback Test"; }
  virtual ::testing::Environment* GetEnvironment() {
    return new DDIEMEnvironment(m_parser->get_command().get_options_map());
  }
};

class Fusion_UART_232_loopback_event_test : public DDICommand
{
public:
  using DDICommand::DDICommand;
  virtual string GetTestFilter() { return "*DDIEM_UART_232_loopback_event*"; }
  virtual string GetDescription() { return "RS-232 Loopback Test"; }
  virtual ::testing::Environment* GetEnvironment() {
    return new DDIEMEnvironment(m_parser->get_command().get_options_map());
  }
};

class Fusion_UART_485_loopback_test : public DDICommand 
{
public:
  using DDICommand::DDICommand;
  virtual string GetTestFilter() { return "*DDIEM_UART_485_loopback*"; }
  virtual string GetDescription() { return "RS-485 Loopback Test"; }
  virtual ::testing::Environment* GetEnvironment() {
    return new DDIEMEnvironment(m_parser->get_command().get_options_map());
  }
};

class Fusion_UART_all_configs_loopback_test : public DDICommand
{
public:
  using DDICommand::DDICommand;
  virtual string GetTestFilter() { return "*DDIEM_UART_all_configs_loopback*"; }
  virtual string GetDescription() { return "Loopback with all the possible config combinations"; }
  virtual ::testing::Environment* GetEnvironment() {
    return new DDIEMEnvironment(m_parser->get_command().get_options_map());
  }
};

class Fusion_UART_stress_test : public DDICommand
{
public:
  using DDICommand::DDICommand;
  virtual string GetTestFilter() { return "*DDIEM_UART_Stress*"; }
  virtual string GetDescription() { return "Run loopback on all UARTs and all channels."; }
  virtual ::testing::Environment* GetEnvironment() {
    return new DDIEMEnvironment(m_parser->get_command().get_options_map());
  }
};

class Fusion_UART_transfer_test : public DDICommand
{
public:
  using DDICommand::DDICommand;
  virtual string GetTestFilter() { return "*DDIEM_UART_Xfer*"; }
  virtual string GetDescription() { return "Run data transfer to/from USB dongle to UART."; }
  virtual ::testing::Environment* GetEnvironment() {
    return new DDIEMEnvironment(m_parser->get_command().get_options_map());
  }
};

#endif
