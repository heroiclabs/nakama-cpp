#!/usr/bin/env python
import sys
import subprocess
import os

if len(sys.argv) < 2:
    print "Pass ABI parameter."
    print "e.g. armeabi-v7a or x86"
    sys.exit(-1)

ABI = sys.argv[1]

ANDROID_NDK = os.environ['ANDROID_NDK']
if not ANDROID_NDK:
    ANDROID_NDK = os.environ['NDK_ROOT']
    if not ANDROID_NDK:
        print "Error: no ANDROID_NDK or NDK_ROOT environment variable"
        sys.exit(-1)

def call(command):
    res = subprocess.call(command)
    if res != 0:
        sys.exit(-1)

build_dir = './build/' + ABI

if not os.path.isdir(build_dir):
    os.makedirs(build_dir)

print 'ANDROID_NDK=' + ANDROID_NDK

call('cmake -DANDROID_ABI=' + ABI +
 ' -DCMAKE_TOOLCHAIN_FILE=' + ANDROID_NDK + '/build/cmake/android.toolchain.cmake' +
 ' -DANDROID_NATIVE_API_LEVEL=16 -B ' + build_dir + ' -GNinja ../..')

call('ninja -C ' + build_dir + ' nakama-cpp')

call('cmake --build ' + build_dir + ' --target install')
