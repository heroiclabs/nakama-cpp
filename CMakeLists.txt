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

cmake_minimum_required(VERSION 3.15)

project(nakama-cpp)

cmake_policy(SET CMP0042 NEW)
cmake_policy(SET CMP0048 NEW)
cmake_policy(SET CMP0054 NEW)
cmake_policy(SET CMP0063 NEW)
cmake_policy(SET CMP0074 NEW)
cmake_policy(SET CMP0077 NEW)
cmake_policy(SET CMP0091 NEW)

#set(CMAKE_C_VISIBILITY_PRESET hidden)
#set(CMAKE_CXX_VISIBILITY_PRESET hidden)
#set(CMAKE_VISIBILITY_INLINES_HIDDEN TRUE)

if(NOT WIN32)
    add_compile_options(-fvisibility=hidden -fPIC)
endif()

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)

include(windows_support)

option(BUILD_REST_CLIENT "Build REST client" ON)
option(BUILD_HTTP_CPPREST "Build HTTP transport using cpprest SDK" ON)
option(BUILD_WEBSOCKET_CPPREST "Build Websocket transport using cpprest SDK" ON)
option(BUILD_GRPC_CLIENT "Build gRPC client" OFF)
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

set(PUBLIC_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include)
set(THIRD_PARTY_DIR ${PROJECT_SOURCE_DIR}/third_party)

add_subdirectory(src)

if(BUILD_HTTP_CPPREST OR BUILD_WEBSOCKET_CPPREST)
    set(USE_CPPREST TRUE)
endif()

if(BUILD_GRPC_CLIENT)
    add_subdirectory(${THIRD_PARTY_DIR}/grpc)
elseif(USE_CPPREST)
    add_subdirectory(${THIRD_PARTY_DIR}/grpc/third_party/boringssl)
    add_subdirectory(${THIRD_PARTY_DIR}/grpc/third_party/protobuf/cmake)
endif()

if(USE_CPPREST)
    set(OPENSSL_INCLUDE_DIR ${THIRD_PARTY_DIR}/grpc/third_party/boringssl/include)
    set(BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(BUILD_SAMPLES OFF CACHE BOOL "" FORCE)
    set(WERROR OFF CACHE BOOL "" FORCE)
    set(CPPREST_EXCLUDE_COMPRESSION ON CACHE BOOL "" FORCE)
    set(CPPREST_ABI_TAG "" CACHE STRING "" FORCE)
    add_subdirectory(${THIRD_PARTY_DIR}/cpprestsdk)
endif()

# only build tests for desktop OS
if(NOT CMAKE_CROSSCOMPILING)
    add_subdirectory(test)
endif()
