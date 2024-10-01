# I think we are constrained to the version that the latest android sdk uses, for Android builds.
cmake_minimum_required(VERSION 3.22.1)
include(version.cmake)
include(cmake/checkPresets.cmake)

if (CMAKE_OSX_SYSROOT STREQUAL "iphonesimulator")
    # this hack seems to be required for now, otherwise CMake still thinks we are building for arm...
    set(CMAKE_XCODE_ATTRIBUTE_EXCLUDED_ARCHS[sdk=iphonesimulator*] "arm64")
endif()

if (ANDROID)
    SET(ANDROID_USE_LEGACY_TOOLCHAIN_FILE FALSE)
endif()

set(CMAKE_CONFIGURATION_TYPES "Debug;MinSizeRel")
set(CMAKE_SUPPRESS_REGENERATION true) ## for Xcode

## TODO figure out why vcpkg seems to toggle these on and prevent downloading from fetchcontent
set(FETCHCONTENT_FULLY_DISCONNECTED OFF)
set(FETCHCONTENT_UPDATES_DISCONNECTED OFF)

# By default, assume user will not obtain optional-lite via third party mechanism.
option(DISTRIBUTE_OPTIONAL_LITE "Distribute optional lite with the Nakama client." ON)
option(BUILD_SHARED_LIBS "Build a shared library." ON)
option(FORCE_DLL_IMPORT_EXPORT "Force exporting and importing of symbols regardless of the platform." OFF)
option(BUILD_APPLE_FRAMEWORK "Build frameworks instead of .dylib files for Apple." ON)

set(HTTP_IMPL "")
set(WS_IMPL "")
set(NAKAMA_SDK_DEPS "")

if (WITH_LIBHTTPCLIENT_HTTP)
    set(HTTP_IMPL "libhttpclient")
elseif (WITH_CPPRESTSDK)
    set(HTTP_IMPL "cpprestsdk")
elseif (BUILD_CURL_HTTP OR BUILD_CURL_HTTP_SYSTEM)
    set(HTTP_IMPL "curl")
endif()

if (WITH_LIBHTTPCLIENT_WS)
    set(WS_IMPL "libhttpclient")
elseif (BUILD_WSLAY)
    set(WS_IMPL "wslay")
elseif (WITH_CPPRESTSDK)
    set(WS_IMPL "cpprestsdk")
endif()

set(HTTP_COMPILE_DEFINITION "")

if(WITH_LIBHTTPCLIENT_HTTP)
    set(HTTP_COMPILE_DEFINITION "BUILD_HTTP_LIBHTTPCLIENT")
elseif(WITH_CPPRESTSDK)
    set(HTTP_COMPILE_DEFINITION "BUILD_HTTP_CPPRESTSDK")
elseif(BUILD_CURL_HTTP OR BUILD_CURL_HTTP_SYSTEM)
    set(HTTP_COMPILE_DEFINITION "BUILD_HTTP_CURL")
else()
    set(HTTP_COMPILE_DEFINITION "WITH_EXTERNAL_HTTP")
endif()

set(WS_COMPILE_DEFINITION "")
set(WS_IO_COMPILE_DEFINITION "")

if (WITH_LIBHTTPCLIENT_WS)
    set(WS_COMPILE_DEFINITION BUILD_WEBSOCKET_LIBHTTPCLIENT)
elseif (BUILD_WSLAY)
    set(WS_COMPILE_DEFINITION BUILD_WEBSOCKET_WSLAY)

    if (BUILD_CURL_IO OR BUILD_CURL_IO_SYSTEM)
        set(WS_IO_COMPILE_DEFINITION BUILD_CURL_IO)
    else()
        # no IO baked in for Wslay -- we expect user to provide the IO.
        set(WS_IO_COMPILE_DEFINITION BUILD_IO_EXTERNAL)
    endif()
elseif (WITH_CPPRESTSDK)
    set(WS_COMPILE_DEFINITION BUILD_WEBSOCKET_CPPRESTSDK)
else()
    # no WS baked in -- we expect user to provide WS transport
    set(WS_COMPILE_DEFINITION WITH_EXTERNAL_WS)
    set(WS_IO_COMPILE_DEFINITION WITH_EXTERNAL_WS_IO)
endif()

cmake_policy(SET CMP0012 NEW)

## force so that gradle doesn't stomp vcpkg toolchain with the android toolchain
set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_LIST_DIR}/submodules/vcpkg/scripts/buildsystems/vcpkg.cmake CACHE FILEPATH "Vcpkg toolchain file" FORCE)

