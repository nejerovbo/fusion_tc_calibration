#!/bin/bash
# Execute the UART loopback test

# Run script for the UART event test-case https://ddieng.atlassian.net/browse/FS-46

# Currently requires a CM with 8 UART slot-cards

SERIAL_ENI=config/8_uart_eni.xml

DATESTR='date +"%F_%T.%3N"'

# mkdir -p results/

for module in {0..0}
  do
    echo $module
    index=$(( 0x5085 + $((module))*0x10 ))
    echo $index
    for i in {0..3}
      do
        ./build/src/bin/sample_test fusion_uart_232_loopback_event -n $index -c $i -e $SERIAL_ENI -s 10000 -i i8254:1 -d 10000 
      done
   done
