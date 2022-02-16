#!/bin/bash

mkdir -p build

cd build

cmake -D VNX_ADDONS_BUILD_TESTS=ON -D CMAKE_CXX_FLAGS="-g -O3 -fmax-errors=1" ..

make -j8 $@

