set(VCPKG_TARGET_ARCHITECTURE arm64)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE static)
set(VCPKG_BUILD_TYPE release)

set(VCPKG_CMAKE_SYSTEM_NAME Darwin)
set(VCPKG_OSX_ARCHITECTURES arm64)

# HeroicLabs additions to standard triplets:
set(VCPKG_OSX_DEPLOYMENT_TARGET "11")
set(VCPKG_BUILD_TYPE release)
include(${CMAKE_CURRENT_LIST_DIR}/feature-visibility-hidden.cmake)
