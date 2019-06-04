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

import os
import sys
import shutil
import subprocess

USE_CPPREST = False

def init_common(build_common_path):
    if build_common_path.find(' ') >= 0:
        print 'Error: space foud in path:', build_common_path
        print 'please remove spaces from path and try again'
        sys.exit(-1)

    execfile(os.path.join(build_common_path, 'build_config.py'), globals())

    global USE_CPPREST
    USE_CPPREST = BUILD_HTTP_CPPREST or BUILD_WEBSOCKET_CPPREST

    print
    print 'BUILD_REST_CLIENT =', str(BUILD_REST_CLIENT)
    print 'BUILD_GRPC_CLIENT =', str(BUILD_GRPC_CLIENT)
    print 'BUILD_HTTP_CPPREST =', str(BUILD_HTTP_CPPREST)
    print 'BUILD_WEBSOCKET_CPPREST =', str(BUILD_WEBSOCKET_CPPREST)

def call(command, shell=False):
    print 'calling:', str(command)
    res = subprocess.call(command, shell=shell)
    if res != 0:
        sys.exit(-1)

def is_windows():
    import platform
    return platform.system() == 'Windows'

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
    print 'copied', os.path.basename(src)

def copy_libs():
    print
    print 'copying to release folder...'

    copy_nakama_lib()

    if BUILD_REST_CLIENT or BUILD_GRPC_CLIENT:
        copy_protobuf_lib()

    if BUILD_GRPC_CLIENT or USE_CPPREST:
        copy_ssl_lib()
    
    if BUILD_GRPC_CLIENT:
        copy_grpc_lib()

    if USE_CPPREST:
        copy_rest_lib()
