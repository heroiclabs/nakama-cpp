cmake_minimum_required(VERSION 3.23)
include(version.cmake)

# we use list append to allow dependent libraries to inject their own triplets and ports into this.
list(APPEND VCPKG_OVERLAY_TRIPLETS "${CMAKE_SOURCE_DIR}/cmake/triplets")
list(APPEND VCPKG_OVERLAY_PORTS "${CMAKE_SOURCE_DIR}/cmake/vcpkg-ports")

option(BUILD_GRPC_CLIENT "Build GRPC client" ON)
set(WS_IMPL OFF CACHE STRING "Websocket implementation.")

set(HTTP_IMPL OFF CACHE STRING "HTTP implementation")

option(UNREAL "Unreal engine support" OFF)

if (UNREAL)
    message(NOTICE "Disabling HTTP transport in Unreal build mode")
    set(HTTP_IMPL OFF CACHE STRING "HTTP implementation" FORCE)

    # too many bugs in the Linux WS implementation:
    # https://github.com/EpicGames/UnrealEngine/pull/7267 and
    # https://github.com/EpicGames/UnrealEngine/pull/9103
    if (NOT VCPKG_TARGET_TRIPLET MATCHES "linux")
        message(NOTICE "Disabling WS transport in Unreal build mode")
        set(WS_IMPL OFF CACHE STRING "WS implementation" FORCE)
    endif()
endif()

# when building android, this cmake is called from gradle with ANDROID_ABI passed.
# use it to select vcpkg triplet
if (ANDROID_ABI MATCHES armeabi-v7a)
    set(VCPKG_TARGET_TRIPLET arm-android-heroic)
elseif (ANDROID_ABI MATCHES arm64-v8a)
    set(VCPKG_TARGET_TRIPLET arm64-android-heroic)
elseif (ANDROID_ABI MATCHES x86_64)
    set(VCPKG_TARGET_TRIPLET x64-android-heroic)
endif()


# We need to set VCPKG_MANIFEST_FEATURE before calling project(), because
# that is where dependencies are installed and where real toolchain file (if any)
# is configured.
if (BUILD_GRPC_CLIENT)
    list(APPEND VCPKG_MANIFEST_FEATURES "grpc")
endif()

# Not that many CMake variables are set before first project(),
# for instance CMAKE_SYSTEM_NAME which would normally identify build target platform
# is empty at this point, so we rely on VCPKG_TARGET_TRIPLET to identify platform.
if (HTTP_IMPL STREQUAL "libhttpclient" AND VCPKG_TARGET_TRIPLET MATCHES "linux|ios|osx|iphonesimulator")
    # on MacOSX curl seems to be part of the system, but we bring our own for consistency
    list(APPEND VCPKG_MANIFEST_FEATURES "curl")
endif()

if (LIBHTTPCLIENT_FORCE_WEBSOCKETPP)
    list(APPEND VCPKG_MANIFEST_FEATURES "websocketpp")
endif()

if (WS_IMPL STREQUAL "wslay")
    list(APPEND VCPKG_MANIFEST_FEATURES "wslay")
    if(VCPKG_TARGET_TRIPLET MATCHES "linux|ios|osx|iphonesimulator")
        list(APPEND VCPKG_MANIFEST_FEATURES "curl")
    endif()
endif()

# being called from gradle
if (DEFINED ANDROID_ABI AND NOT DEFINED VCPKG_CHAINLOAD_TOOLCHAIN_FILE)
    # CmakePresets.json doesn't allow us to use cmake vars in values, so set it here
    set(VCPKG_CHAINLOAD_TOOLCHAIN_FILE ${ANDROID_NDK}/build/cmake/android.toolchain.cmake)

    # and also set env var to be used by *-android-heroic triplets
    set(ENV{VCPKG_CHAINLOAD_TOOLCHAIN_FILE} "${VCPKG_CHAINLOAD_TOOLCHAIN_FILE}")
endif()

cmake_policy(SET CMP0012 NEW)



