# I think we are constrained to the version that the latest android sdk uses, for Android builds.
cmake_minimum_required(VERSION 3.22.1)
include(version.cmake)

set(CMAKE_CONFIGURATION_TYPES "Debug;MinSizeRel")
set(VCPKG_OVERLAY_TRIPLETS "${CMAKE_CURRENT_SOURCE_DIR}/cmake/triplets")
set(CMAKE_SUPPRESS_REGENERATION true) ## for Xcode
set(VCPKG_PREFER_SYSTEM_LIBS OFF)

## TODO figure out why vcpkg seems to toggle these on and prevent downloading from fetchcontent
set(FETCHCONTENT_FULLY_DISCONNECTED OFF)
set(FETCHCONTENT_UPDATES_DISCONNECTED OFF)

if(NOT BUILD_SHARED_LIBS)
    message(WARNING "The Nakama C++ client can only be built as a shared library.")
    set(BUILD_SHARED_LIBS ON)
endif()

# By default, assume user will not obtain optional-lite via third party mechanism.
option(DISTRIBUTE_OPTIONAL_LITE "Distribute optional lite with the Nakama client." OFF)

option(UNREAL "Unreal engine support" OFF)

set(HTTP_IMPL "")
set(WS_IMPL "")
set(NAKAMA_SDK_DEPS "")

if (UNREAL)
    # too many bugs in the Unreal Linux WS implementation:
    # https://github.com/EpicGames/UnrealEngine/pull/7267 and
    # https://github.com/EpicGames/UnrealEngine/pull/9103
    message(NOTICE "Disabling HTTP transport in Unreal build mode")

    if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
        set(WS_IMPL "wslay")
    else()
        message(NOTICE "Disabling WS transport in Unreal build mode")
    endif()
else()
    if (WITH_LIBHTTPCLIENT_HTTP)
        set(HTTP_IMPL "libhttpclient")
    elseif (WITH_CPPRESTSDK)
        set(HTTP_IMPL "cpprestsdk")
    elseif (BUILD_CURL_HTTP)
        set(HTTP_IMPL "curl")
    endif()

    if (WITH_LIBHTTPCLIENT_WS)
        set(WS_IMPL "libhttpclient")
    elseif (BUILD_WSLAY)
        set(WS_IMPL "wslay")
    elseif (WITH_CPPRESTSDK)
        set(WS_IMPL "cpprestsdk")
    endif()
endif()

if (DEFINED ANDROID_ABI)
    if (DEFINED VCPKG_CHAINLOAD_TOOLCHAIN_FILE)
        message(FATAL_ERROR "Cannot specify an external chainload toolchain if building for Android.")
    endif()

    set(VCPKG_CHAINLOAD_TOOLCHAIN_FILE $ENV{ANDROID_NDK_HOME}/build/cmake/android.toolchain.cmake)
endif()

cmake_policy(SET CMP0012 NEW)

## force so that gradle doesn't stomp vcpkg toolchain with the android toolchain
set(CMAKE_TOOLCHAIN_FILE $ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake CACHE FILEPATH "Vcpkg toolchain file" FORCE)

if (DEFINED ANDROID_ABI AND NOT DEFINED VCPKG_CHAINLOAD_TOOLCHAIN_FILE)
    # CmakePresets.json doesn't allow us to use cmake vars in values, so set it here
    set(VCPKG_CHAINLOAD_TOOLCHAIN_FILE $ENV{ANDROID_NDK_HOME}/build/cmake/android.toolchain.cmake)
endif()

project(nakama-sdk LANGUAGES CXX)

include(cmake/platformSupport.cmake)
include(cmake/compilerFlags.cmake)
include(cmake/linkerFlags.cmake)
include(GNUInstallDirs)
include(FetchContent)

if(MSVC)
    # On windows it is critical not to accidentally use DLL
    # compiled with "wrong" runtime (/MD, /MDd) so we name them differently
    set(CMAKE_DEBUG_POSTFIX "_d")
endif()

set(CMAKE_CXX_STANDARD 17)
option(LOGS_ENABLED "Enable log output" ON)