if (DEFINED ANDROID_ABI AND NOT DEFINED VCPKG_CHAINLOAD_TOOLCHAIN_FILE)
    # CmakePresets.json doesn't allow us to use cmake vars in values, so set it here
    set(VCPKG_CHAINLOAD_TOOLCHAIN_FILE $ENV{ANDROID_NDK_HOME}/build/cmake/android.toolchain.cmake)
endif()

include("submodules/devkits/cmake/platform-${VCPKG_TARGET_TRIPLET}.cmake" OPTIONAL)

project(nakama-sdk LANGUAGES CXX)

include(cmake/platformSupport.cmake)
include(cmake/compilerFlags.cmake)
include(cmake/linkerFlags.cmake)
include(GNUInstallDirs)
include(FetchContent)

include(InstallRequiredSystemLibraries)

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


if(APPLE AND BUILD_SHARED_LIBS AND BUILD_APPLE_FRAMEWORK)
    # Just copy over headers into framework dir, because "official" way of doing it
    # via PUBLIC_HEADER property is full of bugs (see comment on PUBLIC_HEADER property below)
    set(CMAKE_INSTALL_INCLUDEDIR ${CMAKE_INSTALL_LIBDIR}/nakama-sdk.framework/Headers)

endif()

if (BUILD_CURL_IO_SYSTEM OR BUILD_CURL_IO OR BUILD_CURL_HTTP_SYSTEM OR BUILD_CURL_HTTP)
    if (BUILD_CURL_IO_SYSTEM OR BUILD_CURL_HTTP_SYSTEM)
        find_package(CURL REQUIRED) # system curl not guaranteed to contain a CMake config file.
    else()
        find_package(CURL CONFIG REQUIRED)
    endif()
endif()

if(WS_IMPL STREQUAL "libhttpclient" OR HTTP_IMPL STREQUAL "libhttpclient")
    if (NOT LIBHTTPCLIENT_BUILTIN)
        include(cmake/libhttpClient.cmake)
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

if (ANDROID AND (HTTP_IMPL STREQUAL "curl" OR WS_IMPL STREQUAL "wslay"))
    add_subdirectory(impl/android)
    set(CREATE_JAR TRUE) # we need the code for accessing native Android CA store.

    if (HTTP_IMPL STREQUAL "curl")
        target_link_libraries(nakama-impl-http-libcurl PUBLIC android-ca)
    endif()

    if (WS_IMPL STREQUAL "wslay")
        target_link_libraries(nakama-impl-ws-wslay INTERFACE android-ca)
    endif()

    list(APPEND NAKAMA_SDK_DEPS android-ca)
else()
    set(CREATE_JAR FALSE)
endif()

if (CREATE_JAR)
    message("-- Configured to build Java code with Gradle.")

    set(LIBNAKAMA_AAR "${CMAKE_SOURCE_DIR}/android/app/build/outputs/aar/libnakama-sdk.aar")
    # TODO it's very odd that add_custom_command did not seem to trigger, so we use add_custom_target
    add_custom_target(
        nakama-aar ALL
        COMMAND ./gradlew assemble
        BYPRODUCTS ${LIBNAKAMA_AAR}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/android
        VERBATIM
        USES_TERMINAL
        DEPENDS ${CMAKE_SOURCE_DIR}/test/android/src/main/java/com/heroiclabs/nakamatest/MainActivity.java
    )

    install(FILES ${LIBNAKAMA_AAR} DESTINATION ${CMAKE_INSTALL_LIBDIR})
endif()

find_package(optional-lite CONFIG REQUIRED)
# SDK API
add_subdirectory(interface)

# Factories, that's where all messy ifdefs to select concrete implementations are
add_subdirectory(factory EXCLUDE_FROM_ALL)

# Implements API, but without concrete implementations of transports
add_subdirectory(core EXCLUDE_FROM_ALL)

# create dummy file because pure "library of libraries" is not supported: https://gitlab.kitware.com/cmake/cmake/-/issues/23688
set(DUMMY_CPP "")
set(DUMMY_H "")

if (APPLE)
    file(TOUCH ${CMAKE_BINARY_DIR}/dummy.h ${CMAKE_BINARY_DIR}/dummy.cpp)
    set(DUMMY_CPP ${CMAKE_BINARY_DIR}/dummy.cpp)
    set(DUMMY_H ${CMAKE_BINARY_DIR}/dummy.h)
endif()

list(APPEND NAKAMA_SDK_DEPS
    nakama-sdk-core-misc
    nakama-sdk-core-rest
    nakama-sdk-core-rt
    nakama-sdk-core-common
    nakama-sdk-client-factory
    nakama-sdk-rtclient-factory
)

