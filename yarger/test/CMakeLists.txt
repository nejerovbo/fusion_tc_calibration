project(yarger-test)
cmake_minimum_required(VERSION 3.17)

file(GLOB SRCS *-test.cpp)
set(CMAKE_CXX_STANDARD 11)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

ADD_EXECUTABLE(${PROJECT_NAME} ${SRCS})
include_directories(../)
target_link_libraries(${PROJECT_NAME} ${CONAN_LIBS})

enable_testing()
add_test(${PROJECT_NAME} SimpleTest WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin COMMAND ${PROJECT_NAME})
