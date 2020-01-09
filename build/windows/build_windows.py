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
parser.add_argument('-t', '--tool', help='platform toolset: v140, v141...')
parser.add_argument(      '--dll',  help='build DLL', action='store_true')

args = parser.parse_args()

BUILD_MODE = 'Debug'
ARCH = 'x86'
TOOLSET = args.tool
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

if BUILD_MODE == 'Debug':
    libs_postfix = 'd'
else:
    libs_postfix = ''

def copy_nakama_lib():
    copy_file(BUILD_DIR + '\\src\\' + BUILD_MODE + '\\nakama-cpp' + libs_postfix + '.lib', release_libs_path)

def copy_protobuf_lib():
    copy_file(BUILD_DIR + '\\third_party\\grpc\\third_party\\protobuf\\' + BUILD_MODE + '\\libprotobuf' + libs_postfix + '.lib', release_libs_path)

def copy_ssl_lib():
    copy_file(BUILD_DIR + '\\third_party\\grpc\\third_party\\boringssl\\ssl\\' + BUILD_MODE + '\\ssl' + libs_postfix + '.lib', release_libs_path)
    copy_file(BUILD_DIR + '\\third_party\\grpc\\third_party\\boringssl\\crypto\\' + BUILD_MODE + '\\crypto' + libs_postfix + '.lib', release_libs_path)

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
#generator, vs_year, default_toolset = 'Visual Studio 14 2015', 2015, 'v140'
#generator, vs_year, default_toolset = 'Visual Studio 15 2017', 2017, 'v141'
generator, vs_year, default_toolset = 'Visual Studio 16 2019', 2019, 'v142'

if ARCH == 'x64' and vs_year < 2019:
    generator += ' Win64'

if not TOOLSET:
    TOOLSET = default_toolset

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
SYSTEM_VERSION = '6.1'

cmake_cmd = ['cmake',
  '-B', BUILD_DIR,
  '-G', generator,
  '-T', TOOLSET,
  '-DCMAKE_SYSTEM_VERSION=' + SYSTEM_VERSION
]

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
