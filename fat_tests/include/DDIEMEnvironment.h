//
// Created by ddodd on 8/26/2021.
//
/**************************************************************************
(c) Copyright 2021 Digital Dynamics Inc. Scotts Valley CA USA.
Unpublished copyright. All rights reserved. Contains proprietary and
confidential trade secrets belonging to DDI. Disclosure or release without
prior written authorization of DDI is prohibited.
**************************************************************************/
#ifndef DDI_ETHERCAT_MASTER_ENVIRONMENT_H
#define DDI_ETHERCAT_MASTER_ENVIRONMENT_H
#include <string>
#include "gtest/gtest.h"
#include "CyclicData.h"
#include "yarger.h"
#define DEBUG

#include "ddi_sdk_common.h"
#include "ddi_sdk_ecat_master.h"
#include "ddi_sdk_ecat_sdo.h"
#include "ddi_status.h"
#include "ddi_sdk_fusion_interface.h"

using namespace std;

#define DDIEM_ENVIRONMENT_NAME "DDIEM"

/**
 * The DDIEM environment is used to run tests based on ddi_ecat_master_lib.  All setup is performed in the Setup method.
 * This is a one time per invocation of the tests.  Multiple tests can be invoked with the same environment.
 */
class DDIEMEnvironment  : public ::testing::Environment {
    bool               m_initialized;
    string             m_train_rate;
    string             m_eni_file;
    string             m_nic;
    string             m_display_rate;
    string             m_ecat_sync_mode;
    int                m_uart_channel;
    int                m_uart_index;
    int                m_partial_mode;       // 0 for false(default), 1 for true
    std::map<string, ddi::Option*> m_options;

public:

    DDIEMEnvironment(std::map<string, ddi::Option*>);

    // Override this to define how to set up the environment.
    void SetUp() override;

    // Override this to define how to tear down the environment.
    void TearDown() override;

    string GetEcatSyncMode(void) { return m_ecat_sync_mode; }
    string GetTrainRate(void)    { return m_train_rate; }
    string GetEniFile(void)      { return m_eni_file; }
    string GetNIC(void)          { return m_nic; }
    int GetUARTChannel(void)     { return m_uart_channel; }
    int GetUARTIndex(void)       { return m_uart_index; }   
    bool IsPartialMode(void)     { return m_partial_mode; }
};

#endif //DDI_ETHERCAT_MASTER_ENVIRONMENT_H
