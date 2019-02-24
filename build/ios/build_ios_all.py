#!/usr/bin/env python
import os
import sys
import subprocess

BUILD_MODE = 'Release'

arch_list = ['arm64',
            'armv7',
            'armv7s',
            'x86_64'   # Simulator
            ]

def call(command):
    res = subprocess.call(command, shell=False)
    if res != 0:
        sys.exit(-1)

release_libs_dir = os.path.abspath('../../release/nakama-cpp-sdk/libs/ios')

if not os.path.isdir(release_libs_dir):
    os.makedirs(release_libs_dir)

def create_universal_lib(libs):
    name = os.path.basename(libs[0])
    print 'creating universal library', name + ' ...'
    lipo_commands = ['lipo', '-create']
    for lib in libs:
        lipo_commands.append(lib)
    lipo_commands.append('-output')
    lipo_commands.append(release_libs_dir + '/' + name)
    call(lipo_commands)

nakama_cpp_libs = []
grpc_libs = []
grpcpp_libs = []
gpr_libs = []
address_sorting_libs = []
cares_libs = []
crypto_libs = []
ssl_libs = []
protobuf_libs = []

build_dir = os.path.abspath('build/' + BUILD_MODE) + '/'

for arch in arch_list:
    print 'building for', arch
    call(['python', 'build_ios.py', arch])

    nakama_cpp_libs.append(build_dir + arch + '/src/libnakama-cpp.a')
    grpc_libs.append(build_dir + arch + '/third_party/grpc/libgrpc.a')
    grpcpp_libs.append(build_dir + arch + '/third_party/grpc/libgrpc++.a')
    gpr_libs.append(build_dir + arch + '/third_party/grpc/libgpr.a')
    address_sorting_libs.append(build_dir + arch + '/third_party/grpc/libaddress_sorting.a')
    cares_libs.append(build_dir + arch + '/third_party/grpc/third_party/cares/cares/lib/libcares.a')
    crypto_libs.append(build_dir + arch + '/third_party/grpc/third_party/boringssl/crypto/libcrypto.a')
    ssl_libs.append(build_dir + arch + '/third_party/grpc/third_party/boringssl/ssl/libssl.a')
    protobuf_libs.append(build_dir + arch + '/third_party/grpc/third_party/protobuf/libprotobuf.a')

create_universal_lib(nakama_cpp_libs)
create_universal_lib(grpc_libs)
create_universal_lib(grpcpp_libs)
create_universal_lib(gpr_libs)
create_universal_lib(address_sorting_libs)
create_universal_lib(cares_libs)
create_universal_lib(crypto_libs)
create_universal_lib(ssl_libs)
create_universal_lib(protobuf_libs)

print 'done.'
