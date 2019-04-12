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
cmake_minimum_required(VERSION 3.6)

set(ROOT_LIBS_FOLDER "${CMAKE_CURRENT_SOURCE_DIR}/libs")

# check architecture
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(CPU_ARCH "x64")
else()
    set(CPU_ARCH "x86")
endif()

if(ANDROID)
    if("${ANDROID_STL}" STREQUAL "c++_static")
        set(LIBS_FOLDER "${ROOT_LIBS_FOLDER}/android/${ANDROID_ABI}")
    else()
        message(FATAL_ERROR "nakama-cpp: only c++_static STL is supported")
    endif()
elseif(WIN32)
    # vc140 - Visual Studio 2015
    # vc141 - Visual Studio 2017
    if(${CPU_ARCH} STREQUAL "x64")
        set(LIBS_FOLDER "${ROOT_LIBS_FOLDER}/win64/${CMAKE_VS_PLATFORM_TOOLSET}/${CMAKE_BUILD_TYPE}")
    else()
        set(LIBS_FOLDER "${ROOT_LIBS_FOLDER}/win32/${CMAKE_VS_PLATFORM_TOOLSET}/${CMAKE_BUILD_TYPE}")
    endif()
elseif(APPLE_IOS)
    # iOS
    set(LIBS_FOLDER "${ROOT_LIBS_FOLDER}/ios")
elseif(APPLE)
    # Mac
    set(LIBS_FOLDER "${ROOT_LIBS_FOLDER}/mac")
elseif(UNIX AND NOT APPLE)
    # Linux
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(LIBS_FOLDER "${ROOT_LIBS_FOLDER}/linux/${CPU_ARCH}")
    else()
        set(LIBS_FOLDER "${ROOT_LIBS_FOLDER}/linux/${CPU_ARCH}")
    endif()
endif()

set(LIB_PREFIX "")
set(LIB_SUFFIX "")
set(LIB_EXT "")

if(WIN32)
    if (NOT CMAKE_BUILD_TYPE)
        set(CMAKE_BUILD_TYPE Debug)
    endif()
    if(CMAKE_BUILD_TYPE MATCHES Debug)
        set(LIB_SUFFIX "d")
    endif()
    set(LIB_EXT ".lib")
else()
    # Android, Mac, iOS, Linux
    set(LIB_PREFIX "lib")
    set(LIB_EXT ".a")
endif()

# grpc++
project(grpc++)
add_library(grpc++ STATIC IMPORTED GLOBAL)
set_target_properties(grpc++ PROPERTIES
    IMPORTED_LOCATION "${LIBS_FOLDER}/${LIB_PREFIX}grpc++${LIB_SUFFIX}${LIB_EXT}"
)

# grpc
project(grpc)
add_library(grpc STATIC IMPORTED GLOBAL)
set_target_properties(grpc PROPERTIES
    IMPORTED_LOCATION "${LIBS_FOLDER}/${LIB_PREFIX}grpc${LIB_SUFFIX}${LIB_EXT}"
)

# protobuf
project(protobuf)
add_library(protobuf STATIC IMPORTED GLOBAL)
if(WIN32)
    set_target_properties(protobuf PROPERTIES
        IMPORTED_LOCATION "${LIBS_FOLDER}/${LIB_PREFIX}libprotobuf${LIB_SUFFIX}${LIB_EXT}"
    )
else()
    # Android, Mac, iOS, Linux
    set_target_properties(protobuf PROPERTIES
        IMPORTED_LOCATION "${LIBS_FOLDER}/${LIB_PREFIX}protobuf${LIB_SUFFIX}${LIB_EXT}"
    )
endif()

# cares
project(cares)
add_library(cares STATIC IMPORTED GLOBAL)
set_target_properties(cares PROPERTIES
    IMPORTED_LOCATION "${LIBS_FOLDER}/${LIB_PREFIX}cares${LIB_SUFFIX}${LIB_EXT}"
)

# address_sorting
project(address_sorting)
add_library(address_sorting STATIC IMPORTED GLOBAL)
set_target_properties(address_sorting PROPERTIES
    IMPORTED_LOCATION "${LIBS_FOLDER}/${LIB_PREFIX}address_sorting${LIB_SUFFIX}${LIB_EXT}"
)

# gpr
project(gpr)
add_library(gpr STATIC IMPORTED GLOBAL)
set_target_properties(gpr PROPERTIES
    IMPORTED_LOCATION "${LIBS_FOLDER}/${LIB_PREFIX}gpr${LIB_SUFFIX}${LIB_EXT}"
)

# zlib
project(zlib)
add_library(zlib STATIC IMPORTED GLOBAL)
if(WIN32)
    set_target_properties(zlib PROPERTIES
        IMPORTED_LOCATION "${LIBS_FOLDER}/${LIB_PREFIX}zlib${LIB_SUFFIX}${LIB_EXT}"
    )
else()
    # Android, Mac, iOS, Linux
    set_target_properties(zlib PROPERTIES
        IMPORTED_LOCATION "${LIBS_FOLDER}/${LIB_PREFIX}z${LIB_SUFFIX}${LIB_EXT}"
    )
endif()

# ssl
project(ssl)
add_library(ssl STATIC IMPORTED GLOBAL)
set_target_properties(ssl PROPERTIES
    IMPORTED_LOCATION "${LIBS_FOLDER}/${LIB_PREFIX}ssl${LIB_SUFFIX}${LIB_EXT}"
)

# crypto
project(crypto)
add_library(crypto STATIC IMPORTED GLOBAL)
set_target_properties(crypto PROPERTIES
    IMPORTED_LOCATION "${LIBS_FOLDER}/${LIB_PREFIX}crypto${LIB_SUFFIX}${LIB_EXT}"
)

if(ANDROID OR APPLE)
    set(IXWEBSOCKET_AVL TRUE)

    # ixwebsocket
    project(ixwebsocket)
    add_library(ixwebsocket STATIC IMPORTED GLOBAL)
    set_target_properties(ixwebsocket PROPERTIES
        IMPORTED_LOCATION "${LIBS_FOLDER}/${LIB_PREFIX}ixwebsocket${LIB_SUFFIX}${LIB_EXT}"
    )
else()
    set(IXWEBSOCKET_AVL FALSE)
endif()

# nakama-cpp
set(lib_name nakama-cpp)
set(target_name ext_${lib_name})

project(${lib_name})

add_library(${target_name} STATIC IMPORTED GLOBAL)

set_target_properties(${target_name} PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

set_target_properties(${target_name} PROPERTIES
    INTERFACE_LINK_DIRECTORIES "${LIBS_FOLDER}"
)

set_target_properties(${target_name} PROPERTIES
    IMPORTED_LOCATION "${LIBS_FOLDER}/${LIB_PREFIX}${lib_name}${LIB_SUFFIX}${LIB_EXT}"
)

target_link_libraries(${target_name} INTERFACE grpc++ grpc protobuf cares address_sorting gpr zlib ssl crypto)

if(IXWEBSOCKET_AVL)
    target_link_libraries(${target_name} INTERFACE ixwebsocket)
endif()

if(ANDROID)
    target_link_libraries(${target_name} INTERFACE log)
endif()

if(APPLE_IOS)
    target_link_libraries(${target_name} INTERFACE resolv.9)
endif()

if(UNIX AND NOT APPLE AND NOT ANDROID)
    # Linux
    target_link_libraries(${target_name} INTERFACE pthread)
endif()
