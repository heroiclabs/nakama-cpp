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

if len(sys.argv) < 2:
    print("Pass ARCH parameter.")
    print("e.g. arm64 armv7 armv7s x86_64")
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
    print('building ' + target + '...')
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
      '-DCMAKE_TOOLCHAIN_FILE=' + cmake_toolchain_path,
      '-DCMAKE_BUILD_TYPE=' + BUILD_MODE,
      '-DENABLE_BITCODE=FALSE',
      '-DENABLE_ARC=TRUE',
      '-G' + generator,
      '../..'
      ])

build('nakama-cmake-client-example')
