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
import os
import subprocess
import argparse
import shutil

parser = argparse.ArgumentParser(description='builder for Windows')
parser.add_argument('-m', '--mode', help='build mode: Debug or Release')
parser.add_argument('-a', '--arch', help='architecture: x86 or x64')

args = parser.parse_args()

BUILD_MODE = 'Debug'
ARCH = 'x86'

if args.mode:
    valid_modes = ['Debug', 'Release']
    if args.mode in valid_modes:
        BUILD_MODE = args.mode
    else:
        print 'Not valid mode. Supported values:', str(valid_modes)
        sys.exit(-1)

if args.arch:
    valid_archs = ['x86', 'x64']
    if args.arch in valid_archs:
        ARCH = args.arch
    else:
        print 'Not valid architecture. Supported values:', str(valid_archs)
        sys.exit(-1)

build_dir = os.path.abspath('build\\' + ARCH)

def makedirs(dir):
    if not os.path.isdir(dir):
        os.makedirs(dir)

def call(command):
    res = subprocess.call(command)
    if res != 0:
        sys.exit(-1)

def build(target):
    print 'building ' + target + ' for ' + BUILD_MODE + '...'
    call('cmake --build ' + build_dir + ' --target ' + target + ' --config ' + BUILD_MODE)

# generate Visual Studio projects
#generator = 'Visual Studio 14 2015'
generator = 'Visual Studio 15 2017'

if ARCH == 'x64':
    generator += ' Win64'

makedirs(build_dir)

call('cmake -B ' + build_dir +
 ' -G"' + generator + '"' +
 ' -DCMAKE_BUILD_TYPE=' + BUILD_MODE +
 ' ../..')

build('nakama-cmake-client-example')
