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

cmake_policy(SET CMP0074 NEW)

project(nakama-cpp VERSION 2.0.2 DESCRIPTION "Nakama C++ Client" LANGUAGES CXX)

if(WIN32)
    include(windows_support)
    set(INSTALL_LIB_DIR ${INSTALL_LIB_DIR}\\win32)
elseif(ANDROID)
    set(INSTALL_LIB_DIR ${INSTALL_LIB_DIR}/android/${ANDROID_ABI})
elseif(CMAKE_OSX_SYSROOT MATCHES ".*iphoneos.*")
    set(INSTALL_LIB_DIR ${INSTALL_LIB_DIR}/ios)
endif()

set(CMAKE_INSTALL_BINDIR)
set(CMAKE_INSTALL_LIBDIR ${INSTALL_LIB_DIR})
#message("INSTALL_LIB_DIR=${INSTALL_LIB_DIR}")

set(_src_root_path ${CMAKE_CURRENT_LIST_DIR})
file(
    GLOB_RECURSE _source_list
    LIST_DIRECTORIES false
    "${_src_root_path}/*.c*"
    "${_src_root_path}/*.h*"
)

set(_include_root_path ${PUBLIC_INCLUDE_DIR}/nakama-cpp)

file(
    GLOB_RECURSE _public_headers_list
    LIST_DIRECTORIES false
    "${_include_root_path}/*.h*"
)

if(NAKAMA_SHARED_LIBRARY)
    add_library(nakama-cpp SHARED ${_source_list} ${_public_headers_list})
    set_target_properties(nakama-cpp PROPERTIES
        VERSION ${PROJECT_VERSION}
        SOVERSION 1
    )

    target_compile_definitions(nakama-cpp PRIVATE NAKAMA_SHARED_LIBRARY)
    target_compile_definitions(nakama-cpp PRIVATE NAKAMA_SHARED_LIBRARY_EXPORTS)

    if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
        # disable C4251 warnings
        target_compile_options(nakama-cpp INTERFACE /wd4251)
    endif()

else()
    # static lib
    add_library(nakama-cpp STATIC ${_source_list} ${_public_headers_list})
endif()

set(CMAKE_CXX_STANDARD 11)

if(LOGS_ENABLED)
    add_definitions(-DNLOGS_ENABLED)
endif(LOGS_ENABLED)

if(NAKAMA_SSL_ENABLED)
    add_definitions(-DNAKAMA_SSL_ENABLED)
endif(NAKAMA_SSL_ENABLED)

if(BUILD_WEBSOCKETPP)
    add_definitions(-DBUILD_WEBSOCKETPP)
    set(Boost_USE_STATIC_LIBS ON) # only find static libs
    find_package(Boost 1.67.0 REQUIRED COMPONENTS system date_time regex)
    if(NOT Boost_FOUND)
        message(FATAL_ERROR "Boost not found")
    endif()
    target_include_directories(nakama-cpp PRIVATE ${Boost_INCLUDE_DIRS})
    target_link_libraries(nakama-cpp PUBLIC ${Boost_LIBRARIES})
endif(BUILD_WEBSOCKETPP)

if(BUILD_IXWEBSOCKET)
    add_definitions(-DBUILD_IXWEBSOCKET)
    target_link_libraries(nakama-cpp PUBLIC ixwebsocket)
endif(BUILD_IXWEBSOCKET)

target_link_libraries(nakama-cpp PUBLIC grpc++ libprotobuf)

target_include_directories(nakama-cpp
    PUBLIC ${THIRD_PARTY_DIR}
    PUBLIC ${PUBLIC_INCLUDE_DIR}
    PRIVATE api
    PRIVATE .
    PRIVATE ${THIRD_PARTY_DIR}/grpc/third_party/protobuf/src
)

foreach(_source IN ITEMS ${_source_list})
    get_filename_component(_source_path "${_source}" PATH)
    file(RELATIVE_PATH _source_path_rel "${_src_root_path}" "${_source_path}")
    string(REPLACE "/" "\\" _group_path "${_source_path_rel}")
    source_group("src\\${_group_path}" FILES "${_source}")
endforeach()

foreach(_header IN ITEMS ${_public_headers_list})
    get_filename_component(_header_path "${_header}" PATH)
    file(RELATIVE_PATH _header_path_rel "${_include_root_path}" "${_header_path}")
    string(REPLACE "/" "\\" _group_path "${_header_path_rel}")
    source_group("include\\${_group_path}" FILES "${_header}")
endforeach()

install(TARGETS nakama-cpp DESTINATION ${INSTALL_LIB_DIR})
