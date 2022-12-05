set(VCPKG_TARGET_ARCHITECTURE arm)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE static)
set(VCPKG_CMAKE_SYSTEM_NAME Android)

set(VCPKG_BUILD_TYPE release)

# toolchain file is included in the NDK, so we can't predefine path to it
if(NOT DEFINED ENV{VCPKG_CHAINLOAD_TOOLCHAIN_FILE})
    message(FATAL_ERROR "Pass VCPKG_CHAINLOAD_TOOLCHAIN_FILE env")
endif()
set(VCPKG_CHAINLOAD_TOOLCHAIN_FILE "$ENV{VCPKG_CHAINLOAD_TOOLCHAIN_FILE}")
