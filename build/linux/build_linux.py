#!/usr/bin/env python
import sys
import subprocess
import os

BUILD_MODE = 'Release'
build_dir = './build/' + BUILD_MODE

if not os.path.isdir(build_dir):
    os.makedirs(build_dir)

def call(command, shell=False):
    res = subprocess.call(command, shell=shell)
    if res != 0:
        sys.exit(-1)

def build(target):
    print 'building ' + target + '...'
    call('cmake --build . --target ' + target, shell=True)

os.chdir(build_dir)

# generate projects
call([
 'cmake',
 '-DCMAKE_BUILD_TYPE=' + BUILD_MODE,
 '../../../..'
])

build('grpc_cpp_plugin')
build('protoc')
build('nakama-cpp')
build('nakama-test')
