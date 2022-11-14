/**************************************************************************
(c) Copyright 2022 Digital Dynamics Inc. Scotts Valley CA USA.
Unpublished copyright. All rights reserved. Contains proprietary and
confidential trade secrets belonging to DDI. Disclosure or release without
prior written authorization of DDI is prohibited.
**************************************************************************/
#ifndef DDI_FUSION_UART_TEST_FIXTURE
#define DDI_FUSION_UART_TEST_FIXTURE
#include "DDIEMEnvironment.h"
#include "ddi_sdk_common.h"
#include "ddi_sdk_ecat_master.h"
#include "ddi_sdk_ecat_sdo.h"
#include "ddi_status.h"
#include "ddi_sdk_fusion_interface.h"
#include "ddi_em_fusion.h"
#include "gtest/gtest.h"
#include "ddi_ntime.h"
#include "CyclicData.h"
#include "DDITestCommon.h"
#include "ddi_em_api.h"
#include "DDIEMUtility.h"
#include "ddi_em_fusion_uart_api.h"

typedef struct {
  ddi_em_handle em_handle;     /**< DDI EtherCAT Master handle */
  ddi_em_slave_config *es_cfg; /**< Slave Configuration Paramters (pd_input_offset, pd_output_offset etc..) */
} uart_pd_callback_args;

class ddi_fusion_uart_test_fixture: public ::testing::Test
{
  DDIEMEnvironment*         m_DDIEMEnvironment;
  ddi_em_handle             em_handle;
  ddi_es_handle             es_handle;
  ddi_em_init_params        init_params;
  uint16_t                  tx_index_base;
  uint16_t                  rx_index_base;
  int                       config_index;
  uint16_t                  info_index;
  uart_channel              channel;
  uint8_t                   is_allocated; 
  DDIEMUtility              m_ddi_em_utility;
  ddi_em_slave_config       es_cfg;  
  ddi_fusion_uart_handle    uart_handle;

protected:
  virtual void SetUp()      override;
  virtual void TearDown()   override;
  ddi_em_result       m_test_result;

public:
  uart_baud baud_list[9]
  {     
    UART_BAUD_1200,
    UART_BAUD_2400,
    UART_BAUD_4800,
    UART_BAUD_9600,
    UART_BAUD_19200,
    UART_BAUD_31200,
    UART_BAUD_38400,
    UART_BAUD_57600,
    UART_BAUD_115200
  };
  uart_parity parity[3]
  {
    UART_PARITY_NONE,
    UART_PARITY_EVEN,
    UART_PARITY_ODD
  };
  uart_data_bits m_data_bits[2]
  {
  //  UART_DATA_BITS_5 ,
  //  UART_DATA_BITS_6,
    UART_DATA_BITS_7,
    UART_DATA_BITS_8    
  };
  uart_flow_control flow_control[3]
  {
    UART_FLOW_CONTROL_OFF,
    UART_FLOW_CONTROL_RTS_CTS,
    UART_FLOW_CONTROL_XON_XOFF
  };
  uart_stop_bits m_stop_bits[1]
  {
    UART_STOP_BITS_1,
  };

                          ddi_fusion_uart_test_fixture(void);

  void                    SetFixtureStatus(ddi_em_result result) { m_test_result = result; }
  ddi_em_result           GetFixtureStatus()                     { return m_test_result; }

  void                    SetInitParamsDefault();

  ddi_em_init_params*     GetInitParamsPointer()                 { return &init_params; }

  ddi_em_handle           GetEtherCATMasterHandle()              { return em_handle; }
  ddi_em_handle*          GetEtherCATMasterHandlePointer()       { return &em_handle; }

  ddi_es_handle           GetEtherCATSlaveHandle()               { return es_handle; }
  ddi_es_handle*          GetEtherCATSlaveHandlePointer()        { return &es_handle; }

  ddi_fusion_uart_handle  GetFusionUARTHandle()                  { return uart_handle; }
  ddi_fusion_uart_handle* GetFusionUARTHandlePointer()           { return &uart_handle; }

  ddi_em_slave_config*    GetEtherCATSlaveConfigPointer()        { return &es_cfg; }

  DDIEMEnvironment*       GetEnvironmentPointer()                { return m_DDIEMEnvironment; }

  ddi_em_interface_select FindNic();

  void                    perform_loopback(bool data_7);
};
#endif
