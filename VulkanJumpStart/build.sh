#!/bin/bash

mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DUSE_PLATFORM=VK_USE_PLATFORM_XLIB_KHR
make -j 8
cd ..