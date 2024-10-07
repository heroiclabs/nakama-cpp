# NDAed platforms might configure it already
if (__PLATFORM_SUPPORT_CONFIGURED)
    return()
endif()

# Various platform specific defines
if (CMAKE_SYSTEM_NAME STREQUAL Windows OR WindowsDesktop)
    add_compile_definitions(_UNICODE UNICODE)
    set(CMAKE_INSTALL_BINDIR ${CMAKE_INSTALL_LIBDIR})  # place .dll where .lib is so that multiplatform archives can be created
elseif(CMAKE_SYSTEM_NAME STREQUAL Darwin)
    message("Configuring Apple MacOSX build")
    # when changing, dont forget also set it in {arm64,x64}-osx-heroic vcpkg triplet
    # NOTE: we can't use CMAKE_SYSTEM_PROCESSOR here because it is always arm64 on M1 silicon for some reason
    if (VCPKG_TARGET_TRIPLET MATCHES "arm64")
        set(CMAKE_OSX_DEPLOYMENT_TARGET "11")  # minimal OSX to support M1
    else()
        set(CMAKE_OSX_DEPLOYMENT_TARGET "10.15")  # this is what Unreal builds for by default
    endif()
elseif(CMAKE_OSX_SYSROOT MATCHES Simulator)
    message("Configuring Apple iphonesimulator build")
    set(CMAKE_OSX_DEPLOYMENT_TARGET "11")
    set(CMAKE_OSX_ARCHITECTURES "x86_64")
elseif(CMAKE_SYSTEM_NAME STREQUAL iOS)
    message("Configuring Apple iOS build")
    # Don't forget to edit ./cmake/triplets too!
    # Value is picked based on  https://developer.apple.com/support/app-store/ numbers
    set(CMAKE_OSX_DEPLOYMENT_TARGET "12")
    set(CMAKE_OSX_ARCHITECTURES "arm64")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    message("Configuring Linux build")
    set(LINUX ON)
    set(THREADS_PREFER_PTHREAD_FLAG ON)
    find_package(Threads REQUIRED)
    set(PTHREADS_LIB Threads::Threads)
endif()

set(__PLATFORM_SUPPORT_CONFIGURED ON)
