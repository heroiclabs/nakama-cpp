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

project(nakama-cmake-client-example CXX)

add_subdirectory(../../release/nakama-cpp-sdk ${CMAKE_CURRENT_BINARY_DIR}/nakama-cpp)

add_executable(nakama-cmake-client-example nakama-cmake-client-example.cpp)
target_compile_features(nakama-cmake-client-example PRIVATE cxx_std_11)
target_include_directories(nakama-cmake-client-example PRIVATE .)

find_package(OpenSSL REQUIRED)
target_link_libraries(nakama-cmake-client-example PUBLIC OpenSSL::SSL OpenSSL::Crypto)
if (APPLE)
    target_link_libraries(nakama-cmake-client-example PUBLIC "-framework foundation" "-framework security")
endif()
target_link_libraries(nakama-cmake-client-example PUBLIC ext_nakama-cpp)
CopyNakamaSharedLib(nakama-cmake-client-example)
