#!/bin/sh
set -e
sudo apt-get update
sudo apt-get install -y build-essential autoconf libtool pkg-config wget
sudo apt-get install -y libgflags-dev libgtest-dev
sudo apt-get install -y clang libc++-dev golang perl
mkdir -p build && cd build
wget https://dl.bintray.com/boostorg/release/1.69.0/source/boost_1_69_0.tar.gz
tar xvzf boost_1_69_0.tar.gz
cd boost_1_69_0
./bootstrap.sh --with-libraries=system,chrono,thread
./b2
set BOOST_ROOT=$PWD
cd ..
wget https://github.com/Kitware/CMake/releases/download/v3.15.5/cmake-3.15.5.tar.gz
tar xvzf cmake-3.15.5.tar.gz
cd cmake-3.15.5
./bootstrap && make
sudo make install
echo
echo "========================================================"
echo "please run this command before building nakama-cpp:" 
echo "export BOOST_ROOT=$BOOST_ROOT"
echo "========================================================"
