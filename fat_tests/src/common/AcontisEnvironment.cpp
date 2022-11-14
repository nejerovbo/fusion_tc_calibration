//
// Created by ddodd on 8/26/2021.
//

#include <map>
#include <string>

#include "AcontisEnvironment.h"
#include "EnvironmentRegistry.h"
#include "yarger.h"

using namespace std; // We probably shouldn't use namespace std in a header, but...

AcontisEnvironment::AcontisEnvironment(std::map<string, ddi::Option*> options) {
    m_options = options;
    m_initialized = false;
}

// Does not appear to be getting called
void AcontisEnvironment::SetUp( ) {

    // Get the parameters from Yarger
    m_eni_file       = m_options["eni-file"]->get_value();
    m_train_rate     = m_options["train-rate"]->get_value();
    m_nic            = m_options["nic"]->get_value();
    m_display_rate   = m_options["display-rate"]->get_value();
    m_ecat_sync_mode = m_options["sync-mode"]->get_value();

    // Set the filter of which tests we are going to run
    ::testing::GTEST_FLAG(filter)=m_options["testfilter"]->get_value();

    m_initialized = true;

    // set the license file, not a fatal error and don't be too noisy about a failure
    ddi_sdk_ecat_set_license_file((char *)"./acontis_licenses.csv");

    // Initialize the SDK
    InitSDK();

    DLOG("AcontisEnvironment::SetParams rate: %d eni: %s nic: %s \n", stoi(m_train_rate), m_eni_file.c_str(), m_nic.c_str());
}


void AcontisEnvironment::InitSDK ()
{
  ddi_status_t result;
  if (!m_initialized)
  {
    ELOG("Acontis failed to initialize...\n");
    exit(-1);
  }

  // set the license file, not a fatal error and don't be too noisy about a failure
  ddi_sdk_ecat_set_license_file((char *)"./acontis_licenses.csv");

  result = ddi_sdk_init(stoi(m_train_rate), (char *)m_eni_file.c_str(), (char *)m_nic.c_str());
  // Initialize the SDK, Assert if there's a failure
  if ( result != false)
  {
    ELOG("DDI SDK failed to initialize... result %d \n", result);
    //exit(result);
  }

  DLOG("AcontisEnvironment::InitSDK success \n");
}

void AcontisEnvironment::TearDown() {}


