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

filename = '../build_common.py'
if sys.version_info[0] <= 2:
    execfile(filename)
else:
    exec(compile(open(filename, "rb").read(), filename, 'exec'))
init_common(os.path.abspath('..'), 'windows')

parser = argparse.ArgumentParser(description='builder for Windows')
parser.add_argument('-m', '--mode', help='build mode: Debug or Release')
parser.add_argument('-a', '--arch', help='architecture: x86 or x64')
parser.add_argument('-t', '--tool', help='platform toolset: v140, v141, v142, v143...')
parser.add_argument('-s', '--stdcpp', help='C++ standard: 11, 14, 17, 20...')
parser.add_argument(      '--dll',  help='build DLL', action='store_true')

args = parser.parse_args()

BUILD_MODE = 'Debug'
ARCH = 'x86'
TOOLSET = 'v142'
CXX_STANDARD = ''
DLL = args.dll

if args.mode:
    valid_modes = ['Debug', 'Release']
    if args.mode in valid_modes:
        BUILD_MODE = args.mode
    else:
        print('Not valid mode. Supported values:', str(valid_modes))
        sys.exit(-1)

if args.arch:
    valid_archs = ['x86', 'x64']
    if args.arch in valid_archs:
        ARCH = args.arch
    else:
        print('Not valid architecture. Supported values:', str(valid_archs))
        sys.exit(-1)

if args.tool:
    valid_toolsets = ['v140', 'v141', 'v142', 'v143']
    if args.tool in valid_toolsets:
        TOOLSET = args.tool
    else:
        print('Not valid toolset. Supported values:', str(valid_toolsets))
        sys.exit(-1)

if args.stdcpp:
    valid_stdcpps = ['11', '14', '17', '20']
    if args.stdcpp in valid_stdcpps:
        CXX_STANDARD = args.stdcpp
    else:
        print('Not valid C++ standard. Supported values:', str(valid_stdcpps))
        sys.exit(-1)

if BUILD_MODE == 'Debug':
    libs_postfix = 'd'
else:
    libs_postfix = ''

def copy_nakama_lib():
    copy_file(BUILD_DIR + '\\src\\' + BUILD_MODE + '\\nakama-cpp' + libs_postfix + '.lib', release_libs_path)

def copy_protobuf_lib():
    copy_file(BUILD_DIR + '\\third_party\\grpc\\third_party\\protobuf\\' + BUILD_MODE + '\\libprotobuf' + libs_postfix + '.lib', release_libs_path)

def copy_ssl_lib():
    copy_file(BUILD_DIR + '\\third_party\\grpc\\third_party\\boringssl-with-bazel\\' + BUILD_MODE + '\\ssl' + libs_postfix + '.lib', release_libs_path)
    copy_file(BUILD_DIR + '\\third_party\\grpc\\third_party\\boringssl-with-bazel\\' + BUILD_MODE + '\\crypto' + libs_postfix + '.lib', release_libs_path)

def copy_grpc_lib():
    copy_file(BUILD_DIR + '\\third_party\\grpc\\' + BUILD_MODE + '\\address_sorting' + libs_postfix + '.lib', release_libs_path)
    copy_file(BUILD_DIR + '\\third_party\\grpc\\' + BUILD_MODE + '\\gpr' + libs_postfix + '.lib', release_libs_path)
    copy_file(BUILD_DIR + '\\third_party\\grpc\\' + BUILD_MODE + '\\grpc++' + libs_postfix + '.lib', release_libs_path)
    copy_file(BUILD_DIR + '\\third_party\\grpc\\' + BUILD_MODE + '\\grpc' + libs_postfix + '.lib', release_libs_path)
    copy_file(BUILD_DIR + '\\third_party\\grpc\\third_party\\cares\\cares\\lib\\' + BUILD_MODE + '\\cares' + libs_postfix + '.lib', release_libs_path)
    copy_file(BUILD_DIR + '\\third_party\\grpc\\third_party\\zlib\\' + BUILD_MODE + '\\zlibstatic' + libs_postfix + '.lib', release_libs_path + '\\zlib' + libs_postfix + '.lib')

def copy_rest_lib():
    copy_file(BUILD_DIR + '\\third_party\\cpprestsdk\\Release\\Binaries\\' + BUILD_MODE + '\\cpprest' + libs_postfix + '.lib', release_libs_path)

def copy_dll(dest):
    print
    print('copying to release folder...')
    copy_file(BUILD_DIR + '\\src\\' + BUILD_MODE + '\\nakama-cpp' + libs_postfix + '.lib', dest)
    copy_file(BUILD_DIR + '\\src\\' + BUILD_MODE + '\\nakama-cpp' + libs_postfix + '.dll', dest)

# generate Visual Studio projects
if TOOLSET == 'v140':
    generator, vs_year = 'Visual Studio 14 2015', 2015
elif TOOLSET == 'v141':
    generator, vs_year = 'Visual Studio 15 2017', 2017
elif TOOLSET == 'v142':
    generator, vs_year = 'Visual Studio 16 2019', 2019
elif TOOLSET == 'v143':
    generator, vs_year = 'Visual Studio 17 2022', 2022

if ARCH == 'x64' and vs_year < 2019:
    generator += ' Win64'

set_build_folder_name(TOOLSET + '_' + ARCH)

print
print('Building for Arch:', ARCH + ', Toolset:', TOOLSET + ', Mode:', BUILD_MODE + ', DLL:', str(DLL))
print

if ARCH == 'x64':
    arch_folder = 'win64'
else:
    arch_folder = 'win32'

# Windows ver | SYSTEM_VERSION
# ----------------------------
# Windows 7   | 6.1
# Windows 8   | 6.2
# Windows 8.1 | 6.3
# Windows 10  | 10.0
SYSTEM_VERSION = '10.0'

cmake_cmd = ['cmake',
  '-B', BUILD_DIR,
  '-G', generator,
  '-T', TOOLSET,
  '-DCMAKE_SYSTEM_VERSION=' + SYSTEM_VERSION
]

if CXX_STANDARD != '':
    cmake_cmd.extend(['-DCMAKE_CXX_STANDARD=' + CXX_STANDARD])

cmake_cmd.extend(get_common_cmake_parameters(DLL))

if vs_year >= 2019:
    cmake_cmd.append('-A')

    if ARCH == 'x64':
        cmake_cmd.append('x64')
    else:
        cmake_cmd.append('Win32')

cmake_cmd.append('../..')

call(cmake_cmd)

if BUILD_GRPC_CLIENT:
    build('grpc_cpp_plugin')
    build('protoc')

build('nakama-cpp')

if DLL:
    release_libs_path = os.path.abspath('../../release/nakama-cpp-sdk/shared-libs/' + arch_folder + '/' + TOOLSET + '/' + BUILD_MODE)
else:
    release_libs_path = os.path.abspath('../../release/nakama-cpp-sdk/libs/' + arch_folder + '/' + TOOLSET + '/' + BUILD_MODE)

makedirs(release_libs_path)
print('release_libs_path:', release_libs_path)

if DLL:
    copy_dll(release_libs_path)
else:
    copy_libs()

if BUILD_NAKAMA_TESTS:
    build('nakama-test')
