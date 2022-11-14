/******************************************************************************
 * (c) Copyright 2020-2021 Digital Dynamics Inc. Scotts Valley CA USA.
 * Unpublished copyright. All rights reserved. Contains proprietary and
 * confidential trade secrets belonging to DDI. Disclosure or release without
 * prior written authorization of DDI is prohibited.
 *****************************************************************************/
//DDI test common - all common test routines should go here.

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "ddi_sdk_common.h"
#include "ddi_sdk_display.h"
#include <time.h>
#include <signal.h>
#include <AtEthercat.h>
#include "ddi_macros.h"
#include "ddi_status.h"
#include "ddi_sdk_processing.h"
#include "ddi_sdk_ecat_master.h"
#include <ctype.h>

/* logging file pointer */
static FILE *log_fp = NULL;
/* statistics file pointer */
static FILE *stats_fp = NULL;
// is the fusion in op-mode?
extern bool is_opmode;
//lost frame from acontis
int lost_frame;
//kill next cycle
extern int kill_delay;
//skip frames (currently triggered by lost_frame)
int skip_frames;

int ddi_log_level=3;

//command line arguments
cmdline_args_t cmdline_args;

uint32_t usage(void)
{
  DLOG(" usage: <app_name> -e eni_file -s refresh_rate in microseconds -i network interface to use [ -d display rate in cyclic frames ] \n");
  return 0;
}

//get the next value to send out
int ddi_sdk_get_next_test_value (void)
{
    static int16_t output_pattern =0;
    static int fixed_pattern_delay = 0;
    static int fixed_pattern_count = 0;
    /* generate a fixed pattern on startup */
    fixed_pattern_delay++;
    if (fixed_pattern_delay == 1000)
    {
      fixed_pattern_delay = 0;
      fixed_pattern_count++;
    }
    //fixed startup pattern
    else if ( fixed_pattern_count == 1 )
    {
      output_pattern = 0;
    }
    else if ( fixed_pattern_count == 2 )
    {
      output_pattern = 1;
    }
    else if ( fixed_pattern_count == 2 )
    {
      output_pattern = 0x1fff;
    }
    else if ( fixed_pattern_count == 3 )
    {
      output_pattern = 0x7fff;
    }
    else if ( fixed_pattern_count == 4 )
    {
      output_pattern = 0x8001;
    }
    else if ( fixed_pattern_count == 5 )
    {
      output_pattern = 0x0;
    }
    else if (fixed_pattern_count >= 5 ) {
        //incremental pattern,not true rng but good enough?
        output_pattern += 0x03;
        //we dont really need to wrap around at 0x8000 as the compare handles it
        if (output_pattern & 0x8000) {
          fixed_pattern_count = 0;
        }
    }
    return output_pattern;
}

void ddi_sdk_control_c_handler(int signal) 
{
  ddi_status_t result;
  //shut down any comparisons
  is_opmode = 0;
  //set master state to INIT
  result = ddi_sdk_ecat_set_master_state( DDI_ECAT_INIT );
  if ( result != ddi_status_ok )
  {
    ddi_sdk_log_error("error setting ethercat master state = 0x%x \n", result);
  }
  //close file handles
  if (log_fp)
    fclose(log_fp);
  if (stats_fp)
    fclose(stats_fp);
  exit(EXIT_SUCCESS);
}

//used to log data for analysis
ddi_status_t ddi_sdk_log_data(const char *message, ...)
{
  if (message == NULL)
    return ddi_status_param_err;
  //log the variable arguments to file and console
  va_list args;
  va_start( args, message );
  vfprintf( stats_fp, message, args );
  va_end( args );
  fflush(log_fp);
  return ddi_status_ok;
}

ddi_status_t ddi_sdk_log_error(const char *message, ...)
{
  if (message == NULL)
    return ddi_status_param_err;
  //log the variable arguments to file and console
  va_list args;
  va_start( args, message );
  vfprintf( log_fp, message, args );
  va_end( args );
  fflush(log_fp);
  return ddi_status_ok;
}

ddi_status_t ddi_sdk_init(uint32_t scan_rate_us, char *eni_filename, char *network_interface)
{
  ddi_status_t result;
  slave_t slave;
  time_t curtime;
  time(&curtime);
  //register for the control+c signal
  signal(SIGINT, ddi_sdk_control_c_handler);
  log_fp = fopen("iolog.txt", "a+"); // Open for appending
  if(!log_fp) 
  {
    printf(" log file open error \n");
    return ddi_status_err;
  } 
  stats_fp = fopen("stats.txt", "a+"); // Open for appending
  if(!stats_fp) 
  {
    printf(" stats file open error \n");
    return ddi_status_err;
  } 
  result = ddi_sdk_ecat_init(scan_rate_us , eni_filename, network_interface);
  if ( result != ddi_status_ok )
  {
    ddi_sdk_log_error("error initializing ethercat stack res = 0x%x \n", result);
    return ddi_status_err;
  }
  result = ddi_sdk_ecat_set_master_state( DDI_ECAT_INIT );
  if ( result != ddi_status_ok )
  {
    ddi_sdk_log_error("error setting ethercat master state = 0x%x \n", result);
    return ddi_status_err;
  }
  return ddi_status_ok;
}

