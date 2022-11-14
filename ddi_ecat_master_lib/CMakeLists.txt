#**************************************************************************
#(c) Copyright 2021 Digital Dynamics Inc. Scotts Valley CA USA.
#Unpublished copyright. All rights reserved. Contains proprietary and
#confidential trade secrets belonging to DDI. Disclosure or release without
#prior written authorization of DDI is prohibited.
#**************************************************************************
# DDI ECAT Master project
project(ddi_em)

cmake_minimum_required(VERSION 3.3.2)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

#set(BUILD_VERSION "BUILD_VERSION" CACHE STRING "Description")
if ( ${BUILD_VERSION} )
  message("Using supplied build version: " STATUS ${BUILD_VERSION})
else()
  set(BUILD_VERSION "1.3.0")
  message("Using default build version: " STATUS ${BUILD_VERSION})
endif()

set (DDI_EM_VERSION ddi_em_${BUILD_VERSION})

# Treat compiler warnings as errors
if (MSVC)
  # warning level 4
  add_compile_options(/W4)
else()
  # Use -Wall for compile
  add_compile_options(-DDEBUG -Wall -g)
endif()

# DDI ECAT Master SDK source files
file(GLOB SRCS
    src/ddi_em.cpp
    src/ddi_em_logging.cpp
    src/ddi_em_notifications.cpp
    src/ddi_em_state_change.cpp
    src/ddi_em_slave_management.cpp
    src/ddi_em_link_layer.cpp
    src/ddi_em_coe.cpp
    src/ddi_em_foe.cpp
    src/ddi_em_process_data.cpp
    src/ddi_em_translate.cpp
    src/ddi_em_remote_access.cpp
    src/ddi_em_eeprom_esc_regs.cpp
    src/fusion_sdk/ddi_em_fusion_uart.cpp
    src/fusion_sdk/ddi_em_fusion_interface.cpp
    )

# C11 support
set(CMAKE_CXX_STANDARD 11)

# Build ECAT SDK as a shared library, Acontis depends on libdl which can't
# be static
ADD_LIBRARY(${DDI_EM_VERSION} SHARED ${SRCS})

# Add include paths
target_include_directories(${DDI_EM_VERSION} PUBLIC include/)
target_include_directories(${DDI_EM_VERSION} PUBLIC src/)
target_include_directories(${DDI_EM_VERSION} PUBLIC src/fusion_sdk/)
target_include_directories(${DDI_EM_VERSION} PUBLIC acontis_lib/SDK/INC/)
target_include_directories(${DDI_EM_VERSION} PUBLIC acontis_lib/SDK/INC/Linux/)

# Add acontis libraries to libddi_em
target_link_libraries(${DDI_EM_VERSION}
 ${CONAN_LIBS}
 ${CMAKE_SOURCE_DIR}/acontis_lib/SDK/LIB/Linux/x64/libAtemRasSrv.a
 ${CMAKE_SOURCE_DIR}/acontis_lib/SDK/LIB/Linux/x64/libEcMaster.a
 ${CMAKE_SOURCE_DIR}/acontis_lib/Bin/Linux/x64/libemllI8254x.so)

# Build Engineering test application
ADD_EXECUTABLE(ddi_em_test
  tests/ddi_em_test.cpp
  tests/ddi_em_uart.cpp
  tests/ddi_em_common.cpp)
target_link_libraries(ddi_em_test
  ${CONAN_LIBS}
  ${DDI_EM_VERSION}
  pthread
  dl)
target_include_directories(ddi_em_test
  PUBLIC
  include/
  tests/config/
  )

# Build FOE test application
ADD_EXECUTABLE(ddi_em_foe_test
  tests/ddi_em_foe.cpp)
target_link_libraries(ddi_em_foe_test
  ${CONAN_LIBS}
  ${DDI_EM_VERSION}
  pthread
  dl)
target_include_directories(ddi_em_foe_test
  PUBLIC
  include/
  tests/config/
  )
  
# Build Sample test applications
add_subdirectory(sample_applications)

# Create the build_artifacts directory, this artifact directory will be packed by the GitHub action
# The build_artifacts directory currently has the following layout:
# <build_artifacts>/
#                   lib/                   # This contains the lib${DDI_EM_VERSION}.so and libemllI8254x.so library
#                   include/               # This contains the ddi_em exported includes
#                   sample_applications    # This contains the sample applications
#                   docs/                  # This contains documentation
#                   linux_installer.sh     # This contains the Linux installer
set(INSTALL_DIR ${CMAKE_BINARY_DIR}/"build_artifacts")
# Create the <build_artifact>/lib/ directory
add_custom_command(TARGET ${DDI_EM_VERSION} POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E make_directory ${INSTALL_DIR}/include/${DDI_EM_VERSION}/)
# Export the ddi_em/include/ directory - make the installation directory
add_custom_command(TARGET ${DDI_EM_VERSION} POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/include/ ${INSTALL_DIR}/include/${DDI_EM_VERSION}/)
# Create the <build_artifact>/lib/ directory
add_custom_command(TARGET ${DDI_EM_VERSION} POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E make_directory ${INSTALL_DIR}/lib/)
# Install the ddi_em library
add_custom_command(TARGET ${DDI_EM_VERSION} POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/lib/lib${DDI_EM_VERSION}.so ${INSTALL_DIR}/lib/lib${DDI_EM_VERSION}.so)
# Install the Acontis optmized link layer library
add_custom_command(TARGET ${DDI_EM_VERSION} POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/acontis_lib/Bin/Linux/x64/libemllI8254x.so ${INSTALL_DIR}/lib/)
# Install the installer
add_custom_command(TARGET ${DDI_EM_VERSION} POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/installer/linux_installer.sh ${INSTALL_DIR})

# Create the <build_artifact>/docs/ directory
find_package(Doxygen) # Utilize cmake support for doxygen docs
  if(DOXYGEN_FOUND)
    # set input and output files
    set(DOXYGEN_IN )
    # request to generate doxygen documetation
    add_custom_target( doc_doxygen ALL
      COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/docs/doxygen_file/doxygen_file
      WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
      COMMENT "Generating API documentation with Doxygen"
      VERBATIM )
    # Create doxygen directory
    add_custom_command(TARGET ${DDI_EM_VERSION} doc_doxygen
      COMMAND ${CMAKE_COMMAND} -E make_directory ${INSTALL_DIR}/docs/doxygen/)
    # Install the documentation
    add_custom_command(TARGET ${DDI_EM_VERSION} doc_doxygen
      COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/html/* ${INSTALL_DIR}/docs/doxygen/)
  endif(DOXYGEN_FOUND)
