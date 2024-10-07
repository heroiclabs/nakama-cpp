set(VCPKG_TARGET_ARCHITECTURE arm64)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE static)
set(VCPKG_CMAKE_SYSTEM_NAME iOS)

# HeroicLabs additions to standard triplets:
set(VCPKG_BUILD_TYPE release)
set(VCPKG_OSX_DEPLOYMENT_TARGET "12")
set(VCPKG_OSX_ARCHITECTURES "arm64")
set(VCPKG_OSX_SYSROOT iphonesimulator)

include(${CMAKE_CURRENT_LIST_DIR}/feature-visibility-hidden.cmake)
