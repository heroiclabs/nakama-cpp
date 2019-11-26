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
import os
import sys

filename = '../build_common.py'
if sys.version_info[0] <= 2:
    execfile(filename)
else:
    exec(compile(open(filename, "rb").read(), filename, 'exec'))
init_common(os.path.abspath('..'), 'ios')

arch_list = ['arm64',
            'armv7',
            'armv7s',
            'x86_64'   # Simulator
            ]

release_libs_dir = os.path.abspath('../../release/nakama-cpp-sdk/libs/ios')

def create_universal_lib(libs):
    if len(libs) == 0:
        return

    name = os.path.basename(libs[0])
    print('creating universal library', name + ' ...')
    lipo_commands = ['lipo', '-create']
    for lib in libs:
        lipo_commands.append(lib)
    lipo_commands.append('-output')
    lipo_commands.append(release_libs_dir + '/' + name)
    call(lipo_commands)

build_dir = os.path.abspath('build/' + BUILD_MODE) + '/'

if BUILD_NAKAMA_STATIC:
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
        z_libs              .append(build_arch_dir + '/third_party/grpc/third_party/zlib/libz.a')
        cpprest_libs        .append(build_arch_dir + '/third_party/cpprestsdk/' + BUILD_MODE + '/Binaries/libcpprest.a')
        protobuf_lib = build_arch_dir + '/third_party/grpc/third_party/protobuf/libprotobuf.a'
        if not os.path.exists(protobuf_lib):
            protobuf_lib = build_arch_dir + '/third_party/grpc/third_party/protobuf/cmake/libprotobuf.a'
        protobuf_libs.append(protobuf_lib)

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

    makedirs(release_libs_dir)
    copy_libs()

    for libs_list in make_universal_list:
        create_universal_lib(libs_list)

    if USE_CPPREST:
        # copy boost libs (they are already universal libs)
        boost_libs_path = os.path.abspath('../../third_party/cpprestsdk/Build_iOS/boost/lib')
        copy_file(os.path.join(boost_libs_path, 'libboost_chrono.a'), release_libs_dir)
        copy_file(os.path.join(boost_libs_path, 'libboost_thread.a'), release_libs_dir)

if BUILD_NAKAMA_SHARED:
    # dynamic libs
    release_libs_dir = os.path.abspath('../../release/nakama-cpp-sdk/shared-libs/ios')

    nakama_cpp_libs = []

    for arch in arch_list:
        call(['python', 'build_ios.py', '--dylib', arch])
        
        build_arch_dir = build_dir + arch
        dylib_in_build = build_arch_dir + '/src/libnakama-cpp.dylib'

        if IOS_UNIVERSAL_SHARED_LIB:
            nakama_cpp_libs.append(dylib_in_build)
        else:
            dest_dir = release_libs_dir + '/' + arch
            makedirs(dest_dir)
            copy_file(dylib_in_build, dest_dir)
            set_install_name(dest_dir + '/libnakama-cpp.dylib')

    if IOS_UNIVERSAL_SHARED_LIB:
        makedirs(release_libs_dir)
        create_universal_lib(nakama_cpp_libs)
        set_install_name(release_libs_dir + '/libnakama-cpp.dylib')

print('done.')
