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

parser = argparse.ArgumentParser(description='builder for Mac')
parser.add_argument('--dylib',  help='build DynamicLib', action='store_true')

args = parser.parse_args()

BUILD_MODE = 'Release'
SHARED_LIB = args.dylib

print
if SHARED_LIB:
    print 'Building dynamic lib'
else:
    print 'Building static lib'
print

build_dir = os.path.abspath('build/' + BUILD_MODE)

if SHARED_LIB:
    release_libs_path = os.path.abspath('../../release/nakama-cpp-sdk/shared-libs/mac')
else:
    release_libs_path = os.path.abspath('../../release/nakama-cpp-sdk/libs/mac')

if not os.path.isdir(build_dir):
    os.makedirs(build_dir)

def call(command):
    res = subprocess.call(command, shell=True)
    if res != 0:
        sys.exit(-1)

def build(target):
    print 'building ' + target + '...'
    call('cmake --build . --target ' + target + ' --config ' + BUILD_MODE)

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
    copy_file(build_dir + '/third_party/cpprestsdk/' + BUILD_MODE + '/Binaries/libcpprest.a', dest)

def copy_shared_lib(dest):
    print
    print 'copying to release folder...'
    copy_file(build_dir + '/src/libnakama-cpp.dylib', dest)

os.chdir(build_dir)

if SHARED_LIB:
    NAKAMA_SHARED_LIBRARY = 'TRUE'
else:
    NAKAMA_SHARED_LIBRARY = 'FALSE'

#generator = 'Xcode' # doesn't build crypto
generator = 'Ninja'

# generate projects
call('cmake' +
 ' -DCMAKE_OSX_DEPLOYMENT_TARGET=10.10' +
 ' -DENABLE_BITCODE=FALSE' +
 ' -DENABLE_ARC=TRUE' +
 ' -DBUILD_WEBSOCKETPP=ON' +
 ' -DNAKAMA_SHARED_LIBRARY=' + NAKAMA_SHARED_LIBRARY +
 ' -G' + generator +
 ' ../../../..')

build('grpc_cpp_plugin')
build('protoc')
build('nakama-cpp')

makedirs(release_libs_path)

if SHARED_LIB:
    copy_shared_lib(release_libs_path)
else:
    copy_libs(release_libs_path)

build('nakama-test')