if (BUILD_GRPC_CLIENT)
    list(APPEND NAKAMA_SDK_DEPS nakama-sdk-core-grpc)
endif()

set(NAKAMA_SDK_DEPS_OBJECTS "")

foreach(lib ${NAKAMA_SDK_DEPS})
  # Append the TARGET_OBJECTS of the current library to the list
  list(APPEND NAKAMA_SDK_DEPS_OBJECTS $<TARGET_OBJECTS:${lib}>)
endforeach()

add_library(nakama-sdk ${NAKAMA_SDK_DEPS_OBJECTS} ${DUMMY_CPP} ${DUMMY_H})

if (BUILD_SHARED_LIBS AND APPLE AND BUILD_APPLE_FRAMEWORK)
    set_target_properties(nakama-sdk PROPERTIES
            FRAMEWORK TRUE
            FRAMEWORK_VERSION A

            # we deliberately dont list all real headers, because it is a can of worms. See:
            # - https://gitlab.kitware.com/cmake/cmake/-/issues/16739
            # - https://gitlab.kitware.com/cmake/cmake/-/issues/22760
            MACOSX_FRAMEWORK_IDENTIFIER com.heroiclabs.libnakama
            MACOSX_FRAMEWORK_BUNDLE_VERSION ${LIBNAKAMA_VERSION}
            MACOSX_FRAMEWORK_SHORT_VERSION_STRING ${LIBNAKAMA_VERSION}
    )
elseif (APPLE)
    set_target_properties(nakama-sdk PROPERTIES
    FRAMEWORK FALSE)
endif()

add_library(nakama::sdk ALIAS nakama-sdk)
find_package(Threads REQUIRED)
set(PTHREADS_LIB Threads::Threads)
target_link_libraries(nakama-sdk
        PUBLIC
            nakama::sdk-interface
            ${PTHREADS_LIB}
)

set(DEPENDENCY_LINK_SCOPE "")
if (BUILD_SHARED_LIBS)
    set(DEPENDENCY_LINK_SCOPE "PRIVATE")
else() # propogate the static dependencies
    set(DEPENDENCY_LINK_SCOPE "PUBLIC")
endif()

target_link_libraries(nakama-sdk
    ${DEPENDENCY_LINK_SCOPE}
    nakama-api-proto
    $<$<TARGET_EXISTS:nakama-sdk-core-grpc>:nakama-grpc-proto>
    ${HTTP_IMPL_LIB} ${WS_IMPL_LIB}
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


# ensure we only perform a single 'target_link_libraries' to libcurl to prevent duplicate linkage for linkers that do not deduplicate.
if (BUILD_CURL_IO_SYSTEM OR BUILD_CURL_IO OR BUILD_CURL_HTTP_SYSTEM OR BUILD_CURL_HTTP)
    target_link_libraries(nakama-sdk PRIVATE CURL::libcurl)
endif()

install(TARGETS
            nakama-sdk
            nakama-sdk-interface
            nakama-api-proto
            ${HTTP_IMPL_LIB}
            ${WS_IMPL_LIB}
            ${NAKAMA_SDK_DEPS}
        EXPORT nakama-export
        FRAMEWORK DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT nakama-sdk
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT nakama-sdk
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT nakama-sdk
        RUNTIME DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT nakama-sdk
        FILE_SET HEADERS COMPONENT headers
        INCLUDES DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
)

if(MSVC AND BUILD_SHARED_LIBS)
    install(FILES $<TARGET_PDB_FILE:nakama-sdk> DESTINATION "${CMAKE_INSTALL_LIBDIR}" COMPONENT nakama-sdk OPTIONAL)
endif()

## use share directory for export files (at least a vcpkg convention)
install(EXPORT nakama-export
        DESTINATION share/nakama-sdk)


include(CMakePackageConfigHelpers)

if(BUILD_SHARED_LIBS)
    set(NAKAMA_SDK_BUILD_TYPE "SHARED")
else()
    set(NAKAMA_SDK_BUILD_TYPE "STATIC")
endif()

configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/nakama-sdk-config.cmake.in
  "${CMAKE_CURRENT_BINARY_DIR}/nakama-sdk-config.cmake"
  INSTALL_DESTINATION share/nakama-sdk
  NO_SET_AND_CHECK_MACRO
  NO_CHECK_REQUIRED_COMPONENTS_MACRO
)

## for any variables that need to be read by consuming packages before their project() call, since find_package must be done after project()
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/nakama-sdk-preconfig.cmake.in ${CMAKE_BINARY_DIR}/nakama-sdk-preconfig.cmake)

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/nakama-sdk-preconfig.cmake
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
include(cmake/localCompilerFlags.cmake)
