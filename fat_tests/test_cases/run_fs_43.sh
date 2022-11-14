#!/bin/bash

# This script is based on the automated test script Johana and Zuola wrote for the interlock timer test

mkdir results

# !!!Note: The dcf_dir directory from /media/engineering/SQA_test_setup/fs-43/dcf_lib directory should be
# copied to the local filesystem
if [ ! -d dcf_dir ];
  then echo "Please copy the dcf_dir from /media/engineering/SQA_test_setup/fs-43/dcf_lib to the local directory"
  exit 1;
fi

# Source the dcf library
source fs-43/dcf_lib.sh

ATE_TEST_DESCRIPTION="DCF Configuration test"
ATE_TEST_VERSION=0.0.1

TARGET_IPADDR=192.168.9.50
UUT=10-200482-00-0003
source dlog.sh

ATE_GPIO_DEV_POWER_24V=DAE0BXaW
MCM_CRC_REG=43d42a1c
ATE_GPIO_CHAN_POWER_24V=8

# Verifies register the CRC REG at 0x43d42A1C read 0x8000 (CRC success)
does_crc_pass() {
  ssh root@$TARGET_IPADDR "mem 43d42A1C -2 -v"
  return $?
}

power_ctrl() {
  for i in {1..5}; do
    sudo java -jar  /home/ddi/denkovi/DenkoviRelayCommandLineTool/DenkoviRelayCommandLineTool.jar $ATE_GPIO_DEV_POWER_24V 8 $ATE_GPIO_CHAN_POWER_24V $1
    if [ $? -eq 0 ]; then
      break;
    fi
    sleep 1;
  done
}

ate_power_on() {
  power_ctrl 0 
}

ate_power_off() {
  power_ctrl 1
}

get_timestamp() {
  printf '%(%F_%T)T'
}

ate_test_for_echo() {
  for i in {0..9}; do
    ping $TARGET_IPADDR -c1 -q
    if [ $? -eq 0 ]; then
      return 0;
    fi
  done
  return $?;
}


DCF_COUNT=0
ITERATION_COUNT=0
NUM_PASSED=0
NUM_FAILED=0


TIMESTAMP=`get_timestamp`
LOGFILE=results/ate_testlog_${UUT}_${TIMESTAMP}.txt

DLOG "$TIMESTAMP : --------------------------\n"
DLOG "Description: \"$ATE_TEST_DESCRIPTION\""
DLOG "Version: $ATE_TEST_VERSION Config: v$ATE_CONFIG_VERSION"
DLOG "Starting $UUT Test"

# Turn power on to the UUT
ate_power_on
sleep 25;

# Main loop
while [ 1 ]; do
  
  # Get and log the timestamp information to a file
  TIMESTAMP=`get_timestamp`
  printf "$TIMESTAMP : %s Iteration: %d : Passed: %d Failed: %d\n" $UUT $ITERATION_COUNT $NUM_PASSED $NUM_FAILED
  printf "$TIMESTAMP : %s Iteration: %d : Passed: %d Failed: %d\n" $UUT $ITERATION_COUNT $NUM_PASSED $NUM_FAILED >> $LOGFILE
 
  # Get the DCF file to program
  DCF=$(get_dcf_name $DCF_COUNT)
  echo "DCF name is:  $DCF"
  scp "dcf_dir/$DCF" root@$TARGET_IPADDR:/tmp/

  # Program the DCF file
  ssh root@192.168.9.50 -t "eeprom -p 0xeedd1 -kMCM -f /tmp/$DCF --raw --verify" 

  # Increment the DCF count and wrap when the DCF count is set to 0
  DCF_COUNT=$(($DCF_COUNT + 1))
  $(get_dcf_entry_max)
  DCF_MAX=$?
  if [ $DCF_COUNT == $DCF_MAX ] ; then
    DCF_COUNT=0
  fi
  
  # Turn off UUT 
  ate_power_off
  sleep 5;

  # Turn on UUT
  ate_power_on
  sleep 25;

  # Verify UUT is online
  ate_test_for_echo
  if [ $? -ne 0 ]; then
    DLOG "Error: LAN Communication Timeout: $UUT"
    DLOG "Exiting Test"
    exit 1
  fi

  # Check CRC Reg
  CRC_PASS=`does_crc_pass`
  printf "Check MCM CRC status: %d CRC_PASS : '%s'\n" $? $CRC_PASS >> $LOGFILE

  OKAY=1

  # 0x2A1C reading 0x8000 means the CRC has passed
  if [ "$CRC_PASS" == "0x8000" ]; then
    echo "CRC Test Passed";
  else
    echo "CRC Test Failed";
    ssh root@$TARGET_IPADDR 'beep 400 0;/mnt/data/mcm_diags_arm -e'
    OKAY=0
    exit
  fi

  if [ $OKAY == 1 ]; then
    echo "Test Passed";
    NUM_PASSED=$(($NUM_PASSED + 1))
  else
    echo "Test Failed";
    NUM_FAILED=$(($NUM_FAILED + 1))
  fi

  ITERATION_COUNT=$(($ITERATION_COUNT + 1))

done

