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

project(nakama-test CXX)

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

get_filename_component(nakama-cpp-shared_path
    ${CMAKE_BINARY_DIR}/src/Debug/nakama-cppd.lib
    ABSOLUTE)
  add_library(nakama-cpp-shared STATIC IMPORTED)
  set_target_properties(nakama-cpp-shared PROPERTIES IMPORTED_LOCATION
    ${nakama-cpp-shared_path})

  include_directories(${THIRD_PARTY_DIR} ${PUBLIC_INCLUDE_DIR})
  add_definitions(-DNAKAMA_SHARED_LIBRARY)

#target_link_libraries(nakama-test PRIVATE nakama-cpp)
target_link_libraries(nakama-test PUBLIC nakama-cpp-shared)

target_include_directories(nakama-test
    PRIVATE .
)

foreach(_source IN ITEMS ${_source_list})
    get_filename_component(_source_path "${_source}" PATH)
    file(RELATIVE_PATH _source_path_rel "${_src_root_path}" "${_source_path}")
    string(REPLACE "/" "\\" _group_path "${_source_path_rel}")
    source_group("src\\${_group_path}" FILES "${_source}")
endforeach()