if(NOT MSVC)
    string(APPEND CMAKE_CXX_FLAGS " -fexceptions")
endif()

option(ADDRESS_SANITIZER "Enable address sanitizer (broken on windows)" OFF)

# libhttpclient uses websocketpp when system winhttp doesn't provide one (< Windows 8)
# this flag allows us to force use of websocketpp so that it can be tested on current Windows versions
option(LIBHTTPCLIENT_FORCE_WEBSOCKETPP "Force use of websocketpp instead of winhttp in the libhttpclient" OFF)

if (LIBHTTPCLIENT_FORCE_WEBSOCKETPP)
    list(APPEND VCPKG_MANIFEST_FEATURES "websocketpp")
endif()

if(LOGS_ENABLED)
    add_compile_definitions(NLOGS_ENABLED)
endif(LOGS_ENABLED)

if (ADDRESS_SANITIZER)
    add_compile_options(-fsanitize=address -O1 -fno-optimize-sibling-calls -fno-omit-frame-pointer)
    string(REPLACE /RTC1 "" CMAKE_CXX_FLAGS_DEBUG ${CMAKE_CXX_FLAGS_DEBUG})
endif()

if(BUILD_SHARED_LIBS)
    set(CMAKE_POSITION_INDEPENDENT_CODE ON)
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
    if (NOT LIBHTTPCLIENT_BUILTIN)
        if (LINUX OR APPLE)
            find_package(CURL CONFIG REQUIRED)
            target_compile_definitions(CURL::libcurl INTERFACE CURL_STRICTER)
        endif()

        if (ANDROID)
            set(LIBHTTPCLIENT_SOURCE_SUBDIR Utilities/CMake/Android/libHttpClient)
            set(LIBHTTPCLIENT_TARGET libHttpClient.Android)
        else()
            set(LIBHTTPCLIENT_SOURCE_SUBDIR Utilities/CMake)

            if(XDK)
                if (WINRT)
                    set(LIBHTTPCLIENT_TARGET libHttpClient.XDK.WinRT)
                else()
                    set(LIBHTTPCLIENT_TARGET libHttpClient.XDK.C)
                endif()
            elseif (BUILDWIN32)
                set(LIBHTTPCLIENT_TARGET libHttpClient.Win32.C)
            elseif (GDK)
                set(LIBHTTPCLIENT_TARGET libHttpClient.GDK.C)
            elseif (APPLE)
                set(LIBHTTPCLIENT_TARGET libHttpClient.Apple.C)
            elseif(DEFINED LINUX)
                set(LIBHTTPCLIENT_TARGET libHttpClient.Linux.C)
            else()
                if (WINRT)
                    set(LIBHTTPCLIENT_TARGET libHttpClient.UWP.WinRT)
                else()
                    set(LIBHTTPCLIENT_TARGET libHttpClient.UWP.C)
                endif()
            endif()
        endif()

        # We build LIBHTTPCLIENT as OBJECT library so that its symbols
        # wont be excluded due to '--exclude-libs ALL' linker flag. Although we
        # dont expose libhttpclient in our API , we still want it's symbols exported
        # in our ABI, because that is how JVM finds native code on Android platform.
        list(APPEND NAKAMA_SDK_DEPS $<TARGET_OBJECTS:${LIBHTTPCLIENT_TARGET}>)

        set(LIBHTTPCLIENT_OBJECT_LIBRARY TRUE)

        FetchContent_Declare(
            libHttpClient
            GIT_REPOSITORY https://github.com/heroiclabs/libHttpClient.git
            GIT_TAG        c535d5943516bb5d8f3a50858da617ebd0dd3f3f
            SOURCE_SUBDIR  ${LIBHTTPCLIENT_SOURCE_SUBDIR}
            GIT_PROGRESS TRUE
            GIT_SHALLOW TRUE
            USES_TERMINAL_DOWNLOAD TRUE
        )

        FetchContent_MakeAvailable(libHttpClient)

        if (NOT WS_IMPL STREQUAL "libhttpclient")
            target_compile_definitions(${LIBHTTPCLIENT_TARGET} PRIVATE "HC_NOWEBSOCKETS")
        else()
            if (LIBHTTPCLIENT_FORCE_WEBSOCKETPP)
                message(STATUS "Forcing libhttpclient to use websocketpp even on modern Windows platforms")
                target_compile_definitions(${LIBHTTPCLIENT_TARGET} PRIVATE "HC_FORCE_WINSOCKETPP")
            endif()
        endif()
    endif()
