cmake_minimum_required(VERSION 3.14)

project(FATParent)

add_custom_target(ddi_acontis_sdk_build
	/usr/bin/make -C "${CMAKE_CURRENT_SOURCE_DIR}/ddi_acontis_sdk/")

add_subdirectory(src)
add_subdirectory(tests)

