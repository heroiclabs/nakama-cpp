# I think we are constrained to the version that the latest android sdk uses, for Android builds.
cmake_minimum_required(VERSION 3.29.3)
include(version.cmake)
include(cmake/checkPresets.cmake)

option(WITH_HTTP_CURL "Use CURL for HTTP transport" OFF)
option(WITH_HTTP_LIBHTTPC "Use libHttpClient for HTTP transport" OFF)
option(WITH_HTTP_CPPREST "Use CppRestSDK for HTTP transport" OFF)

option(WITH_WS_WSLAY "Use wslay for WS transport" OFF)
option(WITH_WS_LIBHTTPC "Use libHttpClient for WS transport" OFF)
option(WITH_WS_CPPREST "Use CppRestSDK for WS transport" OFF)

option(CFG_WSLAY_CURL_IO "Use CURL-based NetIO when wslay is enabled" ON)
option(CFG_LIBHTTPC_SYSTEM "Use system-installed libhttpc" OFF)
option(CFG_CURL_SYSTEM "Use system-installed libCURL" OFF)

option(WITH_GRPC_CLIENT "Build gRPC Client" OFF)

option(BUILD_TESTING "Build tests" OFF)

option(RTTI "Enable RTTI on classes which could be inherited by app" ON)

if (INSIDE_GRADLE)
    # We are called from gradle. Do what normally CMakePresets do and
    # select appropriate triplet, override toolchain file, etc.
    string(REPLACE "_" "-" ANDROID_ABI_TRIPLET ${ANDROID_ABI})
    set(VCPKG_TARGET_TRIPLET "${ANDROID_ABI_TRIPLET}-android-heroic")
    set(VCPKG_CHAINLOAD_TOOLCHAIN_FILE "${CMAKE_TOOLCHAIN_FILE}")
    set(VCPKG_OVERLAY_PORTS "${CMAKE_CURRENT_LIST_DIR}/cmake/vcpkg-ports")
    set(VCPKG_OVERLAY_TRIPLETS "${CMAKE_CURRENT_LIST_DIR}/cmake/triplets")
    # This env var is used by vcpkg's android toolchain
    set(ENV{ANDROID_NDK_HOME} "${ANDROID_NDK}")

    set(WITH_HTTP_CURL ON)
    set(WITH_WS_WSLAY ON)
endif()


if (WITH_WS_WSLAY)
    list(APPEND VCPKG_MANIFEST_FEATURES "wslay")
endif()

if (WITH_HTTP_CURL OR (WITH_WS_WSLAY AND CFG_WSLAY_CURL_IO))
    if (NOT CFG_CURL_SYSTEM)
        list(APPEND VCPKG_MANIFEST_FEATURES "curl")
    endif()
endif()

if ((WITH_WS_LIBHTTPC OR WITH_HTTP_LIBHTTPC) AND NOT CFG_LIBHTTPC_SYSTEM)
    list(APPEND VCPKG_MANIFEST_FEATURES "libhttpclient")
endif()

if (WITH_HTTP_CPPREST OR WITH_WS_CPPREST)
    list(APPEND VCPKG_MANIFEST_FEATURES "cpprestsdk")
endif()

if (WITH_HTTP_LIBHTTPC OR WITH_HTTP_CURL OR WITH_HTTP_CPPREST)
    # This var goes into public config.h, so set it early
    set(HAVE_DEFAULT_TRANSPORT_FACTORY ON)
endif()

if (WITH_WS_LIBHTTPC OR WITH_WS_WSLAY OR WITH_WS_CPPREST)
    # This var goes into public config.h, so set it early
    set(HAVE_DEFAULT_RT_TRANSPORT_FACTORY ON)
endif()

set(CMAKE_CONFIGURATION_TYPES "Debug;MinSizeRel")
set(CMAKE_SUPPRESS_REGENERATION true) ## for Xcode

option(FORCE_DLL_IMPORT_EXPORT "Force exporting and importing of symbols regardless of the platform." OFF)
option(BUILD_APPLE_FRAMEWORK "Build frameworks instead of .dylib files for Apple." ON)

set(NAKAMA_SDK_DEPS)

cmake_policy(SET CMP0012 NEW)

