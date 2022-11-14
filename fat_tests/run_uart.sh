#!/bin/bash
# Creates run script for UART testing

SERIAL_ENI=$2

if [ $# -ne 3 ]
  then
   echo "usage: run_uart.sh <test_to_run> <eni_file> <nic>"
   exit 1
fi

while [ 1 ]
do
  for i in {0..3}
    do
      ./build/src/bin/sample_test $1 -n $(( 0x5005 )) -c $i -e $SERIAL_ENI -s 1000 -i $3 -d 10000
    done
done

