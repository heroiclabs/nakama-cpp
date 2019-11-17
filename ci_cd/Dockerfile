## Copyright 2019 The Nakama Authors
##
## Licensed under the Apache License, Version 2.0 (the "License");
## you may not use this file except in compliance with the License.
## You may obtain a copy of the License at
##
## http:##www.apache.org/licenses/LICENSE-2.0
##
## Unless required by applicable law or agreed to in writing, software
## distributed under the License is distributed on an "AS IS" BASIS,
## WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
## See the License for the specific language governing permissions and
## limitations under the License.

# nakama-cpp base image for Ubuntu 18.04 x64
#
# VERSION               1.0.0

FROM ubuntu:18.04
LABEL Description="This image is used to build nakama-cpp on Ubuntu 18.04 x64" Vendor="Heroiclabs" Version="1.0"

RUN apt-get update && apt-get install -y \
  autoconf \
  build-essential \
  clang \
  golang \
  libc++-dev \
  libgflags-dev \
  libgtest-dev \
  libtool pkg-config \
  perl \
  wget \
  git \
  python \
&& rm -rf /var/lib/apt/lists/*

# download and build boost
ARG BOOST_TMP_DIR=/tmp/boost_1_69_0
ENV BOOST_ROOT /boost_1_69_0
WORKDIR /tmp
RUN wget https://dl.bintray.com/boostorg/release/1.69.0/source/boost_1_69_0.tar.gz && tar xvzf boost_1_69_0.tar.gz
WORKDIR $BOOST_TMP_DIR
RUN ./bootstrap.sh --with-libraries=system,chrono,thread --prefix=$BOOST_ROOT && ./b2 install && rm -rf $BOOST_TMP_DIR

# download and build CMake
ARG CMAKE_TMP_DIR=/tmp/cmake-3.15.5
WORKDIR /tmp
RUN wget https://github.com/Kitware/CMake/releases/download/v3.15.5/cmake-3.15.5.tar.gz && tar xvzf cmake-3.15.5.tar.gz
WORKDIR $CMAKE_TMP_DIR
RUN ./bootstrap && make && make install && rm -rf $CMAKE_TMP_DIR
