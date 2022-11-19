project(FatTest)
cmake_minimum_required(VERSION 3.14)

file(GLOB SRCS
    Echuck/Echuck_Test.cpp
    ECATMaster/ddi_em_uart_test.cpp
    ECATMaster/ddi_em_uart_stress_test.cpp
    ECATMaster/ddi_em_uart_xfer.cpp
    ECATMaster/BasicECATMaster.cpp
    ECATMaster/DDIEMUtility.cpp
    ECATMaster/E84_loopback.cpp
    ECATMaster/EChuck.cpp
    DVT/AOOutputAccuracy.cpp
    DVT/AODCCoupling.cpp
    DVT/AOThermal.cpp
    DVT/AnalogInputLatency.cpp
    DVT/AnalogInputStepresponse.cpp
    DVT/AnalogOutputLatency.cpp
    DVT/DigitalInputLatency.cpp
    DVT/DigitalInputSwitchingThreshold.cpp
    DVT/DigitalOutputLatency.cpp
    DVT/DVTCommands.cpp
    DVT/AIOCombinedAccuracy.cpp
    DVT/EtherCATDataTimeStamp.cpp
    DVT/AnalogInputFreqResponse.cpp
    Calibration/ADCChip.cpp
    Calibration/DACChip.cpp
    Calibration/AIOCal.cpp
    Calibration/IOChannel.cpp
    Calibration/Utils/DAQUtil.cpp
    Calibration/Utils/telnet_open.c
    Calibration/Utils/indsock.c
    Calibration/Utils/DMMUtil.cpp
    Calibration/Utils/DCUtil.cpp
    Calibration/CalibrationCommon.cpp
    Calibration/CalibrationVerification.cpp
    Calibration/TCMeasure.cpp
    Calibration/TCVerify.cpp
    Calibration/TCCard.cpp
    MiniRIM/MiniRIM.cpp
    InterlockBridge/InterlockBridge.cpp
    UART/UARTSDOTest.cpp
    ASMI/ASMI.cpp
    common/ddi_fusion_uart_test_fixture.cpp
    common/AcontisTestFixture.cpp
    common/AcontisEnvironment.cpp
    common/DDIEMEnvironment.cpp
    common/DDIEMTestFixture.cpp
    common/CyclicData.cpp
    common/EnvironmentProvider.cpp
    common/EnvironmentRegistry.cpp
    common/mockmain.cpp
    common/AcontisEnvironment.h
    )
set(CMAKE_CXX_STANDARD 11)
set(DEBUG 1)


