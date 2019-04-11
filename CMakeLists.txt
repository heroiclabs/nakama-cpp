#
# Copyright 2019 The Nakama Authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

cmake_minimum_required(VERSION 3.10)

project(nakama-cpp-all)

cmake_policy(SET CMP0074 NEW)
cmake_policy(SET CMP0054 NEW)
cmake_policy(SET CMP0042 NEW)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)

include(windows_support)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

option(BUILD_WEBSOCKETPP "Build Websocketpp transport" ON)
option(BUILD_IXWEBSOCKET "Build IXWebSocket transport" ON)
option(LOGS_ENABLED "enable logs" ON)
option(NAKAMA_SSL_ENABLED "enable SSL" ON)

set(gRPC_BUILD_CSHARP_EXT OFF)
set(protobuf_BUILD_TESTS OFF)
set(BENCHMARK_ENABLE_LTO:BOOL OFF)
set(BENCHMARK_ENABLE_TESTING:BOOL OFF)
set(BUILD_STATIC_LIBS ON)
set(BUILD_SHARED_LIBS OFF)
set(RUN_HAVE_STD_REGEX 0)
set(RUN_HAVE_POSIX_REGEX 0)
set(RUN_HAVE_GNU_POSIX_REGEX 0)
set(RUN_HAVE_STEADY_CLOCK 0)

set(INSTALL_LIB_DIR ${PROJECT_SOURCE_DIR}/lib)
set(CMAKE_INSTALL_PREFIX ${INSTALL_LIB_DIR})
set(PUBLIC_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include)
set(THIRD_PARTY_DIR ${PROJECT_SOURCE_DIR}/third_party)

add_subdirectory(src)
add_subdirectory(third_party/grpc)
add_subdirectory(third_party/IXWebSocket)

#if((WIN32 OR APPLE OR UNIX) AND NOT IOS_PLATFORM)
# only build tests for desktop OS
if(NOT CMAKE_CROSSCOMPILING)
    add_subdirectory(test)
endif()
