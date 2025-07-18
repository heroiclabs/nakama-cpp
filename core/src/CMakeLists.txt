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

set(_src_root_path ${CMAKE_CURRENT_LIST_DIR})
file(
        GLOB_RECURSE _source_list
        LIST_DIRECTORIES false
        "${_src_root_path}/*.c*"
        "${_src_root_path}/*.h*"
)

add_library(nakama-sdk-core-misc OBJECT ${_source_list})
add_library(nakama::sdk-core-misc ALIAS nakama-sdk-core-misc)
target_link_libraries(nakama-sdk-core-misc PRIVATE nakama::sdk-interface)

if (RTTI)
    # enable RTTI flag only on sources with classes inheritable by app
    if (NOT MSVC)
        target_compile_options(nakama-sdk-core-misc PRIVATE "-frtti")
    else()
        target_compile_options(nakama-sdk-core-misc PRIVATE "/GR")
    endif()
endif()

if (ANDROID)
    target_link_libraries(nakama-sdk-core-misc PRIVATE log)
endif ()

set(CMAKE_INCLUDE_CURRENT_DIR ON)
