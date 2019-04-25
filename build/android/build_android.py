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

parser = argparse.ArgumentParser(description='builder for Windows')
parser.add_argument('arch', help='architecture e.g. armeabi-v7a, arm64-v8a, x86, x86_64')
parser.add_argument('--so',  help='build shared object', action='store_true')

args = parser.parse_args()

if len(sys.argv) < 2:
    print "Pass ABI parameter."
    print ""
    sys.exit(-1)

ABI = args.arch
SHARED_LIB = args.so
BUILD_MODE = 'Release'

print
print 'Building for arch:', ABI + ', so:', str(SHARED_LIB)
print

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

if SHARED_LIB:
    release_libs_dir = os.path.abspath('../../release/nakama-cpp-sdk/shared-libs/android/' + ABI)
else:
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
    copy_file(build_dir + '/third_party/grpc/libaddress_sorting.a', dest)
    copy_file(build_dir + '/third_party/grpc/libgpr.a', dest)
    copy_file(build_dir + '/third_party/grpc/libgrpc++.a', dest)
    copy_file(build_dir + '/third_party/grpc/libgrpc.a', dest)
    copy_file(build_dir + '/third_party/grpc/third_party/cares/cares/lib/libcares.a', dest)
    copy_file(build_dir + '/third_party/grpc/third_party/protobuf/libprotobuf.a', dest)
    copy_file(build_dir + '/third_party/grpc/third_party/zlib/libz.a', dest)
    copy_file(build_dir + '/third_party/grpc/third_party/boringssl/crypto/libcrypto.a', dest)
    copy_file(build_dir + '/third_party/grpc/third_party/boringssl/ssl/libssl.a', dest)
    copy_file(build_dir + '/third_party/IXWebSocket/libixwebsocket.a', dest)

def copy_shared_lib(dest):
    print
    print 'copying to release folder...'
    copy_file(build_dir + '/src/libnakama-cpp.so', dest)

print 'ANDROID_NDK=' + ANDROID_NDK

if SHARED_LIB:
    NAKAMA_SHARED_LIBRARY = 'TRUE'
else:
    NAKAMA_SHARED_LIBRARY = 'FALSE'

makedirs(build_dir)

cmake_args = [
              'cmake',
              '-DANDROID_ABI=' + ABI,
              '-DCMAKE_TOOLCHAIN_FILE=' + ANDROID_NDK + '/build/cmake/android.toolchain.cmake',
              '-DCMAKE_BUILD_TYPE=' + BUILD_MODE,
              '-DNAKAMA_SHARED_LIBRARY=' + NAKAMA_SHARED_LIBRARY,
              '-DANDROID_NATIVE_API_LEVEL=16',
              '-DBUILD_WEBSOCKETPP=OFF',
              '-DBUILD_IXWEBSOCKET=ON',
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

if SHARED_LIB:
    copy_shared_lib(release_libs_dir)
else:
    copy_libs(release_libs_dir)
