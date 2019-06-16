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

filename = '../build_common.py'
if sys.version_info[0] <= 2:
    execfile(filename)
else:
    exec(compile(open(filename, "rb").read(), filename, 'exec'))
init_common(os.path.abspath('..'))

parser = argparse.ArgumentParser(description='builder for Mac')
parser.add_argument('--dylib',  help='build DynamicLib', action='store_true')

args = parser.parse_args()

BUILD_MODE = 'Release'
SHARED_LIB = args.dylib

print
if SHARED_LIB:
    print('Building dynamic lib')
else:
    print('Building static lib')
print

build_dir = os.path.abspath('build/' + BUILD_MODE)

if SHARED_LIB:
    release_libs_path = os.path.abspath('../../release/nakama-cpp-sdk/shared-libs/mac')
else:
    release_libs_path = os.path.abspath('../../release/nakama-cpp-sdk/libs/mac')

makedirs(build_dir)

def build(target):
    print('building ' + target + '...')
    call('cmake --build . --target ' + target + ' --config ' + BUILD_MODE, shell=True)

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

def copy_shared_lib(dest):
    dylib_in_build = build_dir + '/src/libnakama-cpp.dylib'
    call(['install_name_tool', '-id', '@executable_path/libnakama-cpp.dylib', dylib_in_build])
    print
    print('copying to release folder...')
    copy_file(dylib_in_build, dest)

os.chdir(build_dir)

#generator = 'Xcode' # doesn't build crypto
generator = 'Ninja'

# generate projects
call('cmake' +
 ' -DCMAKE_OSX_DEPLOYMENT_TARGET=10.10' +
 ' -DENABLE_BITCODE=FALSE' +
 ' -DENABLE_ARC=TRUE' +
 ' -DNAKAMA_SHARED_LIBRARY=' + bool2cmake(SHARED_LIB) +
 ' -DBUILD_REST_CLIENT=' + bool2cmake(BUILD_REST_CLIENT) +
 ' -DBUILD_GRPC_CLIENT=' + bool2cmake(BUILD_GRPC_CLIENT) +
 ' -DBUILD_HTTP_CPPREST=' + bool2cmake(BUILD_HTTP_CPPREST) +
 ' -DBUILD_WEBSOCKET_CPPREST=' + bool2cmake(BUILD_WEBSOCKET_CPPREST) +
 ' -G' + generator +
 ' ../../../..', shell=True)

build('grpc_cpp_plugin')
build('protoc')
build('nakama-cpp')

makedirs(release_libs_path)

if SHARED_LIB:
    copy_shared_lib(release_libs_path)
else:
    copy_libs()

build('nakama-test')
