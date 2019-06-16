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
import platform

bits, linkage = platform.architecture()

if bits == '64bit':
    ARCH = 'x64'
elif bits == '32bit':
    ARCH = 'x86'
else:
    ARCH = bits

BUILD_MODE = 'Release'
build_dir = os.path.abspath('build/' + BUILD_MODE + '_' + ARCH)

print('Architecture:', ARCH)
print('Build mode  :', BUILD_MODE)

def makedirs(dir):
    if not os.path.isdir(dir):
        os.makedirs(dir)

def call(command, shell=False):
    res = subprocess.call(command, shell=shell)
    if res != 0:
        sys.exit(-1)

def build(target):
    print('building ' + target + '...')
    call('cmake --build . --target ' + target, shell=True)

makedirs(build_dir)

os.chdir(build_dir)

# generate projects
call([
 'cmake',
 '-DCMAKE_BUILD_TYPE=' + BUILD_MODE,
 '../../../..'
])

build('nakama-cmake-client-example')
