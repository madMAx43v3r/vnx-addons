#!/bin/bash

mkdir -p build

cd build

cmake -D CMAKE_CXX_FLAGS="-O3" $@ ..

make -j8 install

