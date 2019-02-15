import sys
import os
import subprocess
import argparse
import shutil

parser = argparse.ArgumentParser(description='builder for Windows')
parser.add_argument('-m', '--mode', help='build mode: Debug or Release')
parser.add_argument('-a', '--arch', help='architecture: x86 or x64')

args = parser.parse_args()

BUILD_MODE = 'Debug'
ARCH = 'x86'

if args.mode:
    valid_modes = ['Debug', 'Release']
    if args.mode in valid_modes:
        BUILD_MODE = args.mode
    else:
        print 'Not valid mode. Supported values:', str(valid_modes)
        sys.exit(-1)

if args.arch:
    valid_archs = ['x86', 'x64']
    if args.arch in valid_archs:
        ARCH = args.arch
    else:
        print 'Not valid architecture. Supported values:', str(valid_archs)
        sys.exit(-1)

build_dir = os.path.abspath('build\\' + ARCH)

if BUILD_MODE == 'Debug':
    libs_postfix = 'd'
else:
    libs_postfix = ''

def makedirs(dir):
    if not os.path.isdir(dir):
        os.makedirs(dir)

def call(command):
    res = subprocess.call(command)
    if res != 0:
        sys.exit(-1)

def build(target):
    print 'building ' + target + ' for ' + BUILD_MODE + '...'
    call('cmake --build ' + build_dir + ' --target ' + target + ' --config ' + BUILD_MODE)

def copy_file(src, dest):
    shutil.copy(src, dest)
    print 'copied', os.path.basename(src)

def copy_libs(dest):
    print
    print 'copying to release folder...'
    copy_file(build_dir + '\\src\\' + BUILD_MODE + '\\nakama-cpp' + libs_postfix + '.lib', dest)

# generate Visual Studio projects
#generator = 'Visual Studio 14 2015'
generator = 'Visual Studio 15 2017'

if ARCH == 'x64':
    generator += ' Win64'

makedirs(build_dir)

call('cmake -B ' + build_dir +
 ' -G"' + generator + '"' +
 ' ../..')

build('grpc_cpp_plugin')
build('protoc')
build('nakama-cpp')
build('nakama-test')

if generator.startswith('Visual Studio 14 2015'):
    vc = 'vc140'
elif generator.startswith('Visual Studio 15 2017'):
    vc = 'vc141'
else:
    print 'Unknown Visual Studio version.'
    sys.exit(-1)

if ARCH == 'x64':
    win = 'win64'
else:
    win = 'win32'

release_libs_dir = os.path.abspath('../../release/nakama-cpp-sdk/libs/' + win + '/' + vc)

makedirs(release_libs_dir)
copy_libs(release_libs_dir)