endif()

if (HTTP_IMPL STREQUAL "cpprestsdk")
    add_subdirectory(impl/httpCppRest EXCLUDE_FROM_ALL)
elseif(HTTP_IMPL STREQUAL "curl")
    add_subdirectory(impl/httpCurl EXCLUDE_FROM_ALL)
endif()

add_library(nakama-ws-impl INTERFACE)
if(WS_IMPL STREQUAL "libhttpclient")
    set(WS_IMPL_LIB nakama-impl-ws-libhttpclient)
    add_subdirectory(impl/wsLibHttpClient EXCLUDE_FROM_ALL)
elseif(WS_IMPL STREQUAL "wslay")
    set(BUILD_WEBSOCKET_WSLAY ON)
    set(WS_IMPL_LIB nakama-impl-ws-wslay)
    add_subdirectory(impl/wsWslay EXCLUDE_FROM_ALL)
elseif(WS_IMPL STREQUAL "cpprestsdk")
    set(WS_IMPL_LIB nakama-impl-ws-cppRest)
    add_subdirectory(impl/wsCppRest EXCLUDE_FROM_ALL)
endif()

if(HTTP_IMPL STREQUAL "libhttpclient")
    set(BUILD_HTTP_LIBHTTPCLIENT ON)
    add_subdirectory(impl/httpLibHttpClient EXCLUDE_FROM_ALL)
endif()

find_package(optional-lite CONFIG REQUIRED)
# SDK API
add_subdirectory(interface)

# Factories, that's where all messy ifdefs to select concrete implementations are
add_subdirectory(factory EXCLUDE_FROM_ALL)

# Implements API, but without concrete implementations of transports
add_subdirectory(core EXCLUDE_FROM_ALL)

# create dummy file because pure "library of libraries" is not supported

