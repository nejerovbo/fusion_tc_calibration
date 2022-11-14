//
// Created by ddodd on 8/26/2021.
//

#ifndef SAMPLE_TEST_APPS_ACONTISENVIRONMENT_H
#define SAMPLE_TEST_APPS_ACONTISENVIRONMENT_H
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

#define ACONTIS_ENVIRONMENT_NAME "AcontisMaster"

/**
 * The Acontis environment is used to run tests based on acontis.  Al setup is performed in the Setup method.
 * This is a one time per invocation of the tests.  Multiple tests can be invoked with the same environment.
 */
class AcontisEnvironment  : public ::testing::Environment {
    AcontisCyclicData* m_cyclic_data;
    bool               m_initialized;
    string             m_train_rate;
    string             m_eni_file;
    string             m_nic;
    string             m_display_rate;
    string             m_ecat_sync_mode;
    std::map<string, ddi::Option*> m_options;

public:

    AcontisEnvironment(std::map<string, ddi::Option*>);

    // Override this to define how to set up the environment.
    void SetUp() override ;

    // Override this to define how to tear down the environment.
    void TearDown() override;

    /**
     * Initializes the SDK
     *
     * This method iniitalizes the Acontis SDK.
     *
     */
    void InitSDK(void);

    string GetEcatSyncMode(void) { return m_ecat_sync_mode; }
    string GetTrainRate(void) { return m_train_rate; }

};

#endif //SAMPLE_TEST_APPS_ACONTISENVIRONMENT_H