/* DDI cleanup entry point, will be called when acontis exits cleanly */
void ddi_sdk_exit(void)
{
  if (log_fp)
    fclose(log_fp);
  if (stats_fp)
    fclose(stats_fp);
  //stop analysis
  is_opmode =0;
  exit(EXIT_SUCCESS);
}

ddi_status_t ddi_sdk_get_cmdline_args (cmdline_args_t *args)
{
  memcpy(args,&cmdline_args,sizeof(cmdline_args_t));
  return ddi_status_ok;
}

/* handle command line arguments */
ddi_status_t ddi_sdk_handle_cmdline (int argc, char **argv, cmdline_args_t *args)
{
  uint32_t scan_rate = 0, instance_value;
  uint32_t display_update_rate = 0;
  uint32_t io_display_update_rate = 0;
  char *io_logfile_name = NULL;
  char *eni_value = NULL;
  char *iterface_value = NULL;
  char *instance;
  char *coe_od_logfile_name = NULL;
  int index;
  int c;
  uint8_t eni_set = 0, iface_set = 0, scan_set = 0, display_rate_set = 0, optimized_set = 0;
  uint8_t file_logging_set = 0, io_display_rate_set = 0, coe_od_logfile_set = 0;

  //validate paramters
  if( !argc || !argv || !args )
    return ddi_status_err;

  memset(&cmdline_args,0,sizeof(cmdline_args_t));

  opterr = 0;

  while ((c = getopt (argc, argv, "i:e:s:d:f:o:c:")) != -1)
  {
    switch (c)
    {
      //bus scan rate
      case 's':
        scan_set = 1;
        scan_rate = atoi(optarg);
        break;
      //eni file
      case 'e':
        eni_set = 1;
        eni_value = optarg;
        break;
      //network interface
      case 'i':
        iface_set = 1;
        iterface_value = optarg;
        //optimized link layer
        if(!strncmp(optarg,"i8254",5))
        {
          optimized_set = 1;
          //extract the instance after the : argument
          instance=strchr(optarg,':');
          //convert from character to numeric
          instance_value = instance[1] - 0x30;
        }
        break;
      //display rate
      case 'd':
        display_rate_set = 1;
        display_update_rate = atoi(optarg);
        break;
      //8khz display rate
      case 'o':
        io_display_rate_set = 1;
        io_display_update_rate = atoi(optarg);
        break;
      //io logfile name
      case 'f':
        file_logging_set = 1;
        io_logfile_name = optarg;
        break;
      case 'c':
        coe_od_logfile_set = 1;
        coe_od_logfile_name = optarg;
        break;
      case '?':
        if (optopt == 'i')
        {
          ELOG ("Option -%c requires an argument.\n", optopt);
        }
        else if (optopt == 'e')
        {
          ELOG( "Option -%c requires an argument.\n", optopt);
        }
        else if (optopt == 's')
        {
          ELOG( "Option -%c requires an argument.\n", optopt);
        }
        else if (optopt == 'd')
        {
          ELOG( "Option -%c requires an argument.\n", optopt);
        }
        else if (optopt == 'f')
        {
          ELOG( "Option -%c requires an argument.\n", optopt);
        }
        else if (optopt == 'c')
        {
          ELOG( "Option -%c requires an argument.\n", optopt);
        }
        else if (isprint (optopt))
          ELOG( "Unknown option `-%i'.\n", optopt);
        else
          ELOG(
                   "Unknown option character `\\x%x'.\n",
                   optopt);
      default:
        break;
    }
  }
  //check for required arguments
  if ( !(scan_set && iface_set && eni_set) )
  {
    usage();
    exit(EXIT_FAILURE);
  }

  if (!display_rate_set)
  {
    display_update_rate = 10000; //default value for display update
  }

  DLOG ("scan_rate = %d, eni_value = %s, iterface_value = %s\n",
          scan_rate, eni_value, iterface_value);

  for (index = optind; index < argc; index++)
    printf ("Non-option argument %s\n", argv[index]);

  //copy over to local structure for use in SDK functions
  cmdline_args.scan_rate_us = scan_rate;
  cmdline_args.display_update_rate = display_update_rate;
  cmdline_args.io_logfile_name = io_logfile_name;
  cmdline_args.io_display_update_rate = io_display_update_rate;
  memcpy(cmdline_args.eni_file, eni_value, strnlen(eni_value,256));
  memcpy(cmdline_args.network_interface, iterface_value, strnlen(iterface_value,256));
  //setup the optimized arguments
  if (optimized_set)
  {
    cmdline_args.optimized_set = 1;
    cmdline_args.optimized_instance = instance_value;
  }
  //coe object dictionary log file
  if (coe_od_logfile_set)
  {
    cmdline_args.coe_od_logfile = coe_od_logfile_name;
  }

  //set return parameters
  memcpy(args,&cmdline_args,sizeof(cmdline_args_t));

  DLOG(" eni %s \n", cmdline_args.eni_file);
  DLOG(" iface %s \n", cmdline_args.network_interface);
  DLOG(" scan rate %d \n", cmdline_args.scan_rate_us);
  DLOG(" display update rate %d \n", cmdline_args.display_update_rate);

  return ddi_status_ok;
}
