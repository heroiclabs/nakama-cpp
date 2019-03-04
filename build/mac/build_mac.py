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
import subprocess
import os
import shutil

cur_dir = os.path.abspath('.')
if cur_dir.find(' ') >= 0:
    print 'Error: space foud in path:', cur_dir
    print 'please remove spaces from path and try again'
    sys.exit(-1)

BUILD_MODE = 'Release'
build_dir = os.path.abspath('build/' + BUILD_MODE)
release_libs_path = os.path.abspath('../../release/nakama-cpp-sdk/libs/mac')

if not os.path.isdir(build_dir):
    os.makedirs(build_dir)

def call(command):
    res = subprocess.call(command, shell=True)
    if res != 0:
        sys.exit(-1)

def build(target):
    print 'building ' + target + '...'
    call('cmake --build . --target ' + target + ' --config ' + BUILD_MODE)

def makedirs(dir):
    if not os.path.isdir(dir):
        os.makedirs(dir)

def copy_file(src, dest):
    shutil.copy(src, dest)
    print 'copied', os.path.basename(src)

def copy_libs(dest):
    print
    print 'copying to release folder...'
    copy_file(build_dir + '/src/libnakama-cpp.a', dest)

os.chdir(build_dir)

#generator = 'Xcode' # doesn't build crypto
generator = 'Ninja'

# generate projects
call('cmake' +
 ' -DENABLE_BITCODE=FALSE' +
 ' -DENABLE_ARC=TRUE' +
 ' -G' + generator +
 ' ../../../..')

build('grpc_cpp_plugin')
build('protoc')
build('nakama-cpp')
build('nakama-test')

makedirs(release_libs_path)
copy_libs(release_libs_path)
