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
init_common(os.path.abspath('..'))

parser = argparse.ArgumentParser(description='builder for Windows')
parser.add_argument('arch', help='architecture e.g. armeabi-v7a, arm64-v8a, x86, x86_64')
parser.add_argument('--so',  help='build shared object', action='store_true')

args = parser.parse_args()

if len(sys.argv) < 2:
    print("Pass ABI parameter.")
    print
    sys.exit(-1)

ABI = args.arch
SHARED_LIB = args.so
BUILD_MODE = 'Release'

print
print('Building for arch:', ABI + ', so:', str(SHARED_LIB))
print

ANDROID_NDK = getEnvVar('ANDROID_NDK')
if not ANDROID_NDK:
    ANDROID_NDK = getEnvVar('NDK_ROOT')
    if not ANDROID_NDK:
        print("Error: no ANDROID_NDK or NDK_ROOT environment variable")
        sys.exit(-1)

build_dir = os.path.abspath('build/' + ABI + '/' + BUILD_MODE)
cwd = os.getcwd()

if SHARED_LIB:
    release_libs_path = os.path.abspath('../../release/nakama-cpp-sdk/shared-libs/android/' + ABI)
else:
    release_libs_path = os.path.abspath('../../release/nakama-cpp-sdk/libs/android/' + ABI)

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
    print
    print('copying to release folder...')
    copy_file(build_dir + '/src/libnakama-cpp.so', dest)

print('ANDROID_NDK=' + ANDROID_NDK)

makedirs(build_dir)

if USE_CPPREST:
    boost_version = '1.69.0'

    CPPREST_ANDROID_PATH = os.path.abspath('../../third_party/cpprestsdk/Build_android')

    Android_Boost_BuildScript_Path = os.path.join(CPPREST_ANDROID_PATH, 'Boost-for-Android')

    if not os.path.exists(Android_Boost_BuildScript_Path):
        # clone Boost-for-Android
        call([
            'git', 'clone', 'https://github.com/moritz-wundke/Boost-for-Android',
            Android_Boost_BuildScript_Path
        ])
        os.chdir(Android_Boost_BuildScript_Path)
        call(['git', 'checkout', 'b1e2cb397d3ec573f1cfdf4f4d965766204c53f1'])
        os.chdir(cwd)

    boost_build_path = os.path.join(Android_Boost_BuildScript_Path, "build")
    if not os.path.exists(boost_build_path):
        # build boost
        os.chdir(Android_Boost_BuildScript_Path)

        if is_windows():
            build_script = 'build-android.bat'
            ANDROID_NDK = ANDROID_NDK.replace('\\', '/')
        else:
            build_script = './build-android.sh'

        call([build_script,
            '--with-libraries=system,thread,chrono',
            '--arch=armeabi-v7a,arm64-v8a,x86,x86_64',
            '--boost=' + boost_version,
            ANDROID_NDK
            ])
        os.chdir(cwd)

cmake_args = [
              'cmake',
              '-DANDROID_ABI=' + ABI,
              '-DCMAKE_TOOLCHAIN_FILE=' + ANDROID_NDK + '/build/cmake/android.toolchain.cmake',
              '-DCMAKE_BUILD_TYPE=' + BUILD_MODE,
              '-DANDROID_NATIVE_API_LEVEL=16',
              '-DNAKAMA_SHARED_LIBRARY=' + bool2cmake(SHARED_LIB),
              '-DBUILD_REST_CLIENT=' + bool2cmake(BUILD_REST_CLIENT),
              '-DBUILD_GRPC_CLIENT=' + bool2cmake(BUILD_GRPC_CLIENT),
              '-DBUILD_HTTP_CPPREST=' + bool2cmake(BUILD_HTTP_CPPREST),
              '-DBUILD_WEBSOCKET_CPPREST=' + bool2cmake(BUILD_WEBSOCKET_CPPREST),
              '-B',
              build_dir,
              '-GNinja',
              '../..'
              ]

# generate projects
call(cmake_args)

# build
call(['ninja', '-C', build_dir, 'nakama-cpp'])

makedirs(release_libs_path)

if SHARED_LIB:
    copy_shared_lib(release_libs_path)
else:
    copy_libs()
