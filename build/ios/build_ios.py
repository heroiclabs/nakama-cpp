#!/usr/bin/env python
import sys
import subprocess
import os
import shutil

if len(sys.argv) < 2:
    print "Pass ARCH parameter."
    print "e.g. arm64 armv7 armv7s x86_64"
    sys.exit(-1)

ARCH = sys.argv[1]
BUILD_MODE = 'Release'
build_dir = './build/' + BUILD_MODE + '/' + ARCH

if ARCH == 'x86_64':
    is_simulator = True
else:
    is_simulator = False

if not os.path.isdir(build_dir):
    os.makedirs(build_dir)

def call(command):
    res = subprocess.call(command, shell=False)
    if res != 0:
        sys.exit(-1)

def build(target):
    print 'building ' + target + '...'
    call(['cmake',
          '--build',
          build_dir,
          '--target',
          target
          ])

if is_simulator:
    cmake_toolchain_path = os.path.abspath('../../cmake/ios.simulator.toolchain.cmake')
else:
    cmake_toolchain_path = os.path.abspath('../../cmake/ios.toolchain.cmake')

#generator = 'Xcode'
generator = 'Unix Makefiles'

# generate projects
call(['cmake',
      '-B',
      build_dir,
      '-DCMAKE_OSX_ARCHITECTURES=' + ARCH,
      '-DBUILD_DEFAULT_WEBSOCKETS=OFF',
      '-Dprotobuf_BUILD_PROTOC_BINARIES=OFF',
      '-DgRPC_BUILD_CODEGEN=OFF',
      '-DCMAKE_TOOLCHAIN_FILE=' + cmake_toolchain_path,
      '-DCMAKE_BUILD_TYPE=' + BUILD_MODE,
      '-DENABLE_BITCODE=FALSE',
      '-DENABLE_ARC=TRUE',
      '-G' + generator,
      '../..'
      ])

build('nakama-cpp')