# If this CMake is called as part of another build system, like gradle's android plugin,
# where CMAKE_TOOLCHAIN_FILE is unconditionally passed, we can pass extra arg TOOLCHAIN_LOAD_FIRST
# to force toolchain value and then chainload original toolchain
if(DEFINED TOOLCHAIN_LOAD_FIRST)
    set(VCPKG_CHAINLOAD_TOOLCHAIN_FILE "${CMAKE_TOOLCHAIN_FILE}")
    cmake_path(ABSOLUTE_PATH TOOLCHAIN_LOAD_FIRST OUTPUT_VARIABLE CMAKE_TOOLCHAIN_FILE)
endif()

# must come before first project() so that libc++ is compiled
# and ready when vcpkg starts building dependencies
include(cmake/with-libcxx.cmake)

# Kickstarts vcpkg dependency manager. It will parse vcpkg.json and install all configured
# dependencies. Some dependencies are optional and enabled via VCPKG_MANIFEST_FEATURES
set(CMAKE_TOOLCHAIN_FILE $ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake
        CACHE FILEPATH "Vcpkg toolchain file")

project(nakama-sdk LANGUAGES CXX)
include(cmake/platformSupport.cmake)
include(cmake/compilerFlags.cmake)
include(cmake/linkerFlags.cmake)
include(cmake/localCompilerFlags.cmake)
include(GNUInstallDirs)
include(FetchContent)

if(MSVC)
    # On windows it is critical not to accidentally use DLL
    # compiled with "wrong" runtime (/MD, /MDd) so we name them differently
    set(CMAKE_DEBUG_POSTFIX "_d")
endif()

set(CMAKE_CXX_STANDARD 17)
option(LOGS_ENABLED "Enable log output" ON)
option(NAKAMA_SHARED_LIBRARY "Build shared library" ON)

if(NOT MSVC)
    string(APPEND CMAKE_CXX_FLAGS " -fexceptions")
endif()

option(ADDRESS_SANITIZER "Enable address sanitizer (broken on windows)" OFF)

# libhttpclient uses websocketpp when system winhttp doesn't provide one (< Windows 8)
# this flag allows us to force use of websocketpp so that it can be tested on current Windows versions
option(LIBHTTPCLIENT_FORCE_WEBSOCKETPP "Force use of websocketpp instead of winhttp in the libhttpclient" OFF)

if(LOGS_ENABLED)
    add_compile_definitions(NLOGS_ENABLED)
endif(LOGS_ENABLED)

if (ADDRESS_SANITIZER)
    add_compile_options(-fsanitize=address -O1 -fno-optimize-sibling-calls -fno-omit-frame-pointer)
    string(REPLACE /RTC1 "" CMAKE_CXX_FLAGS_DEBUG ${CMAKE_CXX_FLAGS_DEBUG})
endif()


set(BUILD_MODE STATIC)
if(NAKAMA_SHARED_LIBRARY)
    set(CMAKE_POSITION_INDEPENDENT_CODE ON)
    set(BUILD_MODE SHARED)
    add_compile_definitions(NAKAMA_SHARED_LIBRARY NAKAMA_SHARED_LIBRARY_EXPORTS)
endif()


if(APPLE)
    file(TOUCH ${CMAKE_BINARY_DIR}/dummy.h ${CMAKE_BINARY_DIR}/dummy.cpp)
    set(DUMMY_CPP ${CMAKE_BINARY_DIR}/dummy.cpp)
    set(DUMMY_H ${CMAKE_BINARY_DIR}/dummy.h)

    # Just copy over headers into framework dir, because "official" way of doing it
    # via PUBLIC_HEADER property is full of bugs (see comment on PUBLIC_HEADER property below)
    set(CMAKE_INSTALL_INCLUDEDIR ${CMAKE_INSTALL_LIBDIR}/nakama-sdk.framework/Headers)

endif()

