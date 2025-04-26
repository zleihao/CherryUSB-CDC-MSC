#!/bin/bash
rm -rf build 
mkdir build 
cd build

cmake -G "MinGW Makefiles" \
    -DCMAKE_BUILD_TYPE=Debug ..

# make
make VERBOSE=1

cd ..