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

cmake_minimum_required(VERSION 3.23)


if(APPLE AND (NOT DEFINED ENV{NAKAMA_TEST_DEVELOPMENT_TEAM}))
    message(FATAL_ERROR "No test development team setup. Please have an entry for NAKAMA_TEST_DEVELOPMENT_TEAM in your environment variables. See README.md for more information.")
endif()

 ## force so that gradle doesn't stomp vcpkg toolchain with the android toolchain
set(CMAKE_TOOLCHAIN_FILE $ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake CACHE FILEPATH "Vcpkg toolchain file" FORCE)

if (DEFINED ANDROID_ABI AND NOT DEFINED VCPKG_CHAINLOAD_TOOLCHAIN_FILE)
    # CmakePresets.json doesn't allow us to use cmake vars in values, so set it here
    set(VCPKG_CHAINLOAD_TOOLCHAIN_FILE $ENV{ANDROID_NDK_HOME}/build/cmake/android.toolchain.cmake)
endif()

project(nakama-test LANGUAGES C CXX)

include(GNUInstallDirs)

option(LOGS_ENABLED "Enable log output" ON)
option(REMOTE_NAKAMA_SDK "Use a remote (vcpkg) installation of Nakama" OFF)
option(BUILD_SHARED_NAKAMA_SDK "Whether the local Nakama SDK should be static or shared." ON)
option(BUILD_EXECUTABLE "Whether or not to make the test an executable for standalone execution." ON)
set(NAKAMA_SDK_CONFIG "Debug" CACHE STRING "The Nakama SDK configuration")

if (NOT BUILD_EXECUTABLE AND BUILD_SHARED_LIBS)
    if(BUILD_SHARED_LIBS)
        set(CMAKE_POSITION_INDEPENDENT_CODE ON)
    endif()
endif()

set_target_properties(nakama-test PROPERTIES CXX_VISIBILITY_PRESET default)

if(REMOTE_NAKAMA_SDK)
    list(APPEND VCPKG_MANIFEST_FEATURES "nakama-sdk")
else()

    if (${PRESET_NAME})
        execute_process(
            COMMAND ${CMAKE_COMMAND} -S .. --preset ${PRESET_NAME} -DBUILD_SHARED_LIBS=${BUILD_SHARED_NAKAMA_SDK}
            RESULT_VARIABLE result
        )

        if (result)
            message(FATAL_ERROR "Configuration of nakama-cpp preset ${PRESET_NAME} failed.")
        endif()

        execute_process(
            COMMAND ${CMAKE_COMMAND} --build ../build/${PRESET_NAME} --config ${NAKAMA_SDK_CONFIG} --target install
            RESULT_VARIABLE result
        )

        if (result)
            message(FATAL_ERROR "Building and installing nakama-cpp with preset ${PRESET_NAME} failed.")
        endif()
    endif()
endif()

include(FetchContent)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(THREADS_PREFER_PTHREAD_FLAG ON)
    find_package(Threads REQUIRED)
    set(PTHREADS_LIB Threads::Threads)
endif()

