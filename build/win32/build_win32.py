import sys
import subprocess
import os
import argparse

parser = argparse.ArgumentParser(description='builder for Windows')
parser.add_argument('-m', '--mode', help='build mode: Debug or Release')

args = parser.parse_args()

BUILD_MODE = 'Debug'

if args.mode:
    valid_modes = ['Debug', 'Release']
    if args.mode in valid_modes:
        BUILD_MODE = args.mode
    else:
        print 'Not valid mode. Supported values:', str(valid_modes)
        sys.exit(-1)

build_dir = '.\\build'

if not os.path.isdir(build_dir):
    os.makedirs(build_dir)

def call(command):
    res = subprocess.call(command)
    if res != 0:
        sys.exit(-1)

def build(target):
    print 'building ' + target + ' for ' + BUILD_MODE + '...'
    call('cmake --build ' + build_dir + ' --target ' + target + ' --config ' + BUILD_MODE)

grpc_cpp_plugin_path = os.path.abspath(r'build\third_party\grpc\\' + BUILD_MODE + '\\grpc_cpp_plugin.exe')
protoc_path = os.path.abspath(r'build\third_party\grpc\third_party\protobuf\\' + BUILD_MODE + '\\protoc.exe')

# generate Visual Studio projects
#generator = 'Visual Studio 14 2015'
generator = 'Visual Studio 15 2017'

call('cmake -B ' + build_dir +
 ' -G"' + generator + '"' +
 ' -DPROTOBUF_PROTOC_EXECUTABLE=' + protoc_path +
 ' -DGRPC_CPP_PLUGIN_EXECUTABLE=' + grpc_cpp_plugin_path +
 ' ../..')

build('grpc_cpp_plugin')
build('protoc')
build('nakama-cpp')
build('nakama-test')

#print 'installing...'
#call('cmake --build ' + build_dir + ' --target install')
