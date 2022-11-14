#!/bin/bash
# Creates run script for UART testing

SERIAL_ENI=$2
TARGET_IP=192.168.9.50

if [ $# -ne 3 ]
  then
   echo "usage: run_uart.sh <test_to_run> <eni_file> <nic>"
   exit 1
fi

while [ 1 ]
do
  for i in {0..3}
    do
     # Run RS-232 loopback
     sudo ./build/src/bin/sample_test $1 -n $(( 0x5005 )) -c $i -e $SERIAL_ENI -s 1000 -i $3 -d 10000
     # Restart RIM
     ssh root@$TARGET_IP spoke 2828 ffff -2
     sleep 1
     ssh root@$TARGET_IP spoke 2828 0 -2
     sleep 1
     ssh root@$TARGET_IP spoke 2822 ffff -2
     sleep 5
    done
done

