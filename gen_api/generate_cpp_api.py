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
import subprocess
import argparse
import platform

def getEnvVar(name):
    if name in os.environ:
        return os.environ[name]
    return ''

parser = argparse.ArgumentParser(description='Nakama C++ API generator')
parser.add_argument('-n', '--nakama', help='Nakama server sources')
parser.add_argument('-g', '--gateway', help='grpc-gateway sources')

args = parser.parse_args()

def getArgOrEnvVar(env_var_name, arg_value):
    if arg_value:
        value = arg_value
    else:
        value = getEnvVar(env_var_name)
        if not value:
            print('Error: missing "' + env_var_name + '" env variable')
            sys.exit(-1)

    return value

# https://github.com/heroiclabs/nakama
NAKAMA = getArgOrEnvVar('NAKAMA', args.nakama)

# https://github.com/grpc-ecosystem/grpc-gateway
GRPC_GATEWAY = getArgOrEnvVar('GRPC_GATEWAY', args.gateway)

def path(p):
    return os.path.normpath(p)

NAKAMA_CPP      = os.path.abspath('./..')
GRPC            = path(NAKAMA_CPP + '/third_party/grpc')
GOOGLEAPIS      = path(GRPC_GATEWAY + '/third_party/googleapis')
PROTOBUF_SRC    = path(GRPC + '/third_party/protobuf/src')
OUT             = os.path.abspath('cppout')

is_windows = platform.system() == 'Windows'
is_mac     = platform.system() == 'Darwin'

if is_windows:
    build_dir = NAKAMA_CPP + '/build/windows/build/v142_x86'
elif is_mac:
    build_dir = NAKAMA_CPP + '/build/mac/build'
else:
    # linux
    build_dir = NAKAMA_CPP + '/build/linux/build'

def find_grpc_cpp_plugin():
    if is_windows:
        grpc_cpp_plugin = path(build_dir + '/third_party/grpc/Debug/grpc_cpp_plugin.exe')
        if not os.path.exists(grpc_cpp_plugin):
            grpc_cpp_plugin = path(build_dir + '/third_party/grpc/Release/grpc_cpp_plugin.exe')
    else:
        grpc_cpp_plugin = path(build_dir + '/Release/third_party/grpc/grpc_cpp_plugin')
        if not os.path.exists(grpc_cpp_plugin):
            grpc_cpp_plugin = path(NAKAMA_CPP + 'Debug/third_party/grpc/grpc_cpp_plugin')

    if not os.path.exists(grpc_cpp_plugin):
        print('grpc_cpp_plugin not found')
        print('Please build for desktop OS first')
        sys.exit(-1)
    
    return grpc_cpp_plugin

def find_protoc():
    if is_windows:
        protoc = path(build_dir + '/third_party/grpc/third_party/protobuf/Debug/protoc.exe')
        if not os.path.exists(protoc):
            protoc = path(build_dir + '/third_party/grpc/third_party/protobuf/Release/protoc.exe')
    else:
        protoc = path(build_dir + '/Release/third_party/grpc/third_party/protobuf/protoc')
        if not os.path.exists(protoc):
            protoc = path(build_dir + '/Debug/third_party/grpc/third_party/protobuf/protoc')

    if not os.path.exists(protoc):
        print('protoc not found')
        print('Please build for desktop OS first')
        sys.exit(-1)
    
    return protoc

def call(commands, shell=False):
    #print('call', str(commands))
    res = subprocess.call(commands, shell=shell)
    if res != 0:
        sys.exit(-1)

def check_required_folder(folder):
    if not os.path.exists(folder):
        print('ERROR: not exist', folder)
        sys.exit(-1)

def makedirs(path):
    if not os.path.exists(path):
        os.makedirs(path)

def mklink(link, target):
    if not os.path.exists(link):
        if is_windows:
            call(['mklink', link, target], shell=True)
        else:
            call(['ln', '-s', target, link], shell=False)

GRPC_CPP_PLUGIN = find_grpc_cpp_plugin()
PROTOC          = find_protoc()

