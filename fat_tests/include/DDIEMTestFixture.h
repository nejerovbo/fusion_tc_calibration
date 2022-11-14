/**************************************************************************
(c) Copyright 2021 Digital Dynamics Inc. Scotts Valley CA USA.
Unpublished copyright. All rights reserved. Contains proprietary and
confidential trade secrets belonging to DDI. Disclosure or release without
prior written authorization of DDI is prohibited.
**************************************************************************/
#ifndef DDI_EM_TEST_FIXTURE
#define DDI_EM_TEST_FIXTURE
#include <vector>
#include <string>
#include <iostream>
#define DEBUG // Enable the DLOG, VLOG and ELOG macros from ddi_debug.h in ddi_common_lib

#include "DDIEMEnvironment.h"
#include "ddi_sdk_common.h"
#include "ddi_sdk_ecat_master.h"
#include "ddi_sdk_ecat_sdo.h"
#include "ddi_status.h"
#include "ddi_sdk_fusion_interface.h"
#include "gtest/gtest.h"
#include "ddi_ntime.h"
#include "CyclicData.h"
#include "DDITestCommon.h"
#include "ddi_em_api.h"
#include "DDIEMUtility.h"

//max lost frames before program exit
#define MAX_LOST_FRAMES_BEFORE_EXIT 15

// EtherCAT Mode definitions
typedef enum {
  SYNC_MODE_FREE_RUN,
  SYNC_MODE_SM_SYNCHRON,
  SYNC_MODE_DC_SYNCHRON // One day right?
} ethercat_sync_mode_t;

// Sync managers are set to 0 for Free-run
#define FREE_RUN_SM_SETTING   0

// Settings for the SYNC MANAGER
#define SM_SYNCHRON_OUTPUT_SM_SETTING 0x22
#define SM_SYNCHRON_INPUT_SM_SETTING  1

// Settings for the input and output sync manager
#define SM_INPUT_SYNC_MANAGER  0x1C32
#define SM_OUTPUT_SYNC_MANAGER 0x1C33

#define DEFAULT_DDIEM_TIMEOUT 1000 // In milliseconds

#define SEND_BUFF_SIZE      512
#define RECV_BUFF_SIZE      512

class DDIEMTestFixture: public ::testing::Test
{
  DDIEMEnvironment*      m_DDIEMEnvironment;
  ddi_em_handle          em_handle;
  ddi_es_handle          es_handle;
  ddi_em_slave_config    es_cfg;
  ddi_em_init_params     init_params;
  DDIEMUtility           m_ddi_em_utility;
  ddi_em_master_stats    master_stats;

protected:
  virtual void SetUp()    override;
  virtual void TearDown() override;
  ddi_em_result           m_test_result;
public:
  DDIEMTestFixture(void)                                         { es_handle = 0; }
  ddi_em_master_stats*    GetMasterStatsPointer()                { return &master_stats; }

  void                    SetFixtureStatus(ddi_em_result result) { m_test_result = result; }
  ddi_em_result           GetFixtureStatus()                     { return m_test_result; }

  ddi_em_handle           GetEtherCATMasterHandle()              { return em_handle; }
  ddi_em_handle*          GetEtherCATMasterHandlePointer()       { return &em_handle; }

  ddi_es_handle           GetEtherCATSlaveHandle()               { return es_handle; }
  ddi_es_handle*          GetEtherCATSlaveHandlePointer()        { return &es_handle; }

  ddi_em_slave_config*    GetEtherCATSlaveConfigPointer()        { return &es_cfg; }

  ddi_em_init_params*     GetInitParamsPointer()                 { return &init_params; }

  DDIEMEnvironment*       GetEnvironmentPointer()                { return m_DDIEMEnvironment; }

  ddi_em_interface_select FindNic();
  ddi_em_result           configure();
  void                    SetInitParams();
};

#endif
