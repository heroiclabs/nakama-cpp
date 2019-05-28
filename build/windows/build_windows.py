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
import subprocess
import argparse
import shutil

cur_dir = os.path.abspath('.')
if cur_dir.find(' ') >= 0:
    print 'Error: space foud in path:', cur_dir
    print 'please remove spaces from path and try again'
    sys.exit(-1)

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
        print 'Not valid mode. Supported values:', str(valid_modes)
        sys.exit(-1)

if args.arch:
    valid_archs = ['x86', 'x64']
    if args.arch in valid_archs:
        ARCH = args.arch
    else:
        print 'Not valid architecture. Supported values:', str(valid_archs)
        sys.exit(-1)

if BUILD_MODE == 'Debug':
    libs_postfix = 'd'
else:
    libs_postfix = ''

def makedirs(dir):
    if not os.path.isdir(dir):
        os.makedirs(dir)

def call(command):
    res = subprocess.call(command)
    if res != 0:
        sys.exit(-1)

def build(target):
    print 'building ' + target + ' for ' + BUILD_MODE + '...'
    call('cmake --build ' + build_dir + ' --target ' + target + ' --config ' + BUILD_MODE)

def copy_file(src, dest):
    shutil.copy(src, dest)
    print 'copied', os.path.basename(src)

def copy_libs(dest):
    print
    print 'copying to release folder...'
    copy_file(build_dir + '\\src\\' + BUILD_MODE + '\\nakama-cpp' + libs_postfix + '.lib', dest)
    copy_file(build_dir + '\\third_party\\grpc\\' + BUILD_MODE + '\\address_sorting' + libs_postfix + '.lib', dest)
    copy_file(build_dir + '\\third_party\\grpc\\' + BUILD_MODE + '\\gpr' + libs_postfix + '.lib', dest)
    copy_file(build_dir + '\\third_party\\grpc\\' + BUILD_MODE + '\\grpc++' + libs_postfix + '.lib', dest)
    copy_file(build_dir + '\\third_party\\grpc\\' + BUILD_MODE + '\\grpc' + libs_postfix + '.lib', dest)
    copy_file(build_dir + '\\third_party\\grpc\\third_party\\cares\\cares\\lib\\' + BUILD_MODE + '\\cares' + libs_postfix + '.lib', dest)
    copy_file(build_dir + '\\third_party\\grpc\\third_party\\protobuf\\' + BUILD_MODE + '\\libprotobuf' + libs_postfix + '.lib', dest)
    copy_file(build_dir + '\\third_party\\grpc\\third_party\\boringssl\\ssl\\' + BUILD_MODE + '\\ssl' + libs_postfix + '.lib', dest)
    copy_file(build_dir + '\\third_party\\grpc\\third_party\\boringssl\\crypto\\' + BUILD_MODE + '\\crypto' + libs_postfix + '.lib', dest)
    copy_file(build_dir + '\\third_party\\grpc\\third_party\\zlib\\' + BUILD_MODE + '\\zlibstatic' + libs_postfix + '.lib', dest + '\\zlib' + libs_postfix + '.lib')

def copy_dll(dest):
    print
    print 'copying to release folder...'
    copy_file(build_dir + '\\src\\' + BUILD_MODE + '\\nakama-cpp' + libs_postfix + '.lib', dest)
    copy_file(build_dir + '\\src\\' + BUILD_MODE + '\\nakama-cpp' + libs_postfix + '.dll', dest)

# generate Visual Studio projects
#generator, vs_year, default_toolset = 'Visual Studio 14 2015', 2015, 'v140'
#generator, vs_year, default_toolset = 'Visual Studio 15 2017', 2017, 'v141'
generator, vs_year, default_toolset = 'Visual Studio 16 2019', 2019, 'v142'

if ARCH == 'x64' and vs_year < 2019:
    generator += ' Win64'

if not TOOLSET:
    TOOLSET = default_toolset

if DLL:
    NAKAMA_SHARED_LIBRARY = 'TRUE'
else:
    NAKAMA_SHARED_LIBRARY = 'FALSE'

build_dir = os.path.abspath('build\\' + TOOLSET + '_' + ARCH)
makedirs(build_dir)

print
print 'Building for Arch:', ARCH + ', Toolset:', TOOLSET + ', Mode:', BUILD_MODE + ', DLL:', str(DLL)
print

if ARCH == 'x64':
    arch_folder = 'win64'
else:
    arch_folder = 'win32'

cmake_cmd = ['cmake',
 '-B', build_dir,
 '-G', generator,
 '-T', TOOLSET,
 '-DCMAKE_BUILD_TYPE=' + BUILD_MODE,
 '-DNAKAMA_SHARED_LIBRARY=' + NAKAMA_SHARED_LIBRARY,
 '-DBUILD_WEBSOCKETPP=ON'
]

if vs_year >= 2019:
    cmake_cmd.append('-A')
    
    if ARCH == 'x64':
        cmake_cmd.append('x64')
    else:
        cmake_cmd.append('Win32')

cmake_cmd.append('../..')

call(cmake_cmd)

build('grpc_cpp_plugin')
build('protoc')
build('nakama-cpp')

if DLL:
    release_libs_dir = os.path.abspath('../../release/nakama-cpp-sdk/shared-libs/' + arch_folder + '/' + TOOLSET + '/' + BUILD_MODE)
else:
    release_libs_dir = os.path.abspath('../../release/nakama-cpp-sdk/libs/' + arch_folder + '/' + TOOLSET + '/' + BUILD_MODE)

makedirs(release_libs_dir)
print 'release_libs_dir:', release_libs_dir

if DLL:
    copy_dll(release_libs_dir)
else:
    copy_libs(release_libs_dir)

build('nakama-test')
