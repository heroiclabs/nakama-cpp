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
import os
import sys

execfile('../build_common.py')
init_common(os.path.abspath('..'))

BUILD_MODE = 'Release'

arch_list = ['arm64',
            'armv7',
            'armv7s',
            'x86_64'   # Simulator
            ]

release_libs_dir = os.path.abspath('../../release/nakama-cpp-sdk/libs/ios')
makedirs(release_libs_dir)

def create_universal_lib(libs):
    if len(libs) == 0:
        return

    name = os.path.basename(libs[0])
    print 'creating universal library', name + ' ...'
    lipo_commands = ['lipo', '-create']
    for lib in libs:
        lipo_commands.append(lib)
    lipo_commands.append('-output')
    lipo_commands.append(release_libs_dir + '/' + name)
    call(lipo_commands)

build_dir = os.path.abspath('build/' + BUILD_MODE) + '/'

# static libs
nakama_cpp_libs = []
grpc_libs = []
grpcpp_libs = []
gpr_libs = []
address_sorting_libs = []
cares_libs = []
crypto_libs = []
ssl_libs = []
protobuf_libs = []
z_libs = []
cpprest_libs = []

for arch in arch_list:
    call(['python', 'build_ios.py', arch])
    
    build_arch_dir = build_dir + arch
    
    nakama_cpp_libs     .append(build_arch_dir + '/src/libnakama-cpp.a')
    grpc_libs           .append(build_arch_dir + '/third_party/grpc/libgrpc.a')
    grpcpp_libs         .append(build_arch_dir + '/third_party/grpc/libgrpc++.a')
    gpr_libs            .append(build_arch_dir + '/third_party/grpc/libgpr.a')
    address_sorting_libs.append(build_arch_dir + '/third_party/grpc/libaddress_sorting.a')
    cares_libs          .append(build_arch_dir + '/third_party/grpc/third_party/cares/cares/lib/libcares.a')
    crypto_libs         .append(build_arch_dir + '/third_party/grpc/third_party/boringssl/crypto/libcrypto.a')
    ssl_libs            .append(build_arch_dir + '/third_party/grpc/third_party/boringssl/ssl/libssl.a')
    protobuf_libs       .append(build_arch_dir + '/third_party/grpc/third_party/protobuf/libprotobuf.a')
    z_libs              .append(build_arch_dir + '/third_party/grpc/third_party/zlib/libz.a')
    cpprest_libs        .append(build_arch_dir + '/third_party/cpprestsdk/' + BUILD_MODE + '/Binaries/libcpprest.a')

make_universal_list = []

def copy_nakama_lib():
    make_universal_list.append(nakama_cpp_libs)

def copy_protobuf_lib():
    make_universal_list.append(protobuf_libs)

def copy_ssl_lib():
    make_universal_list.append(ssl_libs)
    make_universal_list.append(crypto_libs)

def copy_grpc_lib():
    make_universal_list.append(address_sorting_libs)
    make_universal_list.append(gpr_libs)
    make_universal_list.append(grpcpp_libs)
    make_universal_list.append(grpc_libs)
    make_universal_list.append(cares_libs)
    make_universal_list.append(z_libs)

def copy_rest_lib():
    make_universal_list.append(cpprest_libs)

copy_libs()

for libs_list in make_universal_list:
    create_universal_lib(libs_list)

# copy boost libs (they are already universal libs)
boost_libs_path = os.path.abspath('../../third_party/cpprestsdk/Build_iOS/boost/lib')
copy_file(os.path.join(boost_libs_path, 'libboost_chrono.a'), release_libs_dir)
copy_file(os.path.join(boost_libs_path, 'libboost_thread.a'), release_libs_dir)

# dynamic libs
release_libs_dir = os.path.abspath('../../release/nakama-cpp-sdk/shared-libs/ios')

for arch in arch_list:
    call(['python', 'build_ios.py', '--dylib', arch])
    
    build_arch_dir = build_dir + arch
    dest_dir = release_libs_dir + '/' + arch

    dylib_in_build = build_arch_dir + '/src/libnakama-cpp.dylib'
    call(['install_name_tool', '-id', '@executable_path/libnakama-cpp.dylib', dylib_in_build])
    
    makedirs(dest_dir)
    copy_file(dylib_in_build, dest_dir)

print 'done.'