if(WS_IMPL STREQUAL "libhttpclient" OR HTTP_IMPL STREQUAL "libhttpclient")
    if(NOT LIBHTTPCLIENT_BUILTIN)
        if (LINUX OR APPLE)
            find_package(CURL CONFIG REQUIRED)
            target_compile_definitions(CURL::libcurl INTERFACE CURL_STRICTER)
        endif()

        add_subdirectory(third_party/libHttpClient/Utilities/CMake)

        if (NOT WS_IMPL STREQUAL "libhttpclient")
            target_compile_definitions(${LIBHTTPCLIENT_TARGET} PRIVATE "HC_NOWEBSOCKETS")
        else()
            if(LIBHTTPCLIENT_FORCE_WEBSOCKETPP)
                message(STATUS "Forcing libhttpclient to use websocketpp even on modern Windows platforms")
                target_compile_definitions(${LIBHTTPCLIENT_TARGET} PRIVATE "HC_FORCE_WINSOCKETPP")
            endif()
        endif()
        install(TARGETS
                ${LIBHTTPCLIENT_TARGET}
                EXPORT nakama-export
                LIBRARY ARCHIVE RUNTIME
                )
    endif()
endif()

if(WS_IMPL STREQUAL "wslay")
    if (LINUX OR APPLE)
        set(WSLAY_NETIO "CURL")
    endif()
endif()

add_library(nakama-ws-impl INTERFACE)
if(WS_IMPL STREQUAL "libhttpclient")
    set(BUILD_WEBSOCKET_LIBHTTPCLIENT ON)
    set(WS_IMPL_LIB nakama-impl-ws-libhttpclient)
    add_subdirectory(impl/wsLibHttpClient EXCLUDE_FROM_ALL)
elseif(WS_IMPL STREQUAL "wslay")
    set(BUILD_WEBSOCKET_WSLAY ON)
    set(WS_IMPL_LIB nakama-impl-ws-wslay)
    add_subdirectory(impl/wsWslay EXCLUDE_FROM_ALL)
elseif(NOT (WS_IMPL STREQUAL "OFF"))
    message(FATAL_ERROR "Unknown WS_IMPL: ${WS_IMPL}")
elseif(WS_IMPL STREQUAL OFF)
    set(BUILD_WEBSOCKET_IS_BLANK ON)
endif()

if(HTTP_IMPL STREQUAL "libhttpclient")
    set(BUILD_HTTP_LIBHTTPCLIENT ON)
    add_subdirectory(impl/httpLibHttpClient EXCLUDE_FROM_ALL)
elseif(HTTP_IMPL STREQUAL OFF)
    set(BUILD_HTTP_IS_BLANK ON)
endif()

if(NOT BUILD_HTTP_IS_BLANK AND NOT HTTP_IMPL_LIB)
    message(FATAL_ERROR "BUG: CMakeLists.txt in the impl dir must set HTTP_IMPL_LIB with PARENT_SCOPE")
endif()

find_package(optional-lite CONFIG REQUIRED)
# SDK API
add_subdirectory(interface)

# Factories, that's where all messy ifdefs to select concrete implementations are
add_subdirectory(factory EXCLUDE_FROM_ALL)

# Implements API, but without concrete implementations of transports
add_subdirectory(core EXCLUDE_FROM_ALL)

if (NOT LIBHTTPCLIENT_TARGET)
    # A bit of a hack. $<TARGET_EXISTS:""> below makes cmake to error, so
    # we set variable to the value which doesn't match any target
    set(LIBHTTPCLIENT_TARGET doesnt-exist)
endif()

# create dummy file because pure "library of libraries" is not supported

set(NAKAMA_SDK_DEPS
    $<TARGET_OBJECTS:nakama-sdk-core-misc>
    $<TARGET_OBJECTS:nakama-sdk-core-rest>
    $<TARGET_OBJECTS:nakama-sdk-core-rt>
    $<TARGET_OBJECTS:nakama-sdk-core-common>
    $<TARGET_OBJECTS:nakama-sdk-client-factory>
    $<TARGET_OBJECTS:nakama-sdk-rtclient-factory>



    # Xcode generator doesn't produce library for a target with no "own" files
    # https://gitlab.kitware.com/cmake/cmake/-/issues/23688
    ${DUMMY_CPP}

    # library need to have a header so that Headers directory structure is created
    ${DUMMY_H}
)

