#!/usr/bin/env python
import sys
import subprocess
import os
import shutil
import platform

bits, linkage = platform.architecture()

if bits == '64bit':
	ARCH = 'x64'
elif bits == '32bit':
	ARCH = 'x86'
else:
	ARCH = bits

BUILD_MODE = 'Release'
build_dir = os.path.abspath('build/' + BUILD_MODE + '_' + ARCH)
release_libs_dir = os.path.abspath('../../release/nakama-cpp-sdk/libs/linux/' + ARCH)

print 'Architecture:', ARCH
print 'Build mode  :', BUILD_MODE

def makedirs(dir):
	if not os.path.isdir(dir):
		os.makedirs(dir)

def call(command, shell=False):
    res = subprocess.call(command, shell=shell)
    if res != 0:
        sys.exit(-1)

def build(target):
    print 'building ' + target + '...'
    call('cmake --build . --target ' + target, shell=True)

def copy_file(src, dest):
    shutil.copy(src, dest)
    print 'copied', os.path.basename(src)

def copy_libs(dest):
    print
    print 'copying to release folder...'
    copy_file(build_dir + '/src/libnakama-cpp.a', dest)

makedirs(build_dir)
makedirs(release_libs_dir)

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

copy_libs(release_libs_dir)
