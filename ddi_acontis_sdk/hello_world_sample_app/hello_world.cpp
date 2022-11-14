#include "ddi_sdk_common.h"
#include "ddi_sdk_ecat_master.h"
#include "ddi_sdk_ecat_sdo.h"
#include "ddi_status.h"
#include "ddi_sdk_fusion_interface.h"

int is_opmode = 0;
int ddi_log_level = 3;

/* pointer the fusion instance */
ddi_fusion_instance_t *fusion_instance;

cmdline_args_t cmdline_args;

//max lost frames before program exit
#define MAX_LOST_FRAMES_BEFORE_EXIT 15 

//this funciton gets called every process data cycle
//currently it will lookback any detected out and aouts
void update_IO(void *arg)
{
  ddi_fusion_instance_t *fusion_instance;
  static uint32_t display_count=0;
  int count;
  uint16_t output_value;
  bool output_to_display=0;
  int newline_count=0;
  uint16_t input_value;
  uint16_t input_length;
  uint16_t current_state, requested_state;

  //check parameters
  if (!arg)
    return;

  //set the fusion interface from the argument to the cyclic data function
  fusion_instance = (ddi_fusion_instance_t *)arg;

  ddi_sdk_ecat_get_slave_state(fusion_instance->slave,&current_state,&requested_state);
  //don't process data unless the slave is in OP mode
  if ( current_state != DDI_ECAT_OP)
    return;

  display_count++;
  if( display_count > cmdline_args.display_update_rate )
  {
    output_to_display=1;
    display_count=0;
  }
  //send an pattern provided by ddi_sdk_common
  output_value = ddi_sdk_get_next_test_value();
  if (fusion_instance->aout_count)
  {
    //set the aout values according the test pattern
    for(count=0; count < fusion_instance->aout_count; count++)
      //fusion_instance->aouts[count] = output_value;
      ddi_sdk_fusion_set_aout(fusion_instance, count, output_value);
  }
  if (fusion_instance->ain_count)
  {
    //copy over ain values from process data
    if(output_to_display)
    {
      newline_count=0;
      DLOG(" ***** ANALOG IN ***** \n");
      for(count = 0; count < fusion_instance->ain_count; count++)
      {
        newline_count++;
        input_value = ddi_sdk_fusion_get_ain(fusion_instance, count);
        DLOG(" AIN [%04d]= 0x%04x", count, input_value);
        if(newline_count == 7)
        {
          newline_count = 0;
          DLOG("\n");
        }
      }
      DLOG("\n");
   }
  }
  if (fusion_instance->dout_count)
  {
    //send an pattern provided by ddi_sdk_common
    for(count=0; count < fusion_instance->dout_count; count++)
    {
      //dont toggle relays if this happens to be a lowside unit (DOUT modules 2 and 3)
      //if one toggles relay there will be a horrible sound that is caused by the relays
      //switching too fast.

      //uncomment these lines if you want digital output
      //if( ( count == 2 ) || ( count == 3 ) )
      //  ddi_sdk_fusion_set_dout8(fusion_instance, count, 0xFF);
      //else
      //  ddi_sdk_fusion_set_dout16(fusion_instance, count, output_value);
    }
  }
  if (fusion_instance->din_count)
  {
    if(output_to_display)
    {
      newline_count = 0;
      DLOG(" ***** DIGITAL IN ***** \n");
      for(count = 0; count < fusion_instance->din_count; count++)
      {
        newline_count++;
        input_value = ddi_sdk_fusion_get_din(fusion_instance,count,&input_length);
        DLOG(" DIN [%04d]= 0x%04x", count, input_value);
        if(newline_count == 7)
        {
          newline_count = 0;
          DLOG("\n");
        }
      }
      DLOG("\n");
    }
  }
}

//define custom initialization here
int custom_init(void)
{
 /* TBD */
  return 0;
}

