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
import shutil

_7zip_path = 'c:\\Program Files\\7-Zip\\7z.exe'
sdk_path = './nakama-cpp-sdk'
tmp_path = './_tmp'
platforms = ['win32', 'win64', 'mac', 'ios', 'android', 'linux']
libs_path = os.path.join(sdk_path, 'libs')
shared_libs_path = os.path.join(sdk_path, 'shared-libs')
tmp_libs_path = os.path.join(tmp_path, 'libs')
tmp_shared_libs_path = os.path.join(tmp_path, 'shared-libs')
version = 'unknown'

def call(command, shell=False):
    print('calling:', str(command))
    res = subprocess.call(command, shell=shell)
    if res != 0:
        sys.exit(-1)

def archive7zip(src_folder, dest_arch, ignore_list=None):
    command = [_7zip_path, 'a', '-r', dest_arch, src_folder]
    if ignore_list:
        for ignore_item in ignore_list:
            command.append('-xr!' + ignore_item)
    call(command)

def move_folder(src, dest):
    if os.path.exists(src):
        shutil.move(src, dest)

def move_platform_to_temp(platform):
    src = os.path.join(libs_path, platform)
    dest = os.path.join(tmp_libs_path, platform)
    move_folder(src, dest)

    src = os.path.join(shared_libs_path, platform)
    dest = os.path.join(tmp_shared_libs_path, platform)
    move_folder(src, dest)

def move_platform_to_sdk(platform):
    src = os.path.join(tmp_libs_path, platform)
    dest = os.path.join(libs_path, platform)
    move_folder(src, dest)

    src = os.path.join(tmp_shared_libs_path, platform)
    dest = os.path.join(shared_libs_path, platform)
    move_folder(src, dest)

def move_all_to_temp():
    print('moving to', tmp_path)
    for platform in platforms:
        move_platform_to_temp(platform)

def release_platform(platform):
    print('releasing', platform)
    move_platform_to_sdk(platform)
    ignore_list = None
    if platform != 'android':
        ignore_list = ['nakama-cpp-android']
    out_arch = 'nakama-cpp-sdk_{version}_{platform}.7z'.format(version=version, platform=platform)
    os.remove(out_arch)
    archive7zip(sdk_path, out_arch, ignore_list)

def detect_sdk_version():
    with open('../src/Nakama.cpp', 'r') as f:
        while True:
            line = f.readline()
            if not line:
                break
            line = line.lstrip()
            if line.startswith('return'):
                pos1 = line.find('"') + 1
                pos2 = line.find('"', pos1)
                return line[pos1:pos2]

version = detect_sdk_version()

print('releasing sdk version:', version)

for platform in platforms:
    move_all_to_temp()
    release_platform(platform)

for platform in platforms:
    move_platform_to_sdk(platform)

print('done.')
