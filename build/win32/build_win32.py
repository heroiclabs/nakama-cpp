import sys
import subprocess
import os

build_dir = '.\\build'

if not os.path.isdir(build_dir):
    os.makedirs(build_dir)

def call(command):
    res = subprocess.call(command)
    if res != 0:
        sys.exit(-1)

def build(target):
    print 'building ' + target + '...'
    call('cmake --build ' + build_dir + ' --target ' + target + ' --config "Debug"')

# generate Visual Studio projects
#generator = 'Visual Studio 14 2015'
generator = 'Visual Studio 15 2017'
call('cmake -B ' + build_dir + ' -G"' + generator + '" ../..')

build('grpc_cpp_plugin')
build('protoc')
build('nakama-cpp')
build('nakama-test')

#print 'installing...'
#call('cmake --build ' + build_dir + ' --target install')
