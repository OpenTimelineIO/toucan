#!/bin/bash

set -x

BUILD_TYPE=Release
if [ "$#" -eq 1 ]; then
    BUILD_TYPE=$1
fi

cmake -S toucan/cmake/SuperBuild -B superbuild-$BUILD_TYPE -DCMAKE_INSTALL_PREFIX=$PWD/install-$BUILD_TYPE -DCMAKE_PREFIX_PATH=$PWD/install-$BUILD_TYPE -DCMAKE_BUILD_TYPE=$BUILD_TYPE -Dtoucan_svt-av1=off
cmake --build superbuild-$BUILD_TYPE -j 4 --config $BUILD_TYPE

cmake -S toucan -B build-$BUILD_TYPE -DCMAKE_INSTALL_PREFIX=$PWD/install-$BUILD_TYPE -DCMAKE_PREFIX_PATH=$PWD/install-$BUILD_TYPE -DCMAKE_BUILD_TYPE=$BUILD_TYPE
cmake --build build-$BUILD_TYPE -j 4 --config $BUILD_TYPE
cmake --build build-$BUILD_TYPE --config $BUILD_TYPE --target install