if (HTTP_IMPL STREQUAL libhttpclient OR WS_IMPL STREQUAL libhttpclient)
    # We build LIBHTTPCLIENT as OBJECT library so that its symbols
    # wont be excluded due to '--exclude-libs ALL' linker flag. Although we
    # dont expose libhttpclient in our API , we still want it's symbols exported
    # in our ABI, because that is how JVM finds native code on Android platform.
    if (NOT LIBHTTPCLIENT_BUILTIN)
        list(APPEND NAKAMA_SDK_DEPS $<TARGET_OBJECTS:${LIBHTTPCLIENT_TARGET}>)
    endif()
endif()

if (BUILD_GRPC_CLIENT)
    list(APPEND NAKAMA_SDK_DEPS $<TARGET_OBJECTS:nakama-sdk-core-grpc>)
endif()

add_library(nakama-sdk ${BUILD_MODE} ${NAKAMA_SDK_DEPS})

set_target_properties(nakama-sdk PROPERTIES
        FRAMEWORK TRUE
        FRAMEWORK_VERSION A

        # we deliberately dont list all real headers, because it is a can of worms. See:
        # - https://gitlab.kitware.com/cmake/cmake/-/issues/16739
        # - https://gitlab.kitware.com/cmake/cmake/-/issues/22760
        # Probably can be removed when proper FILE_SET support lands: https://gitlab.kitware.com/cmake/cmake/-/issues/23386
        PUBLIC_HEADER "${DUMMY_H}"
        MACOSX_FRAMEWORK_IDENTIFIER com.heroiclabs.libnakama
        VERSION ${LIBNAKAMA_VERSION}
        SOVERSION ${LIBNAKAMA_SOVERSION}
        )

add_library(nakama::sdk ALIAS nakama-sdk)
target_link_libraries(nakama-sdk
        PUBLIC nakama::sdk-interface ${PTHREADS_LIB}
        PRIVATE
            nakama-api-proto
            $<$<TARGET_EXISTS:nakama-sdk-core-grpc>:nakama-grpc-proto>
		${HTTP_IMPL_LIB} ${WS_IMPL_LIB}
        ${PLATFORM_LIBS}
        )

if (ADDRESS_SANITIZER)
   if (MSVC)
        target_link_libraries(nakama-sdk INTERFACE
                clang_rt.asan_dynamic-x86_64 clang_rt.asan_dynamic_runtime_thunk-x86_64)
        target_link_directories(nakama-sdk INTERFACE "C:/Program Files/LLVM/lib/clang/13.0.1/lib/windows")
        target_link_options(nakama-sdk INTERFACE /wholearchive:clang_rt.asan_dynamic_runtime_thunk-x86_64.lib)
    else()
        target_link_options(nakama-sdk PRIVATE -fsanitize=address)
        # Uncomment bellow if main executable is NOT compiled with asan (pretty limited diagnostics in this case).
        # target_link_libraries(nakama-sdk PRIVATE asan)
   endif()
endif()

install(TARGETS
            nakama-sdk
            nakama-sdk-interface
            nakama-api-proto
        EXPORT nakama-export
        FRAMEWORK DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT nakama-sdk
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT nakama-sdk
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT nakama-sdk
        RUNTIME DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT nakama-sdk
        FILE_SET HEADERS COMPONENT headers
        INCLUDES DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
)

if(MSVC)
    install(FILES $<TARGET_PDB_FILE:nakama-sdk> DESTINATION "${CMAKE_INSTALL_LIBDIR}" COMPONENT nakama-sdk OPTIONAL)
endif()

# See recommendations here:
# https://gitlab.kitware.com/cmake/cmake/-/issues/18453
install(EXPORT nakama-export
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/nakama)


include(CMakePackageConfigHelpers)

configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/nakama-sdk-config.cmake.in
  "${CMAKE_CURRENT_BINARY_DIR}/nakama-sdk-config.cmake"
  INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/nakama
  NO_SET_AND_CHECK_MACRO
  NO_CHECK_REQUIRED_COMPONENTS_MACRO
  )

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/nakama-sdk-config.cmake
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/nakama
)

# Call last so that it can reference any target previously defined
include(cmake/linkerFlagsTargets.cmake)
