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

if(NAKAMA_SHARED_LIBRARY)
    add_definitions(-DNAKAMA_SHARED_LIBRARY)
    set(ROOT_LIBS_FOLDER "${CMAKE_CURRENT_SOURCE_DIR}/shared-libs")
else()
    set(ROOT_LIBS_FOLDER "${CMAKE_CURRENT_SOURCE_DIR}/libs")
endif()

if(UNIX AND NOT APPLE)
    set(LINUX TRUE)
endif()

# check architecture
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(CPU_ARCH "x64")
else()
    set(CPU_ARCH "x86")
endif()

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

if(ANDROID)
    if("${ANDROID_STL}" STREQUAL "c++_static")
        set(LIBS_FOLDER "${ROOT_LIBS_FOLDER}/android/${ANDROID_ABI}")
    else()
        message(FATAL_ERROR "nakama-cpp: only c++_static STL is supported")
    endif()
elseif(WIN32)
    # CMAKE_VS_PLATFORM_TOOLSET:
    #   v140 - Visual Studio 2015
    #   v141 - Visual Studio 2017
    #   v142 - Visual Studio 2019
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
elseif(LINUX)
    # Linux
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(LIBS_FOLDER "${ROOT_LIBS_FOLDER}/linux/${CPU_ARCH}")
    else()
        set(LIBS_FOLDER "${ROOT_LIBS_FOLDER}/linux/${CPU_ARCH}")
    endif()
endif()

if(NAKAMA_SHARED_LIBRARY)
    # nakama-cpp shared lib
    message("Using Nakama shared lib")
    set(lib_name nakama-cpp)
    set(target_name ext_${lib_name})

    project(${lib_name})

    if(WIN32)
        add_library(${target_name} STATIC IMPORTED GLOBAL)
    
        set(LIB_SUFFIX "")
        if(CMAKE_BUILD_TYPE MATCHES Debug)
            set(LIB_SUFFIX "d")
        endif()
        
        get_filename_component(nakama-cpp-shared_path
            ${LIBS_FOLDER}/nakama-cpp${LIB_SUFFIX}.lib
            ABSOLUTE)

        message("nakama-cpp-shared_path=${nakama-cpp-shared_path}")

        set_target_properties(${target_name} PROPERTIES
            IMPORTED_LOCATION ${nakama-cpp-shared_path})

        get_filename_component(nakama-cpp-shared_path
            ${LIBS_FOLDER}/nakama-cpp${LIB_SUFFIX}.dll
            ABSOLUTE)
        
        # disable C4251 warnings
        target_compile_options(${target_name} INTERFACE /wd4251)
    else()
        add_library(${target_name} SHARED IMPORTED GLOBAL)
        
        set(LIBS_FOLDER "${LIBS_FOLDER}/${CMAKE_OSX_ARCHITECTURES}")
        
        if(APPLE)
            set(SHARED_LIB_FILE "libnakama-cpp.dylib")
        else()
            set(SHARED_LIB_FILE "libnakama-cpp.so")
        endif()
        
        get_filename_component(nakama-cpp-shared_path
            ${LIBS_FOLDER}/${SHARED_LIB_FILE}
            ABSOLUTE)

        set_target_properties(${target_name} PROPERTIES
            IMPORTED_LOCATION ${nakama-cpp-shared_path})
    endif()

    set(nakama-cpp-shared-lib-path ${nakama-cpp-shared_path} CACHE STRING "Path to Nakama shared library" FORCE)
    target_compile_definitions(${target_name} INTERFACE NAKAMA_SHARED_LIBRARY)
    
    function(CopyNakamaSharedLib target)
        set(output_directory $<TARGET_FILE_DIR:${target}>)
        message("CopyNakamaSharedLib output_directory=${output_directory}")
        message("CopyNakamaSharedLib nakama-cpp-shared-lib-path=${nakama-cpp-shared-lib-path}")
        add_custom_command(
            TARGET ${target} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different ${nakama-cpp-shared-lib-path} ${output_directory})
    endfunction()

else()
    # static libs
    message("Using Nakama static lib")

    set(NAKAMA_REST_CLIENT ON)
    set(NAKAMA_GRPC_CLIENT OFF)
    set(NAKAMA_CPPREST ON)

    set(LIB_PREFIX "")
    set(LIB_SUFFIX "")
    set(LIB_EXT "")

    if(WIN32)
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

    if(NAKAMA_CPPREST)
        # cpprest
        project(cpprest)
        add_library(cpprest STATIC IMPORTED GLOBAL)
        set_target_properties(cpprest PROPERTIES
            IMPORTED_LOCATION "${LIBS_FOLDER}/${LIB_PREFIX}cpprest${LIB_SUFFIX}${LIB_EXT}"
        )

        if(WIN32)
            target_link_libraries(cpprest INTERFACE Crypt32.lib Bcrypt.lib Winhttp.lib)
        elseif(ANDROID OR LINUX)
            # boost not needed
        else()
            target_link_libraries(cpprest INTERFACE
                "${LIBS_FOLDER}/${LIB_PREFIX}boost_thread${LIB_SUFFIX}${LIB_EXT}"
                "${LIBS_FOLDER}/${LIB_PREFIX}boost_chrono${LIB_SUFFIX}${LIB_EXT}"
            )
        endif()
    endif()

    # nakama-cpp static lib
    set(lib_name nakama-cpp)
    set(target_name ext_${lib_name})

    project(${lib_name})

    add_library(${target_name} STATIC IMPORTED GLOBAL)

    set_target_properties(${target_name} PROPERTIES
        INTERFACE_LINK_DIRECTORIES "${LIBS_FOLDER}"
    )

    set_target_properties(${target_name} PROPERTIES
        IMPORTED_LOCATION "${LIBS_FOLDER}/${LIB_PREFIX}${lib_name}${LIB_SUFFIX}${LIB_EXT}"
    )

    if(NAKAMA_REST_CLIENT)
        target_link_libraries(${target_name} INTERFACE protobuf)
    endif()

    if(NAKAMA_GRPC_CLIENT)
        target_link_libraries(${target_name} INTERFACE grpc++ grpc protobuf cares address_sorting gpr zlib ssl crypto)
    endif()

    if(NAKAMA_CPPREST)
        target_link_libraries(${target_name} INTERFACE cpprest ssl crypto)
    endif()
    
    if(APPLE)
        target_link_libraries(${target_name} INTERFACE "-framework foundation" "-framework security")
    endif()
    
    if(ANDROID)
        target_link_libraries(${target_name} INTERFACE log)
    endif()

    if(APPLE_IOS)
        target_link_libraries(${target_name} INTERFACE resolv.9)
    endif()

    if(LINUX)
        target_link_libraries(${target_name} INTERFACE pthread)
    endif()
    
    function(CopyNakamaSharedLib target)
        # do nothing for static lib
    endfunction()
endif()

target_include_directories(${target_name} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
