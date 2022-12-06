#!/usr/bin/csh
# Creating Run Script
#

setenv LD_LIBRARY_PATH  /usr/local/MATLAB/R2022b/bin/glnxa64/:/usr/local/MATLAB/R2022b/sys/os/glnxa64/
echo $LD_LIBRARY_PATH
# ./build/src/bin/sample_test mtaio16 -e config/12-200677-01.xml -s 1000 -i i8254:4 -d 10000
./build/src/bin/sample_test mtaio8 -e config/12-200677-01.xml -s 1000 -i i8254:1 -d 10000
