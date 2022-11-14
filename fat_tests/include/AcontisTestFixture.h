#ifndef ACONTIS_TEST_FIXTURE
#define ACONTIS_TEST_FIXTURE
#include <vector>
#include <string>
#include <iostream>
#define DEBUG // Enable the DLOG, VLOG and ELOG macros from ddi_debug.h in ddi_common_lib

#include "AcontisEnvironment.h"
#include "ddi_sdk_common.h"
#include "ddi_sdk_ecat_master.h"
#include "ddi_sdk_ecat_sdo.h"
#include "ddi_status.h"
#include "ddi_sdk_fusion_interface.h"
#include "gtest/gtest.h"
#include "ddi_ntime.h"
#include "CyclicData.h"
#include "DDITestCommon.h"

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

#define DEFAULT_ACONTIS_TIMEOUT 1000 // In milliseconds

#define SEND_BUFF_SIZE      512
#define RECV_BUFF_SIZE      512

class AcontisTestFixture: public ::testing::Test
{
  AcontisEnvironment*      m_AcontisEnvironment;
protected:
  virtual void SetUp() override;
public:
    AcontisTestFixture(void);
    ~AcontisTestFixture(void);

    ddi_status_t           m_test_result;

    /* pointer to the fusion instance */
    ddi_fusion_instance_t* fusion_instance;
    FILE*                  m_test_log_fd;

    // Go to OP mode and display version string when complete
    void         go_to_op_mode(const char *version);
    // Get serial number
    void         get_PN(char sn[]);
    // Wait for sec_to_wait seconds to expire.  Query the EtherCAT lost frames at poll_rate_ms
    void         poll_EtherCAT(uint32_t sec_to_wait, uint32_t poll_rate_ms);
    static void  hello_world_demo(void *arg);
    void         open_fusion(cyclic_func_t *callback);
    void         open_fusion(AcontisCallback *callback);
    void         close_fusion();
    int          set_pd_struct(void *in_struct, uint length);
    // Log support functions
    ddi_status_t log_open(const char *filename, const char *mode);
    ddi_status_t log_write(bool flush, const char *msg, ...);
    void log_close();
    // Set free-run or synchron mode using the 'mode' parameter
    void         set_sync_mode(ddi_fusion_instance_t *instance, ethercat_sync_mode_t mode);
    // Set free-run or synchron mode using the command line argument
    void         set_sync_mode(ddi_fusion_instance_t *instance);
    // Get free-run or asynchron mode
    string  get_sync_mode(ddi_fusion_instance_t *instance);
    double       quanta_to_volts(int16_t quanta);

    string       get_train_rate();
    // Log CSV header information
    void log_csv_header();
     // Return the keyboard input. Only the non-blocking version is implemented currently.
    ddi_status_t get_keyboard_input(char *key, bool blocking=false);

    int          connect_to_DAQ(void);
    void         collect_data_24(void);
    float        collect_data_1(int channel);


    AcontisCyclicData *cyclic_data;
};

/**
 * The Callback test is specialized version for the AcontisCallback.  It also includes a reference to an
 * AcontisTestFixture which can be referenced during the callback from Acontis in the cyclic_method.
 */
class CallbackTest: public AcontisCallback
{
  protected:
    AcontisTestFixture *m_AcontisTestFixture;
  public:
    CallbackTest (AcontisTestFixture *fixture) { m_AcontisTestFixture = fixture; }
    virtual void cyclic_method (void *) { return; } // By default do nothing.
};

#endif

