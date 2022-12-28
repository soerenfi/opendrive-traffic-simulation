#!/bin/bash

export GENERATOR="Unix Makefiles"

mkdir -p build 
cd build
cmake -DCMAKE_BUILD_TYPE=Release ../
cmake --build . -j ${nproc}