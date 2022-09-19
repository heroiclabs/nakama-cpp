project(nakama-example LANGUAGES CXX)
cmake_minimum_required(VERSION 3.23)
set(CMAKE_CXX_STANDARD 14)

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

get_filename_component(_NAKAMA_PROJECT "${CMAKE_SOURCE_DIR}" DIRECTORY)
set(nakama-sdk_DIR ${_NAKAMA_PROJECT}/out/${CMAKE_INSTALL_LIBDIR}/cmake/nakama)

find_package(nakama-sdk CONFIG REQUIRED)

add_executable(nakama-example MACOSX_BUNDLE main.cpp)
set_target_properties(nakama-example PROPERTIES
    MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/example.plist
)


# no ZERO_CHECK target for Xcode
set(CMAKE_SUPPRESS_REGENERATION true)

# bitcode will be deprecated in Xcode 14 which is in beta at the time of this writing
# https://developer.apple.com/documentation/xcode-release-notes/xcode-14-release-notes
set_target_properties(nakama-example PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER "com.example"
    XCODE_ATTRIBUTE_ENABLE_BITCODE "NO"
    XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER "com.heroiclabs.example"
    XCODE_ATTRIBUTE_CODE_SIGNING_REQUIRED "YES"
    XCODE_ATTRIBUTE_CODE_SIGNING_STYLE "AUTOMATIC"
    XCODE_ATTRIBUTE_CODE_SIGN_ENTITLEMENTS ${PROJECT_SOURCE_DIR}/example.entitlements
    XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "iPhone Developer"
    XCODE_ATTRIBUTE_DEVELOPMENT_TEAM "$ENV{NAKAMA_EXAMPLE_DEVELOPMENT_TEAM}"
    XCODE_LINK_BUILD_PHASE_MODE KNOWN_LOCATION
    XCODE_EMBED_FRAMEWORKS ${_NAKAMA_PROJECT}/out/${CMAKE_INSTALL_LIBDIR}/nakama-sdk.framework ## todo figure out why passing target doesn't work
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
endif()


target_link_libraries(nakama-example
    PRIVATE nakama-sdk
)

set(EXAMPLE_OUT_DIR "${CMAKE_SOURCE_DIR}/out/${CMAKE_INSTALL_LIBDIR}")

install(TARGETS nakama-example
    BUNDLE DESTINATION ${EXAMPLE_OUT_DIR})

install(IMPORTED_RUNTIME_ARTIFACTS nakama-sdk
    RUNTIME DESTINATION ${EXAMPLE_OUT_DIR}
    LIBRARY DESTINATION ${EXAMPLE_OUT_DIR}
 )
