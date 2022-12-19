
cmake_minimum_required(VERSION 3.23)

set(CMAKE_TOOLCHAIN_FILE $ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake
        CACHE FILEPATH "Vcpkg toolchain file")

project(nakama-example LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 14)

find_package(optional-lite)

if (${CMAKE_SYSTEM_NAME} STREQUAL "iOS" OR ${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    # CMake needs to implement a cleaner way of doing this: https://gitlab.kitware.com/cmake/cmake/-/issues/22997 &&
    # https://gitlab.kitware.com/cmake/cmake/-/issues/22240
    set(CMAKE_BUILD_WITH_INSTALL_RPATH YES)
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(THREADS_PREFER_PTHREAD_FLAG ON)
    find_package(Threads REQUIRED)
    set(PTHREADS_LIB Threads::Threads)
endif()

find_package(nakama-sdk CONFIG REQUIRED)

add_executable(nakama-example MACOSX_BUNDLE main.cpp)

# no ZERO_CHECK target for Xcode
set(CMAKE_SUPPRESS_REGENERATION true)

message("framework is ${nakama-sdk_FRAMEWORK}")
# bitcode will be deprecated in Xcode 14 which is in beta at the time of this writing
# https://developer.apple.com/documentation/xcode-release-notes/xcode-14-release-notes
set_target_properties(nakama-example PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER "com.example"
    XCODE_ATTRIBUTE_ENABLE_BITCODE "NO"
    MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/example.plist
    XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER "com.heroiclabs.example"
    XCODE_ATTRIBUTE_CODE_SIGNING_REQUIRED "YES"
    XCODE_ATTRIBUTE_CODE_SIGNING_STYLE "AUTOMATIC"
    XCODE_ATTRIBUTE_CODE_SIGN_ENTITLEMENTS ${CMAKE_CURRENT_SOURCE_DIR}/example.entitlements
    XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "iPhone Developer"
    XCODE_ATTRIBUTE_DEVELOPMENT_TEAM "$ENV{NAKAMA_TEST_DEVELOPMENT_TEAM}"
    XCODE_LINK_BUILD_PHASE_MODE KNOWN_LOCATION
    # targets don't work for embedding frameworks, and can't find a better way to get path
    XCODE_EMBED_FRAMEWORKS ${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/lib/nakama-sdk.framework
    XCODE_EMBED_FRAMEWORKS_CODE_SIGN_ON_COPY		"YES"		# frameworks must be signed by the same developer as the binary
	XCODE_EMBED_FRAMEWORKS_REMOVE_HEADERS_ON_COPY	"YES"
    XCODE_ATTRIBUTE_CODE_SIGN_STYLE Automatic
)


if (${CMAKE_SYSTEM_NAME} STREQUAL "iOS")
    set_target_properties(nakama-example PROPERTIES
        INSTALL_RPATH @executable_path/Frameworks
    )
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set_target_properties(nakama-example PROPERTIES
        INSTALL_RPATH @executable_path/../Frameworks
    )
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set_target_properties(nakama-test PROPERTIES
        INSTALL_RPATH $ORIGIN
    )
endif()


target_link_libraries(nakama-example
    PRIVATE nakama-sdk
)

set(EXAMPLE_OUT_DIR "${CMAKE_SOURCE_DIR}/out/${CMAKE_INSTALL_LIBDIR}")

install(TARGETS nakama-example
    BUNDLE DESTINATION ${EXAMPLE_OUT_DIR})

if (NOT APPLE) ## we embed frameworks for apple
    install(IMPORTED_RUNTIME_ARTIFACTS nakama-sdk
        RUNTIME DESTINATION ${EXAMPLE_OUT_DIR}
        LIBRARY DESTINATION ${EXAMPLE_OUT_DIR}
    )
endif()