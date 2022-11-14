#*****************************************************************************
# * (c) Copyright 2021-2022 Digital Dynamics Inc. Scotts Valley CA USA.
# * Unpublished copyright. All rights reserved. Contains proprietary and
# * confidential trade secrets belonging to DDI. Disclosure or release without
# * prior written authorization of DDI is prohibited.
# *****************************************************************************/

# This file provides general control of a ESPEC BTU-433
# This script will control the chamber from LOW_SETPOINT to HIGH_SETPOINT in 5 degree
# increments. This script will execute ./run.sh after waiting DWELL_TIME_SEC once
# the chamber has reached the target setpoint

from chamberconnectlibrary.watlowf4t import WatlowF4T
import time
import datetime
import struct
import os
import random
import sys
import argparse
# import pyodbc
from subprocess import Popen, PIPE
from datetime import datetime

# This file is used to control the power cycling of the Fusion.IO over temperature
# < -------------------------------------------------<1   Cycle>--------------------------------------------------------------->
#                                                                           15 minutes     10 seconds  20 seconds  20 seconds
# 55C                                                                   ---<I/O test runs>-<power off>-<power on> <I/O test runs>
#                                                                      /
#                                                                     /
# Ambient ----                                                       / 
#             \                                                     /
#              \                                                   /
# 0C            ---<I/O test runs>-<power off>-<power on> <I/O test runs>
#                   15 minutes     10 seconds  20 seconds  20 seconds
VERSION="v1.0.3"
PROJECT=" Temp Control Cycling Utility "

os.environ['LD_LIBRARY_PATH'] = "../lib"

FUSION_CHECK_IO="../bin/CRAM_reliability"

DC_POWEROFF_COMMAND1="java -jar /home/ddi/denkovi/DenkoviRelayCommandLineTool_291.jar DAE0BXaW 8 1 0 > /dev/null"
DC_POWEROFF_COMMAND2="java -jar /home/ddi/denkovi/DenkoviRelayCommandLineTool_291.jar DAE069Ta 8 8 1 > /dev/null;"
DC_POWERON_COMMAND1="java -jar /home/ddi/denkovi/DenkoviRelayCommandLineTool_291.jar DAE0BXaW 8 1 1 > /dev/null;"
DC_POWERON_COMMAND2="java -jar /home/ddi/denkovi/DenkoviRelayCommandLineTool_291.jar DAE069Ta 8 8 0 > /dev/null;"

FUSION_POWEROFF_DELAY=10
FUSION_POWERON_DELAY=25

interface_params = {'interface':'TCP', 'host':'192.168.9.10'}

setpoint_compare_band = 0.3 # Used to compare process value against the setpoint

tc_tol=20 # Thermocouple tolerance

HIGH_SETPOINT=60.0
LOW_SETPOINT=20.0
DWELL_TIME_SEC=(20 * 60) # 20 minutes
PC_TIME_SEC=15 # I/O test length after power cycle

now = datetime.now()

print (interface_params)

##########################################################################################
# Setup the chamber controller
##########################################################################################
controller = WatlowF4T(
    alarms=8, # the number of available alarms
    profiles=True, # the controller has programming
    loops=1, # the number of control loops (ie temperature)
    cond_event=9, # the event that enables/disables conditioning
    cond_event_toggle=False, # is the condition momentary(False), or maintained(True)
    run_module=1, # The io module that has the chamber run output
    run_io=1, # The run output on the mdoule that has the chamber run out put
    limits=[5], # A list of modules that contain limit type cards.
    **interface_params
)

def generate_hw_id():
  cnxn = pyodbc.connect("DSN=CRAMDB;UID=fusion;PWD=Fusion.IO")
  cursor = cnxn.cursor()
  #cursor.execute("select * from hw_config")
  cursor.execute("INSERT INTO hw_config OUTPUT Inserted.hw_config_id VALUES (DEFAULT, DEFAULT)")
  query = cursor.fetchall()
  for row in query:
    hw_config_id = row.hw_config_id
    cnxn.commit()
    return hw_config_id

##########################################################################################
# Generate a test result ID by inserting a hw_config table followed by a test_result table
##########################################################################################
def log_test_report(hw_config_id,result, notes ):
  version=VERSION
  cnxn = pyodbc.connect("DSN=CRAMDB;UID=fusion;PWD=Fusion.IO")
  cursor = cnxn.cursor()
  #cursor.execute("select * from hw_config")
  query = "INSERT INTO test_result OUTPUT Inserted.test_result_id VALUES (DEFAULT, DEFAULT, 'CRAM Power Cycling Test', '" + version + "','" + str(hw_config_id) + "', '" +result + "', '" + notes + "')"
  print("query = " + query)
  cursor.execute(query)
  test_results = cursor.fetchall()
  for row2 in test_results:
    print("test results = " + row2.test_result_id)
  cnxn.commit()
  return row2.test_result_id
  

##########################################################################################
# Spawn two fusion CRAM reliability tests threads
##########################################################################################
def spawn_io_compare (setpoint, time_in_seconds):
  print("*** Starting I/O compare for " + str(time_in_seconds) + " seconds")
  proc_list = [ 0, 0 ]
  cmd1 = ['../bin/CRAM_reliability','--exec-period-seconds',str(time_in_seconds),'-i','i8254:1','-e','../eni/CRAM_rim_event_status_11-11.xml','-s','1000','--tc-upper-bound',str(setpoint+tc_tol),'--tc-lower-bound', str(setpoint-tc_tol)]
  cmd2 = ['../bin/CRAM_reliability','--exec-period-seconds',str(time_in_seconds),'-i','i8254:2','-e','../eni/CRAM_rim_event_status_11-11.xml','-s','1000','--tc-upper-bound',str(setpoint+tc_tol),'--tc-lower-bound', str(setpoint-tc_tol)]
  
  proc_list[0] = Popen(cmd1, stdout=PIPE, stderr=PIPE)
  time.sleep(3)
  proc_list[1] = Popen(cmd2, stdout=PIPE, stderr=PIPE)
  
  time.sleep(time_in_seconds+15) # Give time for test to run, then check the output return code
  for proc in proc_list:
    #proc.wait()
    output, errors = proc.communicate()
    if (proc.returncode != 0 ):
      print ("Error executing CRAM test")
      return 1
  return 0

##########################################################################################
# Cycle Fusion.IO power
##########################################################################################
def cycle_fusion_power ():
  os.system(DC_POWEROFF_COMMAND1)
  os.system(DC_POWEROFF_COMMAND2)
  time.sleep(10)
  os.system(DC_POWERON_COMMAND1)
  os.system(DC_POWERON_COMMAND2)
  # Wait for Power-on delay (Fusion bootup time)
  time.sleep(FUSION_POWERON_DELAY)



##########################################################################################
# Wait for the chamber process value to reach the setpoint
##########################################################################################
def wait_for_pv (setpoint):
  timeout = 0
  process_value = 100000
  while (timeout < 10000):
    temp_dict = controller.get_loop_pv(1)
    process_value = temp_dict['air']
    # Compare against the setpoint band
    if ( (process_value > ( setpoint - setpoint_compare_band )) & (process_value < (setpoint + setpoint_compare_band))):
      # process value matched setpoint
      return process_value
    for key in temp_dict:
      print "Temperature is " + str(temp_dict['air'])
    timeout=timeout+1
    time.sleep(10) # Sleep 10 seconds
    if (timeout == 1000000):
      print("Timeout hit \n")
      return 1


setpoint_values = [ 50.0, 20.0, 45.0, 40.0, 35.0, 30.0, 25.0, 0]

##########################################################################################
# Power cycle between 0 and 55 C
##########################################################################################
def main ():
  # parser = argparse.ArgumentParser("CRAM power cycling utility")
  setpoint_count = 0
  test_execution_time = str(60 * 15)
  # hw_config_id = generate_hw_id()
  rng = random.SystemRandom()

  time.sleep(5)
  print("****************************************");
  print(PROJECT + VERSION)

  

  while setpoint_values >= 20: 
    setpoint = setpoint_values[setpoint_count]
    if ( setpoint == 0 ):
      break; #Exit
    controller.const_start()
    print("****************************************");
    print("Waiting for PV degrees to be within " + str(setpoint) + " +/- .1C");
    controller.set_loop_sp(1,setpoint)
    wait_for_pv(setpoint)

    print("Sleeping for " + str(DWELL_TIME_SEC))
    # Sleep for the dwell time
    time.sleep(DWELL_TIME_SEC)

    os.system("./run.sh")

    dir = "aio_8_9.25.22_data/setpoint" + str(setpoint) + "_ch8_" + now.strftime("%d_%m_%Y_%H_%M_%S")
    print(dir)
    os.system("mkdir -p " + dir)
    os.system("cp *.csv " + dir)

    setpoint += 5
    setpoint_count += 1
  print("Restoring chamber to 25.0 C")
  controller.set_loop_sp(1,25.0)

# Set chamber to 25 on startup
print("Restoring chamber to 25.0 C")
controller.set_loop_sp(1, 37.5)
wait_for_pv(37.5)
print("done");

main()
    