check_required_folder(NAKAMA)
check_required_folder(GRPC)
check_required_folder(GRPC_GATEWAY)
check_required_folder(GOOGLEAPIS)
check_required_folder(GRPC_CPP_PLUGIN)
check_required_folder(PROTOC)
check_required_folder(PROTOBUF_SRC)

CUR_DIR = os.path.abspath('.')

makedirs(OUT)
makedirs(path(OUT + '/google/api'))
makedirs(path(OUT + '/google/rpc'))

makedirs(path(CUR_DIR + '/github.com/heroiclabs/nakama-common/api'))
makedirs(path(CUR_DIR + '/github.com/heroiclabs/nakama/apigrpc'))
makedirs(path(CUR_DIR + '/github.com/heroiclabs/nakama-common/rtapi'))
mklink(path(CUR_DIR + '/github.com/heroiclabs/nakama-common/api/api.proto'), path(NAKAMA + '/vendor/github.com/heroiclabs/nakama-common/api/api.proto'))
mklink(path(CUR_DIR + '/github.com/heroiclabs/nakama/apigrpc/apigrpc.proto'), path(NAKAMA + '/apigrpc/apigrpc.proto'))
mklink(path(CUR_DIR + '/github.com/heroiclabs/nakama-common/rtapi/realtime.proto'), path(NAKAMA + '/vendor/github.com/heroiclabs/nakama-common/rtapi/realtime.proto'))

print('generating apigrpc')

call([PROTOC, '-I.', '-I' + GRPC_GATEWAY, '-I' + GOOGLEAPIS, '-I' + PROTOBUF_SRC, '--grpc_out=' + OUT, '--plugin=protoc-gen-grpc=' + GRPC_CPP_PLUGIN, path('github.com/heroiclabs/nakama/apigrpc/apigrpc.proto')])
call([PROTOC, '-I.', '-I' + GRPC_GATEWAY, '-I' + GOOGLEAPIS, '-I' + PROTOBUF_SRC, '--cpp_out=' + OUT, path('github.com/heroiclabs/nakama/apigrpc/apigrpc.proto')])
call([PROTOC, '-I.', '-I' + GRPC_GATEWAY, '-I' + GOOGLEAPIS, '-I' + PROTOBUF_SRC, '--cpp_out=' + OUT, path('github.com/heroiclabs/nakama-common/api/api.proto')])

os.chdir(path(GOOGLEAPIS + '/google/rpc'))

call([PROTOC, '-I.', '-I' + GRPC_GATEWAY, '-I' + GOOGLEAPIS, '-I' + PROTOBUF_SRC, '--cpp_out=' + path(OUT + '/google/rpc'), 'status.proto'])

os.chdir(path(GOOGLEAPIS + '/google/api'))

call([PROTOC, '-I.', '-I' + GRPC_GATEWAY, '-I' + GOOGLEAPIS, '-I' + PROTOBUF_SRC, '--cpp_out=' + path(OUT + '/google/api'), 'annotations.proto'])
call([PROTOC, '-I.', '-I' + GRPC_GATEWAY, '-I' + GOOGLEAPIS, '-I' + PROTOBUF_SRC, '--cpp_out=' + path(OUT + '/google/api'), 'http.proto'])

os.chdir(CUR_DIR)

call([PROTOC, '-I.', '-I' + GRPC_GATEWAY, '-I' + GOOGLEAPIS, '-I' + PROTOBUF_SRC, '--cpp_out=' + OUT, path(GRPC_GATEWAY + '/protoc-gen-swagger/options/annotations.proto')])
call([PROTOC, '-I.', '-I' + GRPC_GATEWAY, '-I' + GOOGLEAPIS, '-I' + PROTOBUF_SRC, '--cpp_out=' + OUT, path(GRPC_GATEWAY + '/protoc-gen-swagger/options/openapiv2.proto')])

print('generating rtapi')

call([PROTOC, '-I.', '-I' + PROTOBUF_SRC, '--cpp_out=' + OUT, path('github.com/heroiclabs/nakama-common/rtapi/realtime.proto')])

print('done.')
