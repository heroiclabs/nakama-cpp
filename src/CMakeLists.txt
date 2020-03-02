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

project(nakama-cpp DESCRIPTION "Nakama C++ Client" LANGUAGES CXX)

include(windows_support)

set(_src_root_path ${CMAKE_CURRENT_LIST_DIR})
file(
    GLOB_RECURSE _source_list
    LIST_DIRECTORIES false
    "${_src_root_path}/*.c*"
    "${_src_root_path}/*.h*"
)

set(_include_root_path ${PUBLIC_INCLUDE_DIR})

file(
    GLOB_RECURSE _public_headers_list
    LIST_DIRECTORIES false
    "${_include_root_path}/*.h*"
)

if(NAKAMA_SHARED_LIBRARY)
    set(CMAKE_MACOSX_RPATH ON)

    add_library(nakama-cpp SHARED ${_source_list} ${_public_headers_list})

    target_compile_definitions(nakama-cpp PRIVATE NAKAMA_SHARED_LIBRARY)
    target_compile_definitions(nakama-cpp PRIVATE NAKAMA_SHARED_LIBRARY_EXPORTS)

    if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
        # disable C4251 warnings
        target_compile_options(nakama-cpp PRIVATE /wd4251)
    endif()

else()
    # static lib
    add_library(nakama-cpp STATIC ${_source_list} ${_public_headers_list})
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    target_compile_options(nakama-cpp PRIVATE /MP)
elseif(CMAKE_COMPILER_IS_GNUCC)
    target_compile_options(nakama-cpp PRIVATE -Wno-attributes)
    # disable warnings
    target_compile_options(nakama-cpp PRIVATE -Wno-conversion -Wno-pedantic -Wno-cast-qual -Wno-sign-compare)
endif()

target_compile_features(nakama-cpp PUBLIC cxx_std_11)

if(LOGS_ENABLED)
    target_compile_definitions(nakama-cpp PRIVATE NLOGS_ENABLED)
endif(LOGS_ENABLED)

if(BUILD_WEBSOCKET_CPPREST)
    target_compile_definitions(nakama-cpp PRIVATE BUILD_WEBSOCKET_CPPREST)
endif(BUILD_WEBSOCKET_CPPREST)

if(BUILD_HTTP_CPPREST)
    target_compile_definitions(nakama-cpp PRIVATE BUILD_HTTP_CPPREST)
endif(BUILD_HTTP_CPPREST)

if(BUILD_REST_CLIENT)
    target_compile_definitions(nakama-cpp PRIVATE BUILD_REST_CLIENT)
endif(BUILD_REST_CLIENT)

if(BUILD_HTTP_CPPREST OR BUILD_WEBSOCKET_CPPREST)
    target_link_libraries(nakama-cpp PUBLIC cpprest)
    target_compile_definitions(nakama-cpp PRIVATE _TURN_OFF_PLATFORM_STRING)
endif()

if(BUILD_GRPC_CLIENT)
    target_link_libraries(nakama-cpp PUBLIC grpc++)
    target_compile_definitions(nakama-cpp PRIVATE BUILD_GRPC_CLIENT)
endif()

if((BUILD_GRPC_CLIENT AND NAKAMA_SSL_ENABLED) OR BUILD_HTTP_CPPREST OR BUILD_WEBSOCKET_CPPREST)
    target_compile_definitions(nakama-cpp PRIVATE NAKAMA_SSL_ENABLED)
    target_include_directories(nakama-cpp PRIVATE ${THIRD_PARTY_DIR}/grpc/third_party/boringssl/include)
    target_link_libraries(nakama-cpp PUBLIC ssl crypto)
endif()

if(BUILD_C_API)
    target_compile_definitions(nakama-cpp PRIVATE BUILD_C_API)
endif()

target_link_libraries(nakama-cpp PUBLIC libprotobuf)

if(ANDROID)
    target_link_libraries(nakama-cpp PUBLIC log)
endif()

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
