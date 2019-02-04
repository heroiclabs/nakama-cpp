#!/usr/bin/env python
import sys
import subprocess
import os

BUILD_MODE = 'Release'
build_dir = './build/' + BUILD_MODE

if not os.path.isdir(build_dir):
    os.makedirs(build_dir)

def call(command):
    res = subprocess.call(command, shell=True)
    if res != 0:
        sys.exit(-1)

def build(target):
    print 'building ' + target + '...'
    call('cmake --build . --target ' + target + ' --config ' + BUILD_MODE)

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
