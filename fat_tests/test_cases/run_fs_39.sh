#!/bin/bash
# Creates run script for UART testing

for i in {0..3}
do
    # Run RS-232 loopback
    sudo ./build/src/bin/sample_test fusion_uart_232_loopback -n $(( 0x5085 )) -c $i -e config/8_uart_eni.xml -s 1000 -i i8254:2 -d 10000
done