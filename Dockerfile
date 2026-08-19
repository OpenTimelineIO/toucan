# -*- mode: sh -*-
# vi: set ft=bash :

FROM ubuntu:latest AS builder

WORKDIR /home/ubuntu

RUN apt-get update && apt-get install -y build-essential \
			      cmake \
			      g++ \
			      git \
			      xorg-dev \
			      cimg-dev \
			      pkg-config \
			      libgl-dev \
			      libglu1-mesa-dev \
			      mesa-common-dev \
			      mesa-utils \
			      curl \
			      libva-dev \
			      libjpeg-dev \
			      libtiff-dev \
			      libpng++-dev \
			      libimath-dev \
			      libopencv-dev \
			      libopencolorio-dev \
			      libopentimelineio-dev

# Clone, compile and install feather-tk library
RUN mkdir feather-tk
RUN cd feather-tk; git clone https://github.com/grizzlypeak3d/feather-tk.git
# We revert feather-tk to the last date matching version to the current toucan main branch date.
# This is only needed until the toucan codebase and feather-tk library are synchronised in their APIs.
RUN cd feather-tk/feather-tk; git checkout 3cee68ec
RUN cd feather-tk; sh feather-tk/sbuild-linux.sh feather-tk
# TODO replace with cmake --target install 
RUN tar -C feather-tk/install-feather-tk -c -f - . | tar -C /usr/local -x -f -

# Add the pipx installation location:
ENV PATH=/root/.local/share/pipx/venvs/conan/bin:$PATH

# TODO Unable to get the release because of github blocking automation.
# curl -O https://github.com/AcademySoftwareFoundation/openfx/releases/download/OFX_Release_1.5.1/openfx-linux-ubuntu-1.5.1.tar.gz
RUN git clone https://github.com/AcademySoftwareFoundation/openfx.git
# Must turn off the plugins building with BUILD_EXAMPLE_PLUGINS=FALSE to avoid issues from cmake being unable to find OpenGL and CImg.
# Also, remove `--target install` from cmake because the generated Makefile doesn't have an install rule, for some reason?
# TODO this doesn't build on arm64 Linux currently, can't find the architecture.
# RUN cd openfx; scripts/build-cmake.sh -v Release -DBUILD_EXAMPLE_PLUGINS=FALSE -DPLUGIN_INSTALLDIR=./build/Install
# TODO need to find a better installation method, cmake --target install
RUN mkdir /usr/local/include/OpenFX
RUN tar -C openfx/include -c -f - . | tar -C /usr/local/include/OpenFX -x -f -

# RUN git clone https://github.com/OpenTimelineIO/toucan.git
# RUN git clone https://github.com/leighsmith/toucan.git
COPY . /home/ubuntu/toucan

# Build the toucan CLI tools.
RUN sh toucan/sbuild-linux.sh

# TODO Don't use the sbuild-linux.sh, install directly into /usr/local on the container.
# RUN cmake \
#     -S toucan/cmake/SuperBuild \
#     -B sbuild-Release \
#     -DCMAKE_INSTALL_PREFIX=/usr/local \
#     -DCMAKE_PREFIX_PATH=/usr/local \
#     -DCMAKE_BUILD_TYPE=Release \
#     -Dtoucan_FFmpeg_MINIMAL=OFF
# RUN cmake --build sbuild-Release -j 4 --config Release

# RUN cmake \
#     -S toucan \
#     -B build-Release \
#     -DCMAKE_INSTALL_PREFIX=/usr/local \
#     -DCMAKE_PREFIX_PATH=/usr/local \
#     -DCMAKE_BUILD_TYPE=Release
# RUN cmake --build build-Release -j 4 --config Release
# RUN cmake --build build-Release --config Release --target install

# Install the tools & libraries into a clean container.
FROM ubuntu:latest

# Copy just the libraries and resources needed to operate the CLI tools.
COPY --from=builder /usr/lib/aarch64-linux-gnu/ /usr/lib/aarch64-linux-gnu/
COPY --from=builder /usr/local/ /usr/local/
COPY --from=builder /home/ubuntu/install-Release/ /usr/local/

# Define the locations to search for libraries to include local libraries.
ENV LD_LIBRARY_PATH=/usr/local/lib
WORKDIR /home/ubuntu

# TODO for some reason, running toucan-render from the command line, using PATH to find
# it, without an absolute path causes the tool to hang, while running it as
# /usr/local/bin/toucan-render properly runs. It seems that argv[0] is used, and an
# unspecified path then causes the command to hang. The current work-around is to simply
# run it with the full path specified.
