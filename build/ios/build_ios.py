#!/usr/bin/env python
import sys
import subprocess
import os

if len(sys.argv) < 2:
    print "Pass ARCH parameter."
    print "e.g. arm64 armv7 armv7s x86_64"
    sys.exit(-1)

ARCH = sys.argv[1]
BUILD_MODE = 'Release'
build_dir = './build/' + ARCH + '/' + BUILD_MODE

if not os.path.isdir(build_dir):
    os.makedirs(build_dir)

def call(command):
    res = subprocess.call(command, shell=True)
    if res != 0:
        sys.exit(-1)

def build(target):
    print 'building ' + target + '...'
    call('cmake --build . --target ' + target + ' --config ' + BUILD_MODE)

cmake_toolchain_path = os.path.abspath('../../cmake/ios.toolchain.cmake')

print 'cmake_toolchain_path=', cmake_toolchain_path

'''
# generate make files
call('cmake' +
     ' -DCMAKE_TOOLCHAIN_FILE=../../../../cmake/ios.toolchain.cmake' +
     ' -DIOS_PLATFORM=SIMULATOR64' +
     ' -DIOS_ARCH=' + ARCH +
     ' -DENABLE_BITCODE=FALSE' +
     ' -DENABLE_ARC=TRUE' +
     #     ' -DGO_EXECUTABLE=go' +
     #' -DPERL_EXECUTABLE=perl' +
     ' ../../../..')
'''
# generate XCode projects
call('cmake' +
          ' -B ' + build_dir +
     # ' -DCMAKE_OSX_SYSROOT=iphoneos' +
 ' -DCMAKE_OSX_ARCHITECTURES=' + ARCH +
 ' -DCMAKE_TOOLCHAIN_FILE=' + cmake_toolchain_path +
     #' -DBUILD_DEFAULT_WEBSOCKETS=OFF' +
     # ' -DBOOST_ROOT=/Users/dimo4eg/dev/libs/boost_1_69_0' +
     #' -DBOOST_INCLUDEDIR=/Users/dimo4eg/dev/libs/boost_1_69_0/boost' +
     #' -DBOOST_LIBRARYDIR=/Users/dimo4eg/dev/libs/boost_1_69_0/stage/lib' +
     #' -DBoost_NO_SYSTEM_PATHS=NO' +
     #' -DBoost_ADDITIONAL_VERSIONS=1.69.0' +
 ' -DENABLE_BITCODE=FALSE' +
 ' -DENABLE_ARC=TRUE' +
 ' -DBUILD_DEFAULT_WEBSOCKETS=OFF' +
     ' -Dprotobuf_BUILD_PROTOC_BINARIES=OFF' +
     ' -DGO_EXECUTABLE=/usr/bin/go' + # need
     ' -DPERL_EXECUTABLE=/usr/bin/perl' + # need
     ' -DGIT_EXECUTABLE=/usr/bin/git' +
     ' -DCMAKE_MAKE_PROGRAM=Ninja' +
     # ' -G"Xcode"' +
     ' -GNinja' +
 ' ../..')

build('nakama-cpp')

#print 'installing...'
#call('cmake --build ' + build_dir + ' --target install')
