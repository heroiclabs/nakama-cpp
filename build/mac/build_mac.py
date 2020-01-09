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

filename = '../build_common.py'
if sys.version_info[0] <= 2:
    execfile(filename)
else:
    exec(compile(open(filename, "rb").read(), filename, 'exec'))
init_common(os.path.abspath('..'), 'mac')

parser = argparse.ArgumentParser(description='builder for Mac')
parser.add_argument('--dylib',  help='build DynamicLib', action='store_true')

args = parser.parse_args()

SHARED_LIB = args.dylib

print('')
if SHARED_LIB:
    print('Building dynamic lib')
else:
    print('Building static lib')
print('')

set_build_folder_name(BUILD_MODE)

if SHARED_LIB:
    release_libs_path = os.path.abspath('../../release/nakama-cpp-sdk/shared-libs/mac')
else:
    release_libs_path = os.path.abspath('../../release/nakama-cpp-sdk/libs/mac')

def copy_nakama_lib():
    copy_file(BUILD_DIR + '/src/libnakama-cpp.a', release_libs_path)

def copy_protobuf_lib():
    copy_file(BUILD_DIR + '/third_party/grpc/third_party/protobuf/libprotobuf.a', release_libs_path)

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

def copy_shared_lib(dest):
    dylib_in_build = BUILD_DIR + '/src/libnakama-cpp.dylib'
    set_install_name(dylib_in_build)
    print('')
    print('copying to release folder...')
    copy_file(dylib_in_build, dest)

#generator = 'Xcode' # doesn't build crypto
generator = 'Ninja'

# generate projects
cmake_cmd = ['cmake',
 '-DCMAKE_OSX_DEPLOYMENT_TARGET=10.10',
 '-DENABLE_BITCODE=FALSE',
 '-DENABLE_ARC=TRUE',
 '-G' + generator,
 '-B', BUILD_DIR,
 '../..'
]

cmake_cmd.extend(get_common_cmake_parameters(SHARED_LIB))

call(cmake_cmd)

if BUILD_GRPC_CLIENT:
    build('grpc_cpp_plugin')
    build('protoc')

build('nakama-cpp')

makedirs(release_libs_path)

if SHARED_LIB:
    copy_shared_lib(release_libs_path)
else:
    copy_libs()

if BUILD_NAKAMA_TESTS:
    build('nakama-test')
