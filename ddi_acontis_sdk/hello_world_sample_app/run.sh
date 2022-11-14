#/usr/bin/bash

#this is a local version of the run.sh script meant to run out of the hello_world_sample_app
#directory

if [ -z ]; then
export ECAT_NIC=enp7s0
else
export ECAT_NIC=$1
fi

#-i network interface to attach to
#-i network interface to attach to
#note: use -i i8254:<instance_number> to talk to the optimized link layer
#common values for the instance_number are 0,1,2
#the sqa computer in the lab uses instance_number of 1 and 2 for the optimized link layer
#-e eni.xml to use for parsing
#-d process data output display rate in cyclic data cylces
#-s cyclic data rate in microseconds
export LD_LIBRARY_PATH=../../../lib/ddi_acontis_sdk_1.0
./build/hello_world_sample_app -e eni.xml -s 2000 -i $ECAT_NIC -d 1000
