#**************************************************************************
#(c) Copyright 2021 Digital Dynamics Inc. Scotts Valley CA USA.
#Unpublished copyright. All rights reserved. Contains proprietary and
#confidential trade secrets belonging to DDI. Disclosure or release without
#prior written authorization of DDI is prohibited.
#**************************************************************************
# DDI ECAT Master sample application project
project(ddi_em_sample_applications)

cmake_minimum_required(VERSION 3.3.2)

if ( ${BUILD_VERSION} )
  message("Using supplied build version: " STATUS ${BUILD_VERSION})
else()
  set(BUILD_VERSION "1.3.0")
  message("Using default build version: " STATUS ${BUILD_VERSION})
endif()

# Treat compiler warnings as errors
if (MSVC)
  # warning level 4
  add_compile_options(/W4)
else()
  # Use -Wall for compile
  add_compile_options(-DDEBUG -Wall -g)
endif()

# C11 support
set(CMAKE_CXX_STANDARD 11)

# Build Process Data test example
ADD_EXECUTABLE(ddi_em_pd_example
  ddi_em_process_data_example/main.cpp)
target_link_directories(ddi_em_pd_example PUBLIC ../build/lib/)
target_link_libraries(ddi_em_pd_example
  ddi_em_${BUILD_VERSION}
  pthread
  dl)
target_include_directories(ddi_em_pd_example
  PUBLIC
  config/
  ../include/
  include/
  )
