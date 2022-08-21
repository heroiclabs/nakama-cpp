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

filename = '../build_config.py'
if sys.version_info[0] <= 2:
    execfile(filename)
else:
    exec(compile(open(filename, "rb").read(), filename, 'exec'))

modes_list    = ['Release', 'Debug']
arch_list     = ['x86', 'x64']
toolsets_list = ['v140', 'v141', 'v142', 'v143']

def call(command):
    res = subprocess.call(command, shell=False)
    if res != 0:
        sys.exit(-1)

if BUILD_NAKAMA_STATIC:
    # static libs
    for toolset in toolsets_list:
        for arch in arch_list:
            for mode in modes_list:
                call(['python', 'build_windows.py', '-a', arch, '-m', mode, '-t', toolset])

if BUILD_NAKAMA_SHARED:
    # DLLs
    for toolset in toolsets_list:
        for arch in arch_list:
            for mode in modes_list:
                call(['python', 'build_windows.py', '-a', arch, '-m', mode, '-t', toolset, '--dll'])

print('done.')
