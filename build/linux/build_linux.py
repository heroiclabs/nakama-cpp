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
from __future__ import print_function
import sys
import os
import argparse
import shutil
import platform

filename = '../build_common.py'
if sys.version_info[0] <= 2:
    execfile(filename)
else:
    exec(compile(open(filename, "rb").read(), filename, 'exec'))
init_common(os.path.abspath('..'), 'linux')

parser = argparse.ArgumentParser(description='builder for Windows')
parser.add_argument('--so', help='build shared object', action='store_true')

args = parser.parse_args()
SHARED_LIB = args.so

bits, linkage = platform.architecture()

if bits == '64bit':
    ARCH = 'x64'
elif bits == '32bit':
    ARCH = 'x86'
else:
    ARCH = bits

set_build_folder_name(BUILD_MODE + '_' + ARCH)

if SHARED_LIB:
    release_libs_path = os.path.abspath('../../release/nakama-cpp-sdk/shared-libs/linux/' + ARCH)
else:
    release_libs_path = os.path.abspath('../../release/nakama-cpp-sdk/libs/linux/' + ARCH)

print('Architecture:', ARCH)
print('Build mode  :', BUILD_MODE)
print('Shared object:', str(SHARED_LIB))

def copy_nakama_lib():
    copy_file(BUILD_DIR + '/src/libnakama-cpp.a', release_libs_path)

def copy_protobuf_lib():
    copy_one_file_from([
        BUILD_DIR + '/third_party/grpc/third_party/protobuf/libprotobuf.a',
        BUILD_DIR + '/third_party/grpc/third_party/protobuf/cmake/libprotobuf.a',
    ], release_libs_path)

def copy_ssl_lib():
    copy_file(BUILD_DIR + '/third_party/grpc/third_party/boringssl/crypto/libcrypto.a', release_libs_path)
    copy_file(BUILD_DIR + '/third_party/grpc/third_party/boringssl/ssl/libssl.a', release_libs_path)

def copy_grpc_lib():
    copy_file(BUILD_DIR + '/third_party/grpc/libaddress_sorting.a', release_libs_path)
    copy_file(BUILD_DIR + '/third_party/grpc/libgpr.a', release_libs_path)
    copy_file(BUILD_DIR + '/third_party/grpc/libgrpc++.a', release_libs_path)
    copy_file(BUILD_DIR + '/third_party/grpc/libgrpc.a', release_libs_path)
    copy_file(BUILD_DIR + '/third_party/grpc/third_party/cares/cares/lib/libcares.a', release_libs_path)
    copy_file(BUILD_DIR + '/third_party/grpc/third_party/zlib/libz.a', release_libs_path)

def copy_rest_lib():
    copy_file(BUILD_DIR + '/third_party/cpprestsdk/' + BUILD_MODE + '/Binaries/libcpprest.a', release_libs_path)

def copy_so(dest):
    print
    print('copying to release folder...')
    copy_file(BUILD_DIR + '/src/libnakama-cpp.so', dest)

makedirs(release_libs_path)

# generate projects
cmake_cmd = [
  'cmake',
  '-B', BUILD_DIR,
  '../..'
]

cmake_cmd.extend(get_common_cmake_parameters(SHARED_LIB))

call(cmake_cmd)

if BUILD_GRPC_CLIENT:
    build('grpc_cpp_plugin')
    build('protoc')

build('nakama-cpp')

if SHARED_LIB:
    copy_so(release_libs_path)
else:
    copy_libs()

if BUILD_NAKAMA_TESTS:
    build('nakama-test')
