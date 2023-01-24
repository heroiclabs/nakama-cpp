set(VCPKG_TARGET_ARCHITECTURE x64)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE static)
set(VCPKG_BUILD_TYPE release)

set(VCPKG_CMAKE_SYSTEM_NAME Darwin)
set(VCPKG_OSX_ARCHITECTURES x86_64)

# Heroic Labs additions to standard triplets:
set(VCPKG_OSX_DEPLOYMENT_TARGET "10.15")
include(${CMAKE_CURRENT_LIST_DIR}/feature-visibility-hidden.cmake)