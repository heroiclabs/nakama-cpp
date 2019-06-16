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
import os
import argparse
import shutil
import platform

execfile('../build_common.py')
init_common(os.path.abspath('..'))

parser = argparse.ArgumentParser(description='builder for Windows')
parser.add_argument('--so', help='build shared object', action='store_true')

args = parser.parse_args()
SO = args.so

bits, linkage = platform.architecture()

if bits == '64bit':
    ARCH = 'x64'
elif bits == '32bit':
    ARCH = 'x86'
else:
    ARCH = bits

BUILD_MODE = 'Release'
build_dir = os.path.abspath('build/' + BUILD_MODE + '_' + ARCH)

if SO:
    release_libs_path = os.path.abspath('../../release/nakama-cpp-sdk/shared-libs/linux/' + ARCH)
else:
    release_libs_path = os.path.abspath('../../release/nakama-cpp-sdk/libs/linux/' + ARCH)

print('Architecture:', ARCH)
print('Build mode  :', BUILD_MODE)
print('Shared object:', str(SO))

def build(target):
    print('building ' + target + '...')
    call('cmake --build . --target ' + target, shell=True)

def copy_nakama_lib():
    copy_file(build_dir + '/src/libnakama-cpp.a', release_libs_path)

def copy_protobuf_lib():
    copy_file(build_dir + '/third_party/grpc/third_party/protobuf/libprotobuf.a', release_libs_path)

def copy_ssl_lib():
    copy_file(build_dir + '/third_party/grpc/third_party/boringssl/crypto/libcrypto.a', release_libs_path)
    copy_file(build_dir + '/third_party/grpc/third_party/boringssl/ssl/libssl.a', release_libs_path)

def copy_grpc_lib():
    copy_file(build_dir + '/third_party/grpc/libaddress_sorting.a', release_libs_path)
    copy_file(build_dir + '/third_party/grpc/libgpr.a', release_libs_path)
    copy_file(build_dir + '/third_party/grpc/libgrpc++.a', release_libs_path)
    copy_file(build_dir + '/third_party/grpc/libgrpc.a', release_libs_path)
    copy_file(build_dir + '/third_party/grpc/third_party/cares/cares/lib/libcares.a', release_libs_path)
    copy_file(build_dir + '/third_party/grpc/third_party/zlib/libz.a', release_libs_path)

def copy_rest_lib():
    copy_file(build_dir + '/third_party/cpprestsdk/' + BUILD_MODE + '/Binaries/libcpprest.a', release_libs_path)

def copy_so(dest):
    print
    print('copying to release folder...')
    copy_file(build_dir + '/src/libnakama-cpp.so', dest)

makedirs(build_dir)
makedirs(release_libs_path)

os.chdir(build_dir)

# generate projects
call([
  'cmake',
  '-DCMAKE_BUILD_TYPE=' + BUILD_MODE,
  '-DNAKAMA_SHARED_LIBRARY=' + bool2cmake(SO),
  '-DBUILD_REST_CLIENT=' + bool2cmake(BUILD_REST_CLIENT),
  '-DBUILD_GRPC_CLIENT=' + bool2cmake(BUILD_GRPC_CLIENT),
  '-DBUILD_HTTP_CPPREST=' + bool2cmake(BUILD_HTTP_CPPREST),
  '-DBUILD_WEBSOCKET_CPPREST=' + bool2cmake(BUILD_WEBSOCKET_CPPREST),
  '../../../..'
])

build('grpc_cpp_plugin')
build('protoc')
build('nakama-cpp')

if SO:
    copy_so(release_libs_path)
else:
    copy_libs()

build('nakama-test')
