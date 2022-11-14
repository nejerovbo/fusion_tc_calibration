/**************************************************************************
(c) Copyright 2021 Digital Dynamics Inc. Scotts Valley CA USA.
Unpublished copyright. All rights reserved. Contains proprietary and
confidential trade secrets belonging to DDI. Disclosure or release without
prior written authorization of DDI is prohibited.
**************************************************************************/
#ifndef DDI_EM_UTILITY
#define DDI_EM_UTILITY
#include <vector>
#include <string>
#include <iostream>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <pthread.h>
#define DEBUG // Enable the DLOG, VLOG and ELOG macros from ddi_debug.h in ddi_common_lib
#include "gtest/gtest.h"
#include "ddi_sdk_common.h"
#include "ddi_sdk_ecat_master.h"
#include "ddi_sdk_ecat_sdo.h"
#include "ddi_status.h"
#include "ddi_sdk_fusion_interface.h"
#include "ddi_ntime.h"
#include "CyclicData.h"
#include "DDITestCommon.h"
#include "ddi_em_api.h"
#include "ddi_em_fusion.h"
#include "ddi_200143_00_rim_pd.h"

/*! @var TEST_DEFAULT_TIMEOUT
  @brief Value used as a default timeout for Acontis calls
*/
#define TEST_DEFAULT_TIMEOUT       10000
/*! @var DISPLAY_UPDATE_CYCLE_COUNT
  @brief Display rate in cyclic frames
*/
#define DISPLAY_UPDATE_CYCLE_COUNT 1000
#define PRINTF_GOTO_XY(x,y) do{ printf("\033[%d;%dH", y, x); printf("\33[2K"); } while(0)
// Global handle used by the thread manager
static ddi_em_handle          g_em_handle;
// Global copy of the input process data
static six_slot_pd_in_struct  g_pd_input;
// Global copy of the output process data
static six_slot_pd_out_struct g_pd_output;


uint32_t event_handler(ddi_em_event *event);
void process_data_callback(void *args);  
void *ddi_cyclic_thread(void * args);


typedef struct
{
  ddi_em_handle em_handle;     /**< DDI EtherCAT Master handle */
  ddi_em_slave_config *es_cfg; /**< Slave Configuration Paramters (pd_input_offset, pd_output_offset etc..) */
} pd_callback_args;

class DDIEMUtility
{
  pthread_t pthread_handle;
public:
  int start_thread_manager (uint32_t em_handle, pthread_t *pthread_handle);
  void handle_pd_output_update(pd_callback_args *callback_args);
  void handle_pd_input_update(pd_callback_args *ballback_args);
  pthread_t * GetThreadHandlePointer() { return &pthread_handle; }
  static void UART_cyclic_function (void *user_args);
};


#endif