list(APPEND NAKAMA_SDK_DEPS
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

if (BUILD_GRPC_CLIENT)
    list(APPEND NAKAMA_SDK_DEPS $<TARGET_OBJECTS:nakama-sdk-core-grpc>)
endif()

## TODO this can be massively simplified
if (WITH_LIBHTTPCLIENT_WS)
    target_compile_definitions(nakama-sdk-core-rt PUBLIC BUILD_WEBSOCKET_LIBHTTPCLIENT)
    target_compile_definitions(nakama-sdk-core-common PUBLIC BUILD_WEBSOCKET_LIBHTTPCLIENT)
    target_compile_definitions(nakama-sdk-rtclient-factory PUBLIC BUILD_WEBSOCKET_LIBHTTPCLIENT)
    target_compile_definitions(nakama-sdk-interface INTERFACE BUILD_WEBSOCKET_LIBHTTPCLIENT)
elseif (BUILD_WEBSOCKET_WSLAY)
    target_compile_definitions(nakama-sdk-core-rt PUBLIC BUILD_WEBSOCKET_WSLAY)
    target_compile_definitions(nakama-sdk-core-common PUBLIC BUILD_WEBSOCKET_WSLAY)
    target_compile_definitions(nakama-sdk-rtclient-factory PUBLIC BUILD_WEBSOCKET_WSLAY)
    target_compile_definitions(nakama-sdk-interface INTERFACE BUILD_WEBSOCKET_WSLAY)
elseif (WITH_CPPRESTSDK)
    target_compile_definitions(nakama-sdk-core-rt PUBLIC BUILD_WEBSOCKET_CPPRESTSDK)
    target_compile_definitions(nakama-sdk-core-common PUBLIC BUILD_WEBSOCKET_CPPRESTSDK)
    target_compile_definitions(nakama-sdk-rtclient-factory PUBLIC BUILD_WEBSOCKET_CPPRESTSDK)
    target_compile_definitions(nakama-sdk-interface INTERFACE BUILD_WEBSOCKET_CPPRESTSDK)
else()
    target_compile_definitions(nakama-sdk-core-rt PUBLIC WITH_EXTERNAL_WS)
    target_compile_definitions(nakama-sdk-core-common PUBLIC WITH_EXTERNAL_WS)
    target_compile_definitions(nakama-sdk-rtclient-factory PUBLIC WITH_EXTERNAL_WS)
    target_compile_definitions(nakama-sdk-interface INTERFACE WITH_EXTERNAL_WS)
endif()

add_library(nakama-sdk ${NAKAMA_SDK_DEPS})

set_target_properties(nakama-sdk PROPERTIES
        FRAMEWORK TRUE
        FRAMEWORK_VERSION A

        # we deliberately dont list all real headers, because it is a can of worms. See:
        # - https://gitlab.kitware.com/cmake/cmake/-/issues/16739
        # - https://gitlab.kitware.com/cmake/cmake/-/issues/22760
        # Probably can be removed when proper FILE_SET support lands: https://gitlab.kitware.com/cmake/cmake/-/issues/23386
        PUBLIC_HEADER "${DUMMY_H}"
        MACOSX_FRAMEWORK_IDENTIFIER com.heroiclabs.libnakama
        MACOSX_FRAMEWORK_BUNDLE_VERSION ${LIBNAKAMA_VERSION}
        MACOSX_FRAMEWORK_SHORT_VERSION_STRING ${LIBNAKAMA_VERSION}
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

## use share directory for export files (at least a vcpkg convention)
install(EXPORT nakama-export
        DESTINATION share/nakama-sdk)


include(CMakePackageConfigHelpers)

configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/nakama-sdk-config.cmake.in
  "${CMAKE_CURRENT_BINARY_DIR}/nakama-sdk-config.cmake"
  INSTALL_DESTINATION share/nakama-sdk
  NO_SET_AND_CHECK_MACRO
  NO_CHECK_REQUIRED_COMPONENTS_MACRO
  )

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/nakama-sdk-config.cmake
  DESTINATION share/nakama-sdk
)

if (DISTRIBUTE_OPTIONAL_LITE)
    install(DIRECTORY
        ${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/include/nonstd
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )

    file(GLOB OPTIONAL_LITE_CONFIGS "${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/share/optional-lite/*.cmake")
    install(FILES ${OPTIONAL_LITE_CONFIGS} DESTINATION ${CMAKE_INSTALL_PREFIX}/share/optional-lite)
endif()

# Call last so they can reference any target previously defined
include(cmake/linkerFlagsTargets.cmake)
include(cmake/localCompilerFlags.cmake)

if (ANDROID AND HTTP_IMPL STREQUAL "curl")
    set(CREATE_JAR TRUE) # we need the code for accessing native Android CA store.
else()
    set(CREATE_JAR FALSE)
endif()

if (CREATE_JAR)
    message("-- Bundling Java code...")

    execute_process(COMMAND ./gradlew createJar
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/android
        OUTPUT_VARIABLE GRADLE_OUTPUT
        ERROR_VARIABLE GRADLE_ERROR
        COMMAND_ERROR_IS_FATAL ANY
        COMMAND_ECHO STDOUT
    )

    message("-- Gradle Output: ${GRADLE_OUTPUT}")
    set(INPUT_JAR ${CMAKE_SOURCE_DIR}/android/app/build/libs/libnakama-sdk.jar)
    set(OUTPUT_JAR_DIR ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR})

    install(
        CODE
        "execute_process(
            COMMAND
            ${CMAKE_COMMAND}
            -DINPUT_JAR=${INPUT_JAR}
            -DOUTPUT_JAR_DIR=${OUTPUT_JAR_DIR}
            -P
            ${CMAKE_SOURCE_DIR}/cmake/androidPostInstall.cmake
        )"
    )
endif()