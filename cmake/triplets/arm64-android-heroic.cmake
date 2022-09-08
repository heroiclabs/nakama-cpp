set(VCPKG_TARGET_ARCHITECTURE arm64)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE static)
set(VCPKG_CMAKE_SYSTEM_NAME Android)

set(VCPKG_BUILD_TYPE release)

# toolchain file is included in the NDK, so we can't predefine path to it
if(NOT DEFINED ENV{VCPKG_CHAINLOAD_TOOLCHAIN_FILE})
    message(FATAL_ERROR "Pass VCPKG_CHAINLOAD_TOOLCHAIN_FILE env")
endif()
set(VCPKG_CHAINLOAD_TOOLCHAIN_FILE "$ENV{VCPKG_CHAINLOAD_TOOLCHAIN_FILE}")

set(VCPKG_CMAKE_CONFIGURE_OPTIONS
        -DANDROID_USE_LEGACY_TOOLCHAIN_FILE=FALSE
        -DANDROID_ABI=arm64-v8a
        # TODO: pass these from main CMake via env vars so that they are always in sync
        -DANDROID_PLATFORM=21
        -DANDROID_STL=c++_shared
        -DANDROID_ARM_NEON=ON
)