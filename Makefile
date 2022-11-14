SPARAM := 2000

run-something:
	echo ddi-em-basic  -e config/cram_timestamp_eni.xml -s $(SPARAM) -i i8254:4 -d 10000

run-em-basic:
	./build/src/bin/sample_test ddi-em-basic  -e config/cram_timestamp_eni.xml -s $(SPARAM) -i i8254:4 -d 10000

run-asmi:
	./fat_tests/build/src/bin/ && sudo sample_test ASMI_IO -e config/1096_fusion_asmi_eni.xml -s 18000 -i i8254:4  -d 10000

init:
	cd fat_tests && conan profile update settings.compiler.libcxx=libstdc++11 default  # Sets libcxx to C++11 ABI
	cd fat_tests && conan install . -if build/ -pr lin64

clean:
	rm -rf ./fat_tests/build

build:
	cd fat_tests && cmake -B build/ -DDEBUG=1 -DCMAKE_BUILD_TYPE=DEBUG
	cd fat_tests && cmake --build build/

testdir:
	cd fat_tests && pwd
	cd fat_tests/build && pwd

#--------------------------------------------
# Z
SERIAL_ENI ?= ""
CHANNEL ?= 0
NIC ?= i8254:1
PARTIAL ?= 0

ddi-em-basic:
	./fat_tests/build/src/bin/sample_test ddi-em-basic             -n 20485 -c ${CHANNEL} -e ${SERIAL_ENI} -s 1000 -i ${NIC} -d 10000 -p ${PARTIAL}

ddi-em-partial:
	./fat_tests/build/src/bin/sample_test ddi-em-partial           -n 20485 -c ${CHANNEL} -e ${SERIAL_ENI} -s 1000 -i ${NIC} -d 10000 -p ${PARTIAL}

fusion-uart-param:
	./fat_tests/build/src/bin/sample_test fusion_uart_param        -n 20485 -c ${CHANNEL} -e ${SERIAL_ENI} -s 1000 -i ${NIC} -d 10000 

fusion-uart-all-params:
	./fat_tests/build/src/bin/sample_test fusion_uart_232_loopback -n 20485 -c ${CHANNEL} -e ${SERIAL_ENI} -s 1000 -i ${NIC} -d 10000

fusion-uart-232-loopback:
	./fat_tests/build/src/bin/sample_test fusion_uart_232_loopback -n 20485 -c ${CHANNEL} -e ${SERIAL_ENI} -s 1000 -i ${NIC} -d 10000

fusion-uart-485-loopback:
	./fat_tests/build/src/bin/sample_test fusion_uart_485_loopback -n 20485 -c ${CHANNEL} -e ${SERIAL_ENI} -s 1000 -i ${NIC} -d 10000

# Legacy tests 
acontis:
	./fat_tests/build/tests/bin/sample_test acontis     -e config/CRAM_rim_event_status_11-11.xml -s 1000 -i i8254:2 -d 10000

aioaccuracy:
	./fat_tests/build/src/bin/sample_test   aioaccuracy -e config/CRAM_rim_event_status_11-11.xml -s 2000 -i eno1 -d 10000

aiaccuracy:
	./fat_tests/build/src/bin/sample_test   aiaccuracy  -e config/CRAM_rim_event_status_11-11.xml -s 2000 -i eno1 -d 10000

aoaccuracy:
	./fat_tests/build/src/bin/sample_test   aoaccuracy  -e config/CRAM_rim_event_status_11-11.xml -s 2000 -i i8254:2 -d 10000

aoutstep:
	./fat_tests/build/src/bin/sample_test   aoutstep    -e config/ib_cm_setup_v2.xml -s 1000 -i eno1 -d 10000

UART-SDO:
	./fat_tests/build/src/bin/sample_test   UART_SDO    -e config/cram_serial_eni_v3.xml -s 1000 -i i8254:2 -d 10000

UART-Tenma:
	./fat_tests/build/src/bin/sample_test   UART_Tenma  -e config/cram_serial_eni_v3.xml -s 1000 -i i8254:2 -d 10000

UART-Loopback:
	./fat_tests/build/src/bin/sample_test   UART_Loopback -e config/cram_serial_eni_v3.xml -s 1000 -i i8254:2 -d 10000

dinlatency:
	./fat_tests/build/src/bin/sample_test   dinlatency  -e config/CRAM_rim_event_status_11-11.xml -s 2000 -i i8254:2 -d 10000

doutlatency:
	./fat_tests/build/src/bin/sample_test   doutlatency -e config/CRAM_rim_event_status_11-11.xml -s 1100 -i i8254:2 -d 10000

aifreqresponse-beckhoff:
	./fat_tests/build/src/bin/sample_test   aifreqresponse-beckhoff -e config/Beckhoff_Fusion_eni.xml -m sync -s 1000 i8254:2 -d 10000

ecattimestamp:
	./fat_tests/build/src/bin/sample_test   ecattimestamp -e config/CRAM_rim_event_status_11-11.xml -s 1000 -i i8254:2 -d 10000 --gtest_repeat=4

ainlatency-beckhoff:
	./fat_tests/build/src/bin/sample_test   ainlatency-beckhoff -e config/Beckhoff_Fusion_eni.xml -m free -s 1000 -i i8254:2 -d 10000

dinlatency-beckhoff:
	./fat_tests/build/src/bin/sample_test   dinlatency-beckhoff -e config/Beckhoff_Fusion_eni.xml -m free -s 2000 -i i8254:2 -d 10000

ainlatency:
	./fat_tests/build/src/bin/sample_test   ainlatency    -e config/CRAM_rim_event_status_11-11.xml -s 2000 -i i8254:2 -d 10000

	
help:
	@echo Available tests and their usage: 
	@echo \  ddi-em-basic -c 'CHANNEL' -e 'SERIAL_ENI' -i 'NIC' -p 'Partial_mode'
	@echo \  ddi-em-partial -c 'CHANNEL' -e 'SERIAL_ENI' -i 'NIC' -p 'Partial_mode'
	@echo \  fusion-uart-param -c 'CHANNEL' -e 'SERIAL_ENI' -i 'NIC'
	@echo \  fusion-uart-all-params -c 'CHANNEL' -e 'SERIAL_ENI' -i 'NIC'
	@echo \  fusion-uart-232-loopback -c 'CHANNEL' -e 'SERIAL_ENI' -i 'NIC'
	@echo \  fusion-uart-485-loopback -c 'CHANNEL' -e 'SERIAL_ENI' -i 'NIC'
	@echo For a list of legacy tests, type in 
	@echo \  make help-legacy

help-legacy:
	@echo Available legacy tests:
	@echo acontis
	@echo aioaccuracy
	@echo aiaccuracy
	@echo aoaccuracy
	@echo aoutstep
	@echo UART_SDO
	@echo UART_Tenma
	@echo UART_Loopback
	@echo dinlatency
	@echo doutlatency
	@echo aifreqresponse-beckhoff
	@echo ecattimestamp
	@echo ainlatency-beckhoff
	@echo dinlatency-beckhoff
	@echo ainlatency
