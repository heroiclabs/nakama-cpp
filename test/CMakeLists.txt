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
cmake_minimum_required(VERSION 3.13)

project(nakama-test C CXX)

include(windows_support)

set(_src_root_path ${CMAKE_CURRENT_LIST_DIR})
file(
    GLOB_RECURSE _source_list
    LIST_DIRECTORIES false
    "${_src_root_path}/*.c*"
    "${_src_root_path}/*.h*"
)

add_executable(nakama-test ${_source_list})
target_compile_features(nakama-test PRIVATE cxx_std_11)

if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    target_compile_options(nakama-test PRIVATE /MP)
endif()

if(NAKAMA_SHARED_LIBRARY)
    add_subdirectory(../release/nakama-cpp-sdk ${CMAKE_CURRENT_BINARY_DIR}/nakama-cpp-shared)
    target_link_libraries(nakama-test PUBLIC ext_nakama-cpp)
    CopyNakamaSharedLib(nakama-test)
else()
    target_link_libraries(nakama-test PRIVATE nakama-cpp)
endif()

target_include_directories(nakama-test
    PRIVATE .
    PRIVATE ${THIRD_PARTY_DIR}
)

if(BUILD_C_API)
    target_compile_definitions(nakama-test PRIVATE BUILD_C_API)
endif()

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} PREFIX src FILES ${_source_list})
