#!/bin/bash
# Creates run script for UART stress testing

./build/src/bin/sample_test fusion_uart_stress -s 1000 -i i8254:2 -d 10000


