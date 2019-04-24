#!/usr/bin/env python
#
# Copyright 2019 The Nakama Authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
import sys
import subprocess
import os
import shutil
import argparse

cur_dir = os.path.abspath('.')
if cur_dir.find(' ') >= 0:
    print 'Error: space foud in path:', cur_dir
    print 'please remove spaces from path and try again'
    sys.exit(-1)

parser = argparse.ArgumentParser(description='builder for iOS')
parser.add_argument('arch',     help='architecture e.g. arm64 armv7 armv7s x86_64')
parser.add_argument('--dylib',  help='build DynamicLib', action='store_true')

args = parser.parse_args()

ARCH = args.arch
SHARED_LIB = args.dylib
BUILD_MODE = 'Release'

print
print 'Building for', ARCH + ', dylib:', str(SHARED_LIB)
print

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

if SHARED_LIB:
    NAKAMA_SHARED_LIBRARY = 'TRUE'
else:
    NAKAMA_SHARED_LIBRARY = 'FALSE'

#generator = 'Xcode'
generator = 'Unix Makefiles'

# generate projects
call(['cmake',
      '-B',
      build_dir,
      '-DCMAKE_OSX_DEPLOYMENT_TARGET=8.0',
      '-DCMAKE_OSX_ARCHITECTURES=' + ARCH,
      '-Dprotobuf_BUILD_PROTOC_BINARIES=OFF',
      '-DgRPC_BUILD_CODEGEN=OFF',
      '-DCMAKE_TOOLCHAIN_FILE=' + cmake_toolchain_path,
      '-DCMAKE_BUILD_TYPE=' + BUILD_MODE,
      '-DNAKAMA_SHARED_LIBRARY=' + NAKAMA_SHARED_LIBRARY,
      '-DENABLE_BITCODE=FALSE',
      '-DENABLE_ARC=TRUE',
      '-DBUILD_WEBSOCKETPP=OFF',
      '-DBUILD_IXWEBSOCKET=ON',
      '-G' + generator,
      '../..'
      ])

build('nakama-cpp')

