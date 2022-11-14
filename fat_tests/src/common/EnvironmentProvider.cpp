//
// Created by ddodd on 8/27/2021.
//
#include <vector>

#include "EnvironmentProvider.h"
#include "EnvironmentRegistry.h"
#include "AcontisEnvironment.h"
#include "yarger.h"
#include "DVTCommands.h"
#include "DDIEMTestCommands.h"
#include "InterlockBridge.h"
#include "UART.h"
#include "ddi_fusion_uart_test_commands.h"
#include "ASMI.h"
#include "CalibrationCommands.h"


DDIEnvironmentProvider::DDIEnvironmentProvider(int argc, char *argv[]) {
    m_argc = argc;
    m_argv = argv;
}


::testing::Environment* DDIEnvironmentProvider::GetEnvironment() {    
    
    vector<DDICommand*> testCommands;
    ddi::ArgumentParser* parser = new ArgumentParser("FAT", "Fusion Automated Tester");
    
    // Add all our tests
    testCommands.push_back(new AnalogInputLatency("ainlatency"));
    testCommands.push_back(new AnalogInputStepResponse("ainstep"));
    testCommands.push_back(new AnalogOutputLatency("aoutstep"));
    testCommands.push_back(new AIOCombinedAccuracy("aioaccuracy"));
    testCommands.push_back(new AOOutputAccuracy("aoaccuracy"));
    testCommands.push_back(new AOOutputThermal("ao-thermal"));
    testCommands.push_back(new AODCCoupling("aodccoupling"));
    testCommands.push_back(new AIInputAccuracy("aiaccuracy"));
    testCommands.push_back(new InterlockBridge("ib"));
    testCommands.push_back(new DigitalInputSwitchingThreshold("dinthresh"));
    testCommands.push_back(new DigitalOutputLatency("doutlatency"));
    testCommands.push_back(new DigitalInputLatency("dinlatency"));
    testCommands.push_back(new EtherCATDataTimestamp("ecattimestamp"));
    testCommands.push_back(new AnalogInputLatencyBeckhoff("ainlatency-beckhoff"));
    testCommands.push_back(new DigitalInputLatencyBeckhoff("dinlatency-beckhoff"));
    testCommands.push_back(new UART_SDO("UART_SDO"));
    testCommands.push_back(new UART_Loopback("UART_Loopback"));
    testCommands.push_back(new UART_Match("UART_Match"));
    testCommands.push_back(new UART_Tenma("UART_Tenma"));
    testCommands.push_back(new AnalogInputFreqResponse("aifreqresponse-beckhoff"));
    testCommands.push_back(new DDIEMBasicTest("ddi-em-basic"));
    testCommands.push_back(new Fusion_UART_param_test("fusion_uart_param"));
    testCommands.push_back(new Fusion_UART_transfer_test("fusion_uart_transfer"));
    testCommands.push_back(new Fusion_UART_stress_test("fusion_uart_stress"));
    testCommands.push_back(new Fusion_UART_232_loopback_test("fusion_uart_232_loopback"));
    testCommands.push_back(new Fusion_UART_485_loopback_test("fusion_uart_485_loopback"));
    testCommands.push_back(new ASMICommand("ASMI_IO"));
    testCommands.push_back(new DDIEM_E84LoopbackTest("E84_loopback"));
    testCommands.push_back(new EChuckCommand("EChuck_IO"));
    testCommands.push_back(new Fusion_UART_232_loopback_event_test("fusion_uart_232_loopback_event"));
    testCommands.push_back(new DDIEM_PartialModeTest("ddi-em-partial"));
    testCommands.push_back(new DDIEM_Echuck_Test("echuck-op-mode"));
    testCommands.push_back(new CovertMATLABModel("convert-MATLAB"));
    testCommands.push_back(new DMMResolutionTest("AOTestDMM"));
    testCommands.push_back(new MultiAIOCalibration8("maio8"));
    testCommands.push_back(new MultiAIOTest8("mtaio8"));
    testCommands.push_back(new MultiAIOCalibration16("maio16"));
    testCommands.push_back(new MultiAIOTest16("mtaio16"));
    testCommands.push_back(new ReturnPartNumber("return-part-number"));
    testCommands.push_back(new TCTest("tc_data"));
    testCommands.push_back(new TCVerify("tc_verify"));
    testCommands.push_back(new RTDVerify("rtd_verify"));
    
    // Initialize test commands
    for (auto & command : testCommands) {
        command->Initialize(parser);
    }

    // Parse the command line arguments with Yarger
    int retval = parser->parse(m_argc, m_argv);
    if (retval == 0) {
        DDICommand& command = (DDICommand&) parser->get_command();
        string command_name = command.get_name();

        ::testing::Environment* environment = command.GetEnvironment();
        ::testing::GTEST_FLAG(filter) = command.GetTestFilter();

        
        // Register the environment
        g_environment_registry.register_environment(DDIEM_ENVIRONMENT_NAME, environment );

        return environment;
        
    } else {
        // If we had an error while parsing the print out some help
        if (parser->get_command().get_runner()) {
            parser->get_command().get_runner()->run();
        }
    }

    return nullptr;
}


/**
 * Adds a default set of options to the given command.
 * @param command
 */
void add_default_options(ddi::Command& command) {

    command.add_option("eni-file").shortname("e").
        description("eni input file");
    command.add_option("train-rate").shortname("s").
        description("Train rate (us)").defaultvalue("1000");
    command.add_option("nic").shortname("i").
        description("Network interface to use").defaultvalue("i8254:2");
    command.add_option("display-rate").shortname("d").
        description("Display rate in cyclic frames").defaultvalue("10000");
    command.add_option("sync-mode").shortname("m").
        description("Ethercat sync mode").defaultvalue("sync");
    command.add_option("uart-channel").shortname("c").
        description("uart-channel-from-0-3");
    command.add_option("uart-index").shortname("n").
        description("UART-index");
    command.add_option("partial-mode").shortname("p").
        description("Partial mode").defaultvalue("0");
}

