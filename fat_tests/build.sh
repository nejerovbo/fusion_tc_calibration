#!/usr/bin/env bash
# Helper script to build the gtest apps
conan profile update settings.compiler.libcxx=libstdc++11 default  # Sets libcxx to C++11 ABI
conan install . -if build/ 
cmake -B build/ -DDEBUG=1 -DCMAKE_BUILD_TYPE=DEBUG
cmake --clean-first build/
cmake --build build/