file(GLOB SDK_SRCS ../../ddi_acontis_sdk/src/*.cpp)
include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

ADD_EXECUTABLE(sample_test ${SRCS})

# Build the ddi_em SDK
file(GLOB DDI_EM_SRCS ../../ddi_ecat_master_lib/src/*.cpp ../../ddi_ecat_master_lib/src/fusion_sdk/*.cpp)

add_library(ddi_em_sdk SHARED ${DDI_EM_SRCS} ../../ddi_common_lib/ddi_ntime.c ../../ddi_common_lib/ddi_os_posix.c)
target_include_directories(ddi_em_sdk PUBLIC ../../ddi_common_lib/)
target_include_directories(ddi_em_sdk PUBLIC ../../acontis_lib/SDK/INC/)
target_include_directories(ddi_em_sdk PUBLIC ../../acontis_lib/SDK/INC/Linux/)
target_include_directories(ddi_em_sdk PUBLIC ${FATParent_SOURCE_DIR}/../ddi_ecat_master_lib/src/)
target_include_directories(ddi_em_sdk PUBLIC ${FATParent_SOURCE_DIR}/../ddi_ecat_master_lib/include/)
target_include_directories(ddi_em_sdk PUBLIC ${FATParent_SOURCE_DIR}/../ddi_ecat_master_lib/src/fusion_sdk/)
target_link_libraries(ddi_em_sdk ${CMAKE_SOURCE_DIR}/../acontis_lib/SDK/LIB/Linux/x64/libEcMaster.a ${CMAKE_SOURCE_DIR}/../acontis_lib/SDK/LIB/Linux/x64/libAtemRasSrv.a ${CMAKE_SOURCE_DIR}/../acontis_lib/Bin/Linux/x64/libemllI8254x.so)

# To fix the unknown linker error
set_target_properties(ddi_em_sdk PROPERTIES LINKER_LANGUAGE CXX)

add_library(
ddi_acontis_sdk SHARED ${SDK_SRCS} ../../ddi_common_lib/ddi_ntime.c ../../ddi_common_lib/ddi_os_posix.c ../../ddi_common_lib/ddi_queue.c)

link_directories(/usr/local/MATLAB/R2022b/extern/bin/glnxa64/)
find_library(MATLAB_ENGINE MatlabEngine HINTS /usr/local/MATLAB/R2022b/extern/bin/glnxa64/)
find_library(MATLAB_DATA_ARRAY MatlabDataArray HINTS /usr/local/MATLAB/R2022b/extern/bin/glnxa64/)

target_link_libraries(ddi_acontis_sdk ${CMAKE_SOURCE_DIR}/../acontis_lib/SDK/LIB/Linux/x64/libEcMaster.a ${CMAKE_SOURCE_DIR}/../acontis_lib/Bin/Linux/x64/libemllI8254x.so)

target_include_directories(ddi_acontis_sdk PUBLIC ../../ddi_acontis_sdk/include/)
target_include_directories(ddi_acontis_sdk PUBLIC ../../ddi_acontis_sdk/ddi_common_lib/)
target_include_directories(ddi_acontis_sdk PUBLIC ../../acontis_lib/SDK/INC/)
target_include_directories(ddi_acontis_sdk PUBLIC ../../acontis_lib/SDK/INC/Linux/)
target_include_directories(ddi_acontis_sdk PUBLIC /usr/include/)
link_directories(${CMAKE_SOURCE_DIR}/../ddi_ecat_master_lib/build/lib /opt/rh/devtoolset-9/root/usr/lib/gcc/x86_64-redhat-linux/9/)

# To fix the unknown linker error
set_target_properties(ddi_acontis_sdk PROPERTIES LINKER_LANGUAGE CXX)

# Link the DDI Acontis SDK , Conan libraries and Matlab libraries
target_link_libraries(sample_test ${MATLAB_ENGINE} ${MATLAB_DATA_ARRAY} dl odbc pthread ddi_acontis_sdk ddi_em_sdk ${CONAN_LIBS})

# Setup include directories
target_include_directories(sample_test PRIVATE ${FatTest_SOURCE_DIR}/include/)
target_include_directories(sample_test PRIVATE ${FatTest_SOURCE_DIR}/DVT/)
target_include_directories(sample_test PRIVATE ${FatTest_SOURCE_DIR}/Calibration/Include)
target_include_directories(sample_test PRIVATE ${FatTest_SOURCE_DIR}/ASMI/)
target_include_directories(sample_test PRIVATE ${FatTest_SOURCE_DIR}/UCSC/)
target_include_directories(sample_test PRIVATE ${FatTest_SOURCE_DIR}/InterlockBridge/)
target_include_directories(sample_test PRIVATE ${FatTest_SOURCE_DIR}/UART/)
target_include_directories(sample_test PRIVATE ${FatTest_SOURCE_DIR}/ECATMaster/)

target_include_directories(sample_test PUBLIC ${FATParent_SOURCE_DIR}/include/)
target_include_directories(sample_test PUBLIC ${FATParent_SOURCE_DIR}/../yarger/)
target_include_directories(sample_test PUBLIC ${FATParent_SOURCE_DIR}/../ddi_common_lib/)
target_include_directories(sample_test PUBLIC ${FATParent_SOURCE_DIR}/../ddi_acontis_sdk/include/)
target_include_directories(sample_test PUBLIC ${FATParent_SOURCE_DIR}/../ddi_acontis_sdk/acontis_lib/SDK/INC/)
target_include_directories(sample_test PUBLIC ${FATParent_SOURCE_DIR}/../ddi_acontis_sdk/acontis_lib/SDK/INC/Linux/)
target_include_directories(sample_test PUBLIC ${FATParent_SOURCE_DIR}/../ddi_ecat_master_lib/include/)
target_include_directories(sample_test PUBLIC /usr/local/MATLAB/R2022b/extern/include/)

enable_testing()
add_test(NAME simple_tests WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin COMMAND simple_tests)

# Usefule macro to see what is available in cmake
macro(print_all_variables)
    message(STATUS "print_all_variables------------------------------------------{")
    get_cmake_property(_variableNames VARIABLES)
    foreach (_variableName ${_variableNames})
        message(STATUS "${_variableName}=${${_variableName}}")
    endforeach()
    message(STATUS "print_all_variables------------------------------------------}")
endmacro()

# Uncomment this to print variables.
# print_all_variables()