## force so that gradle doesn't stomp vcpkg toolchain with the android toolchain
set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_LIST_DIR}/submodules/vcpkg/scripts/buildsystems/vcpkg.cmake CACHE FILEPATH "Vcpkg toolchain file" FORCE)

include("cmake/triplets/${VCPKG_TARGET_TRIPLET}.cmake" OPTIONAL)
include("submodules/devkits/cmake/platform-${VCPKG_TARGET_TRIPLET}.cmake" OPTIONAL)
include("submodules/private/cmake/preproject-${VCPKG_TARGET_TRIPLET}.cmake" OPTIONAL)

project(nakama-sdk LANGUAGES CXX)

include(cmake/platformSupport.cmake)
include(cmake/compilerFlags.cmake)
include(cmake/linkerFlags.cmake)
include(GNUInstallDirs)
include(FetchContent)

set(CMAKE_CXX_STANDARD 17)
option(LOGS_ENABLED "Enable log output" ON)

if(NOT MSVC)
    string(APPEND CMAKE_CXX_FLAGS " -fexceptions")
endif()

option(ADDRESS_SANITIZER "Enable address sanitizer (broken on windows)" OFF)

if(LOGS_ENABLED)
    add_compile_definitions(NLOGS_ENABLED)
endif(LOGS_ENABLED)

if (ADDRESS_SANITIZER)
    add_compile_options(-fsanitize=address -O1 -fno-optimize-sibling-calls -fno-omit-frame-pointer)
    string(REPLACE /RTC1 "" CMAKE_CXX_FLAGS_DEBUG ${CMAKE_CXX_FLAGS_DEBUG})
endif()

add_compile_definitions(NAKAMA_SHARED_LIBRARY_EXPORTS)

if(APPLE AND BUILD_APPLE_FRAMEWORK)
    # Just copy over headers into framework dir, because "official" way of doing it
    # via PUBLIC_HEADER property is full of bugs (see comment on PUBLIC_HEADER property below)
    set(CMAKE_INSTALL_INCLUDEDIR ${CMAKE_INSTALL_LIBDIR}/nakama-sdk.framework/Headers)
endif()

if (WITH_HTTP_CURL OR (WITH_WS_WSLAY AND CFG_WSLAY_CURL_IO))
    if (CFG_CURL_SYSTEM)
        find_package(CURL REQUIRED) # system curl not guaranteed to contain a CMake config file.
    else()
        find_package(CURL CONFIG REQUIRED)
    endif()
endif()


if(WITH_HTTP_LIBHTTPC OR WITH_WS_LIBHTTPC)
    # build libHttpClient dependency if not using system
    if (NOT CFG_LIBHTTPC_SYSTEM)
        include(cmake/libhttpClient.cmake)
    endif()
endif()

# unconditional
find_package(RapidJSON CONFIG REQUIRED)
set_property(TARGET rapidjson APPEND PROPERTY
        # on consoles rapidjson fail to detect C++11 strings presence
        # we know we have them, so set them unconditionally
        INTERFACE_COMPILE_DEFINITIONS
            RAPIDJSON_HAS_STDSTRING=1
            RAPIDJSON_HAS_CXX11_RVALUE_REFS=1
)

# SDK API
add_subdirectory(interface)

# Implements API, but without concrete implementations of transports
add_subdirectory(core EXCLUDE_FROM_ALL)

# Factories, that's where all messy ifdefs to select concrete implementations are
add_subdirectory(factory EXCLUDE_FROM_ALL)

add_subdirectory(satori-cpp)

include("submodules/private/cmake/target-${VCPKG_TARGET_TRIPLET}.cmake" OPTIONAL)

if (WITH_HTTP_CPPREST)
    set(HTTP_IMPL_LIB nakama::impl-http-cppRest)
    add_subdirectory(impl/httpCppRest EXCLUDE_FROM_ALL)
elseif(WITH_HTTP_CURL)
    set(HTTP_IMPL_LIB nakama::impl-http-libcurl)
    add_subdirectory(impl/httpCurl EXCLUDE_FROM_ALL)
elseif(WITH_HTTP_LIBHTTPC)
    set(HTTP_IMPL_LIB nakama::impl-http-libhttpclient)
    add_subdirectory(impl/httpLibHttpClient EXCLUDE_FROM_ALL)
