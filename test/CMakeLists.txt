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
cmake_minimum_required(VERSION 3.22)

project(nakama-test LANGUAGES CXX)

if (ADDRESS_SANITIZER)
    # In the top level CMakeLists we used `add_compile_options` to globally add
    # Asan instrumentation to everything we compile, but we dont' want it
    # to be in the test binary, to replicate exactly how asan-enabled dll/so shared
    # library behaves. So here we remove address sanitizer from tests.
    get_directory_property(_compile_options COMPILE_OPTIONS)
    list(REMOVE_ITEM _compile_options -fsanitize=address)
    set_directory_properties(PROPERTIES COMPILE_OPTIONS "${_compiler_options}")
    unset(_compile_options)
endif()


#include(ExternalProject)
#ExternalProject_Add(nakama-sdk
#        SOURCE_DIR ..
#        CMAKE_CACHE_ARGS "-DBUILD_SHARED=ON"
#)

file(
    GLOB SRCS
    LIST_DIRECTORIES false
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c*"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h*"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/realtime/*.c*"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/realtime/*.h*"
)

add_executable(nakama-test ${SRCS})
target_compile_features(nakama-test PRIVATE cxx_std_14)
target_link_libraries(nakama-test PRIVATE nakama::sdk)

get_target_property(NAKAMA_TGT_TYPE nakama::sdk TYPE)
if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows" AND ${NAKAMA_TGT_TYPE} STREQUAL "SHARED_LIBRARY")
    # Copy DLL so that nakama-test.exe can find it
    add_custom_command(TARGET nakama-test POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:nakama-test> $<TARGET_FILE_DIR:nakama-test>
            COMMAND_EXPAND_LISTS
            )
endif()

target_include_directories(nakama-test
    PRIVATE src
)

### Prepare unreal test suite
# Directory where NakamaTestApp.uproject can find sources of dependent modules
set(_unreal_test_tmp ${CMAKE_CURRENT_SOURCE_DIR}/unreal/Intermediate/Source/)
file(MAKE_DIRECTORY ${_unreal_test_tmp})
file(COPY src DESTINATION ${_unreal_test_tmp})
file(COPY ${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/include/rapidjson DESTINATION ${_unreal_test_tmp})

