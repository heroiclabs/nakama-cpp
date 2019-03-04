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

cur_dir = os.path.abspath('.')
if cur_dir.find(' ') >= 0:
    print 'Error: space foud in path:', cur_dir
    print 'please remove spaces from path and try again'
    sys.exit(-1)

if len(sys.argv) < 2:
    print "Pass ABI parameter."
    print "e.g. armeabi-v7a, arm64-v8a, x86 or x86_64"
    sys.exit(-1)

ABI = sys.argv[1]
BUILD_MODE = 'Release'

def getEnvVar(name):
    if name in os.environ:
        return os.environ[name]
    return ''

ANDROID_NDK = getEnvVar('ANDROID_NDK')
if not ANDROID_NDK:
    ANDROID_NDK = getEnvVar('NDK_ROOT')
    if not ANDROID_NDK:
        print "Error: no ANDROID_NDK or NDK_ROOT environment variable"
        sys.exit(-1)

def call(command):
    res = subprocess.call(command, shell=False)
    if res != 0:
        sys.exit(-1)

build_dir = os.path.abspath('build/' + ABI + '/' + BUILD_MODE)
release_libs_dir = os.path.abspath('../../release/nakama-cpp-sdk/libs/android/' + ABI)

def makedirs(dir):
    if not os.path.isdir(dir):
        os.makedirs(dir)

def copy_file(src, dest):
    shutil.copy(src, dest)
    print 'copied', os.path.basename(src)

def copy_libs(dest):
    print
    print 'copying to release folder...'
    copy_file(build_dir + '/src/libnakama-cpp.a', dest)

print 'ANDROID_NDK=' + ANDROID_NDK

makedirs(build_dir)

cmake_args = [
              'cmake',
              '-DANDROID_ABI=' + ABI,
              '-DCMAKE_TOOLCHAIN_FILE=' + ANDROID_NDK + '/build/cmake/android.toolchain.cmake',
              '-DBUILD_DEFAULT_WEBSOCKETS=OFF',
              '-DCMAKE_BUILD_TYPE=' + BUILD_MODE,
              '-DANDROID_NATIVE_API_LEVEL=16',
              '-B',
              build_dir,
              '-GNinja',
              '../..'
              ]

# generate projects
call(cmake_args)

# build
call(['ninja', '-C', build_dir, 'nakama-cpp'])

makedirs(release_libs_dir)
copy_libs(release_libs_dir)
