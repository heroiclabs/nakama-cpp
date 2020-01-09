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
import shutil
import subprocess
import multiprocessing

USE_CPPREST = False
BUILD_MODE = 'Release'
BUILD_DIR = ''
TARGET_PLATFORM = '' # mac, ios, android, windows, linux

def init_common(build_common_path, target_platform):
    if build_common_path.find(' ') >= 0:
        print('Error: space foud in path:', build_common_path)
        print('please remove spaces from path and try again')
        sys.exit(-1)

    filename = os.path.join(build_common_path, 'build_config.py')
    if sys.version_info[0] <= 2:
        execfile(filename, globals())
    else:
        exec(compile(open(filename, "rb").read(), filename, 'exec'), globals())

    global USE_CPPREST, TARGET_PLATFORM
    USE_CPPREST = BUILD_HTTP_CPPREST or BUILD_WEBSOCKET_CPPREST
    TARGET_PLATFORM = target_platform

    print
    print('BUILD_REST_CLIENT =', str(BUILD_REST_CLIENT))
    print('BUILD_GRPC_CLIENT =', str(BUILD_GRPC_CLIENT))
    print('BUILD_HTTP_CPPREST =', str(BUILD_HTTP_CPPREST))
    print('BUILD_WEBSOCKET_CPPREST =', str(BUILD_WEBSOCKET_CPPREST))
    print('BUILD_C_API =', str(BUILD_C_API))
    print('BUILD_NAKAMA_TESTS =', str(BUILD_NAKAMA_TESTS))

    src_include = os.path.realpath(os.path.join(build_common_path, '..', 'include'))
    src_third_party = os.path.realpath(os.path.join(build_common_path, '..', 'third_party'))
    dest_include = os.path.realpath(os.path.join(build_common_path, '..', 'release', 'nakama-cpp-sdk', 'include'))
    copy_folder(os.path.join(src_include, 'nakama-cpp'), dest_include)
    copy_folder(os.path.join(src_third_party, 'nonstd'), dest_include)
    if BUILD_C_API:
        copy_folder(os.path.join(src_include, 'nakama-c'), dest_include)
        copy_folder(os.path.join(src_include, 'nakama-cpp-c-wrapper'), dest_include)

def get_common_cmake_parameters(is_shared):
    return [
      '-DCMAKE_BUILD_TYPE=' + BUILD_MODE,
      '-DNAKAMA_SHARED_LIBRARY=' + bool2cmake(is_shared),
      '-DBUILD_REST_CLIENT=' + bool2cmake(BUILD_REST_CLIENT),
      '-DBUILD_GRPC_CLIENT=' + bool2cmake(BUILD_GRPC_CLIENT),
      '-DBUILD_HTTP_CPPREST=' + bool2cmake(BUILD_HTTP_CPPREST),
      '-DBUILD_WEBSOCKET_CPPREST=' + bool2cmake(BUILD_WEBSOCKET_CPPREST),
      '-DBUILD_C_API=' + bool2cmake(BUILD_C_API)
    ]

def call(command, shell=False):
    print('calling:', str(command))
    res = subprocess.call(command, shell=shell)
    if res != 0:
        sys.exit(-1)

def is_windows():
    import platform
    return platform.system() == 'Windows'

def set_build_folder_name(name):
    global BUILD_DIR
    BUILD_DIR = os.path.abspath(os.path.join('build', name))
    makedirs(BUILD_DIR)

def build(target):
    print()
    print('building ' + target + '...')
    call(['cmake',
        '--build', BUILD_DIR,
        '--target', target,
        '--config', BUILD_MODE,
        '--parallel', str(multiprocessing.cpu_count())
    ], shell=False)

def makedirs(dir):
    if not os.path.isdir(dir):
        os.makedirs(dir)

def mklink(link, target):
    if not os.path.exists(link):
        if is_windows():
            call(['mklink', link, target], shell=True)
        else:
            call(['ln', '-s', target, link], shell=False)

def getEnvVar(name):
    if name in os.environ:
        return os.environ[name]
    return ''

def bool2cmake(bVal):
    if bVal:
        return 'ON'
    else:
        return 'OFF'

def copy_file(src, dest):
    shutil.copy(src, dest)
    print('copied', os.path.basename(src))

def copy_folder(src, dest, replace=True):
    folder_name = os.path.basename(src)
    dest_full_path = os.path.join(dest, folder_name)
    if replace:
        if os.path.exists(dest_full_path):
            shutil.rmtree(dest_full_path)

    shutil.copytree(src, dest_full_path)
    print('copied', os.path.basename(src))

def copy_libs():
    print
    print('copying to release folder...')

    copy_nakama_lib()

    if BUILD_REST_CLIENT or BUILD_GRPC_CLIENT:
        copy_protobuf_lib()

    if BUILD_GRPC_CLIENT or USE_CPPREST:
        copy_ssl_lib()
    
    if BUILD_GRPC_CLIENT:
        copy_grpc_lib()

    if USE_CPPREST:
        copy_rest_lib()

def set_install_name(dylib_path):
    if (IOS_RPATH_ENABLE and TARGET_PLATFORM == 'ios') or (MAC_RPATH_ENABLE and TARGET_PLATFORM == 'mac'):
        path = '@rpath'
    else:
        path = '@executable_path'
    call(['install_name_tool', '-id', path + '/libnakama-cpp.dylib', dylib_path])
