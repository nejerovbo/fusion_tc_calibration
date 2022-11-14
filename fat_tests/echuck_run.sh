# Creating Run Script

# SERIAL_ENI="config/mattson_1.09.6.xml"

# ./build/src/bin/sample_test ai16 -e config/CRAM_rim_event_status_11-11.xml -s 1000 -i i8254:1 -d 10000
# ./build/src/bin/sample_test tai16 -e config/CRAM_rim_event_status_11-11.xml -s 1000 -i i8254:1 -d 10000
# ./build/src/bin/sample_test ai8 -e config/CRAM_rim_event_status_11-11.xml -s 1000 -i i8254:1 -d 10000


# For running Echuck test
sudo rmmod e1000e
make -C /home/matt/code/github/atemsys
sudo insmod /home/matt/code/github/atemsys/atemsys.ko
./build/src/bin/sample_test EChuck_IO -e config/echuck-eni.xml -s 2500 -i i8254:1 -d 10000
