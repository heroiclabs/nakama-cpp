#!/usr/bin/env python
import sys
import subprocess
import os

if len(sys.argv) < 2:
    print "Pass ABI parameter."
    print "e.g. armeabi-v7a, arm64-v8a or x86"
    sys.exit(-1)

ABI = sys.argv[1]
BUILD_MODE = 'Release'

def getEnvVar(name):
    if name in os.environ:
        return os.environ[name]
    return ''

ANDROID_NDK = getEnvVar('ANDROID_NDK')
if not ANDROID_NDK:
    ANDROID_NDK = getEnvVar('NDK_ROOT')
    if not ANDROID_NDK:
        print "Error: no ANDROID_NDK or NDK_ROOT environment variable"
        sys.exit(-1)

def call(command):
    res = subprocess.call(command, shell=False)
    if res != 0:
        sys.exit(-1)

build_dir = './build/' + ABI + '/' + BUILD_MODE

if not os.path.isdir(build_dir):
    os.makedirs(build_dir)

print 'ANDROID_NDK=' + ANDROID_NDK

cmake_args = [
              'cmake',
              '-DANDROID_ABI=' + ABI,
              '-DCMAKE_TOOLCHAIN_FILE=' + ANDROID_NDK + '/build/cmake/android.toolchain.cmake',
              '-DBUILD_DEFAULT_WEBSOCKETS=OFF',
              '-DCMAKE_BUILD_TYPE=' + BUILD_MODE,
              '-DANDROID_NATIVE_API_LEVEL=16',
              '-B',
              build_dir,
              '-GNinja',
              '../..'
              ]

# generate projects
call(cmake_args)

# build
call(['ninja', '-C', build_dir, 'nakama-cpp'])
