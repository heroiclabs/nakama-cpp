if (LINUX OR APPLE)
    find_package(CURL CONFIG REQUIRED)
    target_compile_definitions(CURL::libcurl INTERFACE CURL_STRICTER)
endif()


set(LIBHTTPCLIENT_OBJECT_LIBRARY TRUE)
set(WAS_SHARED_LIBS ${BUILD_SHARED_LIBS})

if (ANDROID)
    set(LIBHTTPCLIENT_SOURCE_SUBDIR Utilities/CMake/Android/libHttpClient)
else()
    set(LIBHTTPCLIENT_SOURCE_SUBDIR Utilities/CMake)
endif()

## force libhttpclient to build statically
set(BUILD_SHARED_LIBS OFF)

add_subdirectory("${CMAKE_SOURCE_DIR}/submodules/libHttpClient/${LIBHTTPCLIENT_SOURCE_SUBDIR}")

set(BUILD_SHARED_LIBS ${WAS_SHARED_LIBS})

if (ANDROID)
    set(LIBHTTPCLIENT_TARGET libHttpClient.Android)
else()

    if(XDK)
        if (WINRT)
            set(LIBHTTPCLIENT_TARGET libHttpClient.XDK.WinRT)
        else()
            set(LIBHTTPCLIENT_TARGET libHttpClient.XDK.C)
        endif()
    elseif (GDK)
        set(LIBHTTPCLIENT_TARGET libHttpClient.GDK.C)
        include(submodules/devkits/cmake/gdk-targets.cmake)
        target_link_libraries(${LIBHTTPCLIENT_TARGET} PRIVATE GDK::XCurl)
    elseif (BUILDWIN32)
        set(LIBHTTPCLIENT_TARGET libHttpClient.Win32.C)
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
list(APPEND NAKAMA_SDK_DEPS ${LIBHTTPCLIENT_TARGET})

if (NOT WITH_WS_LIBHTTPC)
    target_compile_definitions(${LIBHTTPCLIENT_TARGET} PRIVATE "HC_NOWEBSOCKETS")
else()
    if (LIBHTTPCLIENT_FORCE_WEBSOCKETPP)
        message(STATUS "Forcing libhttpclient to use websocketpp even on modern Windows platforms")
        target_compile_definitions(${LIBHTTPCLIENT_TARGET} PRIVATE "HC_FORCE_WINSOCKETPP")
    endif()
endif()
