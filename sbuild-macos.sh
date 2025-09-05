#!/bin/sh

set -x

BUILD_TYPE=Release
if [ "$#" -eq 1 ]; then
    BUILD_TYPE=$1
fi

if [ $FFMPEG_MINIMAL = "" ]; then
    FFMPEG_MINIMAL = OFF
fi

JOBS=4

cmake \
    -S toucan/cmake/SuperBuild \
    -B sbuild-$BUILD_TYPE \
    -DCMAKE_INSTALL_PREFIX=$PWD/install-$BUILD_TYPE \
    -DCMAKE_PREFIX_PATH=$PWD/install-$BUILD_TYPE \
    -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
    -Dtoucan_FFmpeg_MINIMAL=$FFMPEG_MINIMAL
cmake --build sbuild-$BUILD_TYPE -j $JOBS --config $BUILD_TYPE

cmake \
    -S toucan \
    -B build-$BUILD_TYPE \
    -DCMAKE_INSTALL_PREFIX=$PWD/install-$BUILD_TYPE \
    -DCMAKE_PREFIX_PATH=$PWD/install-$BUILD_TYPE \
    -DCMAKE_BUILD_TYPE=$BUILD_TYPE
cmake --build build-$BUILD_TYPE -j $JOBS --config $BUILD_TYPE
cmake --build build-$BUILD_TYPE --config $BUILD_TYPE --target install

