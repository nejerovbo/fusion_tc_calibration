# Creating Run Script

export LD_LIBRARY_PATH=../lib/
# ./build/tests/bin/sample_test acontis -e config/CRAM_rim_event_status_11-11.xml -s 1000 -i i8254:2 -d 10000
./build/src/bin/sample_test aioaccuracy -e config/CRAM_rim_event_status_11-11.xml -s 2000 -i eno1 -d 10000
# ./build/src/bin/sample_test aiaccuracy -e config/CRAM_rim_event_status_11-11.xml -s 2000 -i eno1 -d 10000
# ./build/src/bin/sample_test aoaccuracy -e config/CRAM_rim_event_status_11-11.xml -s 2000 -i eno1 -d 10000
#./build/src/bin/sample_test aoutstep -e config/ib_cm_setup_v2.xml -s 1000 -i eno1 -d 10000 
# ./build/tests/bin/sample_test acontis -e config/mattson_eni.xml -s 10000 -i i8254:2 -d 10000 
# ./build/tests/bin/sample_test acontis -e config/ib_cm_setup_demo.xml -s 1000 -i i8254:2 -d 10000
