#ifndef UART_CONFIG
#define UART_CONFIG

#include "ddi_debug.h" // For color information

#define SDO_DESCRIPTION \
 (GREEN "UART_SDO" CLEAR "\n" \
        "                   ->Provides an SDOTest for UARTs\n" \
        "                   ->Tests baud rate,parity mode\n" \
        "                   ->" YELLOW "Requires a CRAM/UART slot-card" CLEAR "\n" )

#define LOOPBACK_DESCRIPTION \
 (GREEN "UART_Loopback" CLEAR "\n" \
        "                   ->Provides a loopback test for UARTs\n" \
        "                   ->Tests UART Tx/Rx in loopback mode\n" \
        "                   ->" YELLOW "Requires a cRAM/UART slot-card with 4 channels in loopback mode" CLEAR "\n" )

#define MATCH_DESCRIPTION \
 (GREEN "UART_Match" CLEAR "\n" \
        "                   ->Provides a test application for interfacing with the Mattson match\n" \
        "                   ->Tests serial communication with a Mattson device\n" \
        "                   ->" YELLOW "Requires a CRAM/UART slot-card with an external impedance match" CLEAR "\n" )

// Tests basic UART functionality
class UART_SDO : public DDICommand {
public:
    using DDICommand::DDICommand;
    virtual string GetTestFilter() { return "*UART_SDO*"; }
    virtual string GetDescription()
    {
        return SDO_DESCRIPTION;
    }
};

// Tests basic UART Loopback functionality
class UART_Loopback : public DDICommand {
public:
    using DDICommand::DDICommand;
    virtual string GetTestFilter() { return "*UART_Loopback*"; }
    virtual string GetDescription()
    {
      return LOOPBACK_DESCRIPTION;
    }
};

// Tests basic UART with Ad-tec RF Match
class UART_Match : public DDICommand {
public:
    using DDICommand::DDICommand;
    virtual string GetTestFilter() { return "*UART_Match*"; }
    virtual string GetDescription()
    {
      return MATCH_DESCRIPTION;
    }
};

// Tests basic Tenma Command
class UART_Tenma : public DDICommand {
public:
    using DDICommand::DDICommand;
    virtual string GetTestFilter() { return "*UART_Tenma*"; }
    virtual string GetDescription()
    {
      return MATCH_DESCRIPTION;
    }
};

#endif
