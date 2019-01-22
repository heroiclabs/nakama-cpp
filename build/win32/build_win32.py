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

grpc_cpp_plugin_path = os.path.abspath(r'..\win32\build\third_party\grpc\Debug\grpc_cpp_plugin.exe')
protoc_path = os.path.abspath(r'..\win32\build\third_party\grpc\third_party\protobuf\Debug\protoc.exe')

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
