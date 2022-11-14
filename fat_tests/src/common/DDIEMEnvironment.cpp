//
// Created by ddodd on 8/26/2021.
//
/**************************************************************************
(c) Copyright 2021 Digital Dynamics Inc. Scotts Valley CA USA.
Unpublished copyright. All rights reserved. Contains proprietary and
confidential trade secrets belonging to DDI. Disclosure or release without
prior written authorization of DDI is prohibited.
**************************************************************************/
#include <map>
#include <string>
#include <math.h>

#include "ddi_em_api.h"
#include "DDIEMEnvironment.h"
#include "EnvironmentRegistry.h"
#include "yarger.h"

using namespace std; // We probably shouldn't use namespace std in a header, but...

DDIEMEnvironment::DDIEMEnvironment(std::map<string, ddi::Option*> options) {
    m_options = options;
    m_initialized = false;
    m_train_rate = "";
    m_eni_file = "";
    m_nic = "a";
    m_display_rate = "";
    m_ecat_sync_mode = "";
    m_uart_channel = 0;
    m_uart_index = 0;
    std::map<string, ddi::Option*> m_options;
}

// Does not appear to be getting called
void DDIEMEnvironment::SetUp( ) {

    // Get the parameters from Yarger
    m_eni_file       = m_options["eni-file"]->get_value();
    m_train_rate     = m_options["train-rate"]->get_value();
    m_nic            = m_options["nic"]->get_value();
    m_display_rate   = m_options["display-rate"]->get_value();
    m_ecat_sync_mode = m_options["sync-mode"]->get_value();
    m_uart_channel   = atoi(m_options["uart-channel"]->get_value().c_str());
    m_uart_index     = atoi(m_options["uart-index"]->get_value().c_str());
    m_partial_mode   = atoi(m_options["partial-mode"]->get_value().c_str());

    // Set the filter of which tests we are going to run
    ::testing::GTEST_FLAG(filter)=m_options["testfilter"]->get_value();

    // Print out all arguments passed into the env file
    printf(GREEN "\nArguments taken:\nENI File: %s" "\n", m_eni_file.c_str());
    printf(GREEN "Train Rate: %d \n", stoi(m_train_rate));
    printf(GREEN "NIC: %s \n", m_nic.c_str());
    printf(GREEN "Display Rate: %d \n", stoi(m_display_rate));
    printf(GREEN "ECAT Sync Mode: %s \n", m_ecat_sync_mode.c_str());
    printf(GREEN "UART Channel: %d \n", m_uart_channel);
    printf(GREEN "UART Index: %d \n", m_uart_index);
    printf(GREEN "Partial Mode: %d \n", m_partial_mode);
    m_initialized = true;
    
    DLOG("DDIEMEnvironment::SetParams rate: %d eni: %s nic: %s \n", stoi(m_train_rate), m_eni_file.c_str(), m_nic.c_str());
}


void DDIEMEnvironment::TearDown() {}
