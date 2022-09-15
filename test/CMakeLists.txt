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

include(FetchContent)

FetchContent_Declare(
  rapidjson
  GIT_REPOSITORY https://github.com/Tencent/rapidjson
  GIT_TAG        master
  GIT_SHALLOW    TRUE
)

FetchContent_GetProperties(rapidjson)
if(NOT rapidjson_POPULATED)
  FetchContent_Populate(rapidjson)
  add_subdirectory(${rapidjson_SOURCE_DIR} ${rapidjson_BINARY_DIR} EXCLUDE_FROM_ALL) # don't install rapidjson
endif()

get_filename_component(_NAKAMA_PROJECT "${CMAKE_SOURCE_DIR}" DIRECTORY)
set(nakama-sdk_DIR ${_NAKAMA_PROJECT}/out/${CMAKE_INSTALL_LIBDIR}/cmake/nakama)
find_package(nakama-sdk CONFIG REQUIRED)

if (ADDRESS_SANITIZER)
    # In the top level CMakeLists we used `add_compile_options` to globally add
    # Asan instrumentation to everything we compile, but we dont' want it
    # to be in the test binary, to replicate exactly how asan-enabled dll/so shared
    # library behaves. So here we remove address sanitizer from test.
    get_directory_property(_compile_options COMPILE_OPTIONS)
    list(REMOVE_ITEM _compile_options -fsanitize=address)
    set_directory_properties(PROPERTIES COMPILE_OPTIONS "${_compiler_options}")
    unset(_compile_options)
endif()

file(
    GLOB SRCS
    LIST_DIRECTORIES false
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c*"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h*"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/realtime/*.c*"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/realtime/*.h*"
)

add_executable(nakama-test ${SRCS})

target_link_libraries(nakama-test
    INTERFACE rapidjson
    PRIVATE nakama-sdk
)

target_include_directories(nakama-test
    PRIVATE src
    ${rapidjson_SOURCE_DIR}/include ### rapidjson probably not setup correctly by its authors, because target_link_libraries should handle this
)

target_compile_features(nakama-test PRIVATE cxx_std_14)

get_target_property(NAKAMA_TGT_TYPE nakama-sdk TYPE)
if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows" AND ${NAKAMA_TGT_TYPE} STREQUAL "SHARED_LIBRARY")
    # Copy DLL so that nakama-test.exe can find it
    add_custom_command(TARGET nakama-test POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:nakama-test> $<TARGET_FILE_DIR:nakama-test>
            COMMAND_EXPAND_LISTS
            )
endif()

if (${CMAKE_SYSTEM_NAME} STREQUAL "iOS")
    # CMake needs to implement a cleaner way of doing this: https://gitlab.kitware.com/cmake/cmake/-/issues/22997 &&
    # https://gitlab.kitware.com/cmake/cmake/-/issues/22240
    set(CMAKE_BUILD_WITH_INSTALL_RPATH YES)
endif()

# no ZERO_CHECK target for Xcode
set(CMAKE_SUPPRESS_REGENERATION true)

# bitcode will be deprecated in Xcode 14 which is in beta at the time of this writing
# https://developer.apple.com/documentation/xcode-release-notes/xcode-14-release-notes
set_target_properties(nakama-test PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER "com.test"
    XCODE_ATTRIBUTE_ENABLE_BITCODE "NO"
    XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER "com.heroiclabs.test"
    XCODE_ATTRIBUTE_CODE_SIGNING_REQUIRED "YES"
    XCODE_ATTRIBUTE_CODE_SIGNING_STYLE "AUTOMATIC"
    XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "iPhone Developer"
    XCODE_ATTRIBUTE_DEVELOPMENT_TEAM "$ENV{NAKAMA_TEST_DEVELOPMENT_TEAM}"
    XCODE_LINK_BUILD_PHASE_MODE KNOWN_LOCATION
    XCODE_EMBED_FRAMEWORKS ${_NAKAMA_PROJECT}/out/${CMAKE_INSTALL_LIBDIR}/nakama-sdk.framework ## todo figure out why passing target doesn't work
    XCODE_EMBED_FRAMEWORKS_CODE_SIGN_ON_COPY		"YES"		# frameworks must be signed by the same developer as the binary
	XCODE_EMBED_FRAMEWORKS_REMOVE_HEADERS_ON_COPY	"YES"
    XCODE_ATTRIBUTE_CODE_SIGN_STYLE Automatic
)


if (${CMAKE_SYSTEM_NAME} STREQUAL "iOS")
    set_target_properties(nakama-test PROPERTIES
        INSTALL_RPATH @executable_path/Frameworks
    )
endif()

set(_TEST_OUT_DIR "${CMAKE_SOURCE_DIR}/out/test/${CMAKE_INSTALL_LIBDIR}")

install(TARGETS nakama-test
    BUNDLE DESTINATION ${_TEST_OUT_DIR}
    RUNTIME DESTINATION ${_TEST_OUT_DIR}
)

