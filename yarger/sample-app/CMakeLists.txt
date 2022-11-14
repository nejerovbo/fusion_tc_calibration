project(yarger-sample)
cmake_minimum_required(VERSION 3.17)

file(GLOB SRCS *.cpp)
set(CMAKE_CXX_STANDARD 14)
#SET(CMAKE_CXX_FLAGS_DEBUG "-g")
SET(CMAKE_C_FLAGS_DEBUG "-D_DEBUG")

set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -D_DEBUG")

SET(CMAKE_BUILD_TYPE Debug)

if (CMAKE_BUILD_TYPE MATCHES Debug)
    message(" ")
    message("CMAKE IN DEBUG MODE")
    message(" ")
elseif(CMAKE_BUILD_TYPE MATCHES Release)
    message(" ")
    message("CMAKE IN RELEASE MODE")
    message(" ")
endif ()

ADD_EXECUTABLE(${PROJECT_NAME} ${SRCS})
include_directories(../)