//entry point for hello world
int main(int argc, char *argv[])
{
  char msg_buffer[1024];
  uint32_t msg_length;
  uint32_t *module_ptr;
  uint32_t number_of_detected_modules;
  uint32_t count;
  ddi_status_t result;
  ddi_ecat_master_stats_t ecat_master_stats;

  //process standard EtherCAT command line arguments
  //this will parse the command line for EtherCAT startup arguments
  //disable this function if you want to control the arguments directly
  if(ddi_sdk_handle_cmdline(argc, argv, &cmdline_args) != ddi_status_ok)
  {
    ELOG("ddi_sdk_init error with comamnd line\n");
    exit(-2);
  }

  //set the license file, not a fatal error and don't be too noisy about a failure
  ddi_sdk_ecat_set_license_file((char *)"./acontis_licenses.csv");

  //initialize SDK
  result = ddi_sdk_init(cmdline_args.scan_rate_us,cmdline_args.eni_file,cmdline_args.network_interface);
  if ( result != ddi_status_ok)
  {
    ELOG("ddi_sdk_init error \n");
    exit(-1);
  }

  //open Fusion interface, register process data callback
  //TODO: add station address to this function to support multiple instances
  if (ddi_sdk_fusion_open(&fusion_instance, (cyclic_func_t*)update_IO) != ddi_status_ok)
  {
    ELOG(" sdk fusion open failuire \n");
    exit(-1);
  }

  if( fusion_instance == NULL)
  {
    ELOG("Returned Fusion interface is NULL, exiting\n");
    exit(-1);
  }

  /* ethercat will be in INIT state , add custom startup before going to OP */
  custom_init();

  //put the Fusion into PREOP
  result = ddi_sdk_ecat_set_master_state( DDI_ECAT_PREOP );
  if ( result != ddi_status_ok )
  {
    ddi_sdk_log_error("error setting ethercat master state = 0x%x \n", result);
    exit(0);
  }

  //get the SW version
  result = ddi_sdk_ecat_sdo_msg(fusion_instance->slave, DDI_COE_GET_SW_VERSION, msg_buffer, &msg_length);
  if ( result != ddi_status_ok)
  {
    ELOG("ddi_sdk_ecat_sdo_msg error \n");
  }
  DLOG("Fusion version %s \n", msg_buffer);

  //get the number of detected modules
  result = ddi_sdk_ecat_sdo_msg(fusion_instance->slave, DDI_COE_GET_NUMBER_OF_DETECTED_MODULES, msg_buffer, &msg_length);
  if ( result != ddi_status_ok)
  {
    ELOG("number of detected modules sdo error \n");
  }
  number_of_detected_modules = msg_buffer[0];
  DLOG(" number_of_detected_modules 0x%x \n", number_of_detected_modules);

  //clear msg buffer
  memset(msg_buffer,0,sizeof(msg_buffer));
  //get the actual detected modules
  result = ddi_sdk_ecat_sdo_msg(fusion_instance->slave, DDI_COE_GET_DETECTED_MODULES, msg_buffer, &msg_length);
  if ( result != ddi_status_ok)
  {
    ELOG("detected modules sdo error \n");
  }
  DLOG("\\nn ******* Start Detected Modules ******* \n");
  module_ptr = (uint32_t *)msg_buffer;
  for(count = 0; count < number_of_detected_modules; count++)
  {
    DLOG(" detected_module slot[0x%x] = 0x%x \n", count, module_ptr[count]);
    module_ptr++;
  }
  DLOG("******* End Detected Modules ******* \n\n");

  //go to OP mode
  result = ddi_sdk_ecat_set_master_state( DDI_ECAT_OP );
  if ( result != ddi_status_ok )
  {
    ddi_sdk_log_error("error setting ethercat master state = 0x%x \n", result);
    exit(0);
  }

  DLOG(" Fusion is in OP mode, but process data will only display every 10000 trains by defaut \n \
         Use the -d argument to control how often process data displays \n \
         DOUT toggling is disabled by default as it can cause relays to toggle too fast if they are present  \n\n" );

  //endless loop, just process cyclic data
  while(1)
  {
    //if the amount of lost frames exceeds 15, then exit the program
    result = ddi_sdk_ecat_get_master_stats(&ecat_master_stats);
    if ( result != ddi_status_ok )
    {
      ddi_sdk_log_error("error getting master state = 0x%x \n", result);
      ELOG("ddi_sdk_ecat_sdo_msg error \n");
      exit(-1);
    }
    if( ecat_master_stats.lost_frame_count >= MAX_LOST_FRAMES_BEFORE_EXIT )
    {
      ddi_sdk_log_error("lost frames exceeded max value of %d\n", MAX_LOST_FRAMES_BEFORE_EXIT);
      ELOG("lost frames exceeded max value of %d \n", MAX_LOST_FRAMES_BEFORE_EXIT);
      exit(-1);
    }
  }
}