endif()
if (HTTP_IMPL_LIB)
    target_link_libraries(nakama-sdk-client-factory
            PRIVATE ${HTTP_IMPL_LIB}
            INTERFACE $<TARGET_OBJECTS:${HTTP_IMPL_LIB}>)
endif()

if(WITH_WS_LIBHTTPC)
    set(WS_IMPL_LIB nakama::impl-ws-libhttpclient)
    add_subdirectory(impl/wsLibHttpClient EXCLUDE_FROM_ALL)
elseif(WITH_WS_WSLAY)
    set(WS_IMPL_LIB nakama::impl-ws-wslay)
    add_subdirectory(impl/wsWslay EXCLUDE_FROM_ALL)
elseif(WITH_WS_CPPREST)
    set(WS_IMPL_LIB nakama::impl-ws-cppRest)
    add_subdirectory(impl/wsCppRest EXCLUDE_FROM_ALL)
endif()
if (WS_IMPL_LIB)
    target_link_libraries(nakama-sdk-rtclient-factory
            PRIVATE ${WS_IMPL_LIB}
            INTERFACE $<TARGET_OBJECTS:${WS_IMPL_LIB}>)
endif()

if (ANDROID)
    add_subdirectory(impl/android)
#    set(CREATE_JAR TRUE) # we need the code for accessing native Android CA store.
    set(CREATE_JAR FALSE) # we need the code for accessing native Android CA store.

    if (TARGET nakama::impl-http-libcurl)
        target_link_libraries(nakama-impl-http-libcurl PUBLIC android-ca)
    endif()

    if (TARGET nakama::impl-ws-wslay)
        target_link_libraries(nakama-impl-ws-wslay PUBLIC android-ca)
    endif()
   list(APPEND NAKAMA_SDK_DEPS nakama::android-ca)
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

# create dummy file because pure "library of libraries" is not supported: https://gitlab.kitware.com/cmake/cmake/-/issues/23688
set(DUMMY_CPP "")
set(DUMMY_H "")

if (APPLE)
    file(TOUCH ${CMAKE_BINARY_DIR}/dummy.h ${CMAKE_BINARY_DIR}/dummy.cpp)
    set(DUMMY_CPP ${CMAKE_BINARY_DIR}/dummy.cpp)
    set(DUMMY_H ${CMAKE_BINARY_DIR}/dummy.h)
endif()


# Use 'nakama::' ALIASes because it forces CMake to reference
# defined target and not to fallback to '-lname' when target doesn't exist,
# thus failing at configure time, rather than at link time should we make mistake
# in build scripts
list(APPEND NAKAMA_SDK_DEPS
        nakama::sdk-core-misc
        nakama::sdk-core-rest
        nakama::sdk-core-rt
        nakama::sdk-core-common
        nakama::sdk-client-factory
        nakama::sdk-rtclient-factory
        satori::client
)
if (WITH_GRPC_CLIENT)
    list(APPEND NAKAMA_SDK_DEPS nakama::sdk-core-grpc)
endif()

add_library(nakama-sdk SHARED ${DUMMY_CPP} ${DUMMY_H})
add_library(nakama::sdk ALIAS nakama-sdk)
target_link_libraries(nakama-sdk
        PUBLIC
            nakama::sdk-interface
        PRIVATE
            nakama::api-proto
            $<TARGET_NAME_IF_EXISTS:nakama::sdk-core-grpc>
            $<$<BOOL:${LIBHTTPCLIENT_TARGET}>:$<TARGET_NAME_IF_EXISTS:${LIBHTTPCLIENT_TARGET}>>
            ${NAKAMA_SDK_DEPS}
)
if (WITH_HTTP_CURL OR (WITH_WS_WSLAY AND CFG_WSLAY_CURL_IO))
    target_link_libraries(nakama-sdk PRIVATE CURL::libcurl)
endif()

if (APPLE)
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
endif()

if (BUILD_TESTING)
    # Make tests produce .exe in the same dir as our nakama-sdk.dll
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY $<TARGET_FILE_DIR:nakama-sdk>)
    add_subdirectory(test)
endif ()


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
        EXPORT nakama-export
        FRAMEWORK DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_LIBDIR}
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

# Call last so they can reference any target previously defined
include(cmake/localCompilerFlags.cmake)
