#!/usr/bin/env python
import sys
import subprocess
import os

if len(sys.argv) < 2:
    print "Pass ABI parameter."
    print "e.g. armeabi-v7a or x86"
    sys.exit(-1)

ABI = sys.argv[1]

if os.name == 'nt':
    # windows
    grpc_cpp_plugin_path = r'..\win32\build\third_party\grpc\Debug\grpc_cpp_plugin.exe'
    protoc_path = r'..\win32\build\third_party\grpc\third_party\protobuf\Debug\protoc.exe'

    if not os.path.isfile(grpc_cpp_plugin_path) or not os.path.isfile(protoc_path):
        print 'grpc_cpp_plugin_path =', grpc_cpp_plugin_path
        print 'protoc_path =', protoc_path
        print "please build for windows first"
        sys.exit(-1)

else:
    print "currently not supported OS"
    sys.exit(-1)

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
