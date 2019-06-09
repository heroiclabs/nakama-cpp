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

execfile('../build_config.py')

def call(command):
    res = subprocess.call(command, shell=False)
    if res != 0:
        sys.exit(-1)

if BUILD_NAKAMA_STATIC:
    call(['python', 'build_linux.py'])

if BUILD_NAKAMA_SHARED:
    call(['python', 'build_linux.py', '--so'])