### This needs to be one of the first commands in order to prevent code signing issues.
if (${CMAKE_SYSTEM_NAME} STREQUAL "iOS" OR ${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    # CMake needs to implement a cleaner way of doing this: https://gitlab.kitware.com/cmake/cmake/-/issues/22997 &&
    # https://gitlab.kitware.com/cmake/cmake/-/issues/22240
    set(CMAKE_BUILD_WITH_INSTALL_RPATH YES)
endif()

find_package(RapidJSON CONFIG REQUIRED)
find_package(optional-lite CONFIG REQUIRED)

### CMakePresets.json doesn't seem to pass the ${installDir} to the CMake file, so we have to reconstitute it.
string(REPLACE ${CMAKE_SOURCE_DIR}/ "" CMAKE_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}")
get_filename_component(_NAKAMA_PROJECT "${CMAKE_SOURCE_DIR}" DIRECTORY)

if (NOT REMOTE_NAKAMA_SDK)
    set(nakama-sdk_DIR ${_NAKAMA_PROJECT}/${CMAKE_INSTALL_DIR}/share/nakama-sdk)
endif()

find_package(nakama-sdk CONFIG REQUIRED)

file(
    GLOB SRCS
    LIST_DIRECTORIES false
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c*"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h*"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/realtime/*.c*"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/realtime/*.h*"
)

# for Android, we are following this guide: https://medium.com/androiddevelopers/getting-started-with-c-and-android-native-activities-2213b402ffff
# alternatively we could do this in `Android.mk` but we already use CMake.
if (BUILD_EXECUTABLE)
    if (ANDROID)
        find_library(log-lib log)

        add_library(nakama-test ${SRCS} "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp")

        target_link_libraries(nakama-test
            PUBLIC
                ${log-lib}
                android
        )
    else()
        add_executable(nakama-test MACOSX_BUNDLE ${SRCS} "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp")
    endif()
else()
    add_library(nakama-test ${SRCS})
endif()


target_include_directories(nakama-test
    INTERFACE
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
    PRIVATE
        src
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

if (LOGS_ENABLED)
    target_compile_definitions(nakama-test PRIVATE "NLOGS_ENABLED")
endif()

target_link_libraries(nakama-test
    INTERFACE rapidjson
    PRIVATE nakama-sdk
)

if (NAKAMA_SDK_BUILD_TYPE STREQUAL "STATIC")
    find_package(Protobuf CONFIG)
    find_package(wslay CONFIG)
    find_package(curl CONFIG)
endif()

target_compile_features(nakama-test PRIVATE cxx_std_14)

## Some proprietary platforms disable exceptions by default. We need to enable them for the test library.
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  target_compile_options(nakama-test PRIVATE -fexceptions)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  target_compile_options(nakama-test PRIVATE /EHsc)
endif()

if (BUILD_EXECUTABLE)
    get_target_property(NAKAMA_TGT_TYPE nakama-sdk TYPE)
    if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows" AND ${NAKAMA_TGT_TYPE} STREQUAL "SHARED_LIBRARY")
        # Copy DLL so that nakama-test.exe can find it
        add_custom_command(TARGET nakama-test POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:nakama-test> $<TARGET_FILE_DIR:nakama-test>
                COMMAND_EXPAND_LISTS
                )
    endif()
endif()

# no ZERO_CHECK target for Xcode
set(CMAKE_SUPPRESS_REGENERATION true)


if (BUILD_SHARED_LIBS)
    if (REMOTE_NAKAMA_SDK)
        set(NAKAMA_SDK_FRAMEWORK_LOCATION ${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/lib/nakama-sdk.framework)
    else()
        set(NAKAMA_SDK_FRAMEWORK_LOCATION ${_NAKAMA_PROJECT}/${CMAKE_INSTALL_DIR}/lib/nakama-sdk.framework)
    endif()
endif()


if (BUILD_EXECUTABLE)
    set_target_properties(nakama-test PROPERTIES
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.test"
        MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/test.plist
        XCODE_ATTRIBUTE_ENABLE_BITCODE "NO"
        XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER "com.heroiclabs.test"
        XCODE_ATTRIBUTE_CODE_SIGNING_REQUIRED "YES"
        XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "Apple Development"
        XCODE_ATTRIBUTE_DEVELOPMENT_TEAM "$ENV{NAKAMA_TEST_DEVELOPMENT_TEAM}"
        XCODE_LINK_BUILD_PHASE_MODE KNOWN_LOCATION
        XCODE_ATTRIBUTE_CODE_SIGN_STYLE Automatic
    )

    if (BUILD_SHARED_LIBS)
        set_target_properties(nakama-test PROPERTIES
            XCODE_EMBED_FRAMEWORKS ${NAKAMA_SDK_FRAMEWORK_LOCATION} ## todo figure out why passing target doesn't work
            XCODE_EMBED_FRAMEWORKS_CODE_SIGN_ON_COPY		"YES"		# frameworks must be signed by the same developer as the binary
            XCODE_EMBED_FRAMEWORKS_REMOVE_HEADERS_ON_COPY	"YES"
        )
    endif()

    if(${CMAKE_SYSTEM_NAME} STREQUAL "iOS")
        set_target_properties(nakama-test PROPERTIES
            INSTALL_RPATH @executable_path/Frameworks
        )
    elseif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
        set_target_properties(nakama-test PROPERTIES
            INSTALL_RPATH @executable_path/../Frameworks
        )
    elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
        set_target_properties(nakama-test PROPERTIES
            INSTALL_RPATH $ORIGIN
        )
    endif()

endif()

install(TARGETS nakama-test
    EXPORT nakama-test-export
    BUNDLE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_LIBDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

file(GLOB HEADER_LIST CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/include/*.h")
install(FILES ${HEADER_LIST} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/nakama-test)

if(NOT BUILD_EXECUTABLE)
    ## use share directory for export files (at least a vcpkg convention)
    install(EXPORT nakama-test-export
            DESTINATION share/nakama-test)

    include(CMakePackageConfigHelpers)

    configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/nakama-test-config.cmake.in
        "${CMAKE_CURRENT_BINARY_DIR}/nakama-test-config.cmake"
        INSTALL_DESTINATION share/nakama-test
        NO_SET_AND_CHECK_MACRO
        NO_CHECK_REQUIRED_COMPONENTS_MACRO
    )

    install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/nakama-test-config.cmake
        DESTINATION share/nakama-test
    )

endif()

if (BUILD_EXECUTABLE)
    if (${NAKAMA_SDK_BUILD_TYPE} STREQUAL "SHARED")
        install(IMPORTED_RUNTIME_ARTIFACTS nakama-sdk
            RUNTIME DESTINATION ${CMAKE_INSTALL_LIBDIR}
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            FRAMEWORK DESTINATION ${CMAKE_INSTALL_LIBDIR}
        )
    endif()

    if (CMAKE_SYSTEM_NAME MATCHES "Windows" OR CMAKE_SYSTEM_NAME MATCHES "Linux")
        add_custom_target(run
            COMMAND ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR}/nakama-test
            USES_TERMINAL
            DEPENDS install
        )
    elseif (CMAKE_SYSTEM_NAME MATCHES "Darwin")
        add_custom_target(run
            COMMAND $<TARGET_BUNDLE_DIR:nakama-test>/Contents/MacOS/nakama-test
            USES_TERMINAL
            DEPENDS install
        )
    endif()
endif()
