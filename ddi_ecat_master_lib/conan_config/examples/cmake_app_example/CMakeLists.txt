project(ddi_hello_world)

cmake_minimum_required(VERSION 3.14)

# Adds main source file
file(GLOB SRCS
    hello_world.cpp)
# Sets C++ 11
set(CMAKE_CXX_STANDARD 11)

# Use the conan configuration from conanfile.txt
include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

# Create a basic executable
ADD_EXECUTABLE(${PROJECT_NAME} ${SRCS})
