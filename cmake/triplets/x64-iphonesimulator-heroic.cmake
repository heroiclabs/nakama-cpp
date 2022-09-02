set(VCPKG_TARGET_ARCHITECTURE x64)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE static)
set(VCPKG_CMAKE_SYSTEM_NAME iOS)

# HeroicLabs additions to standard triplets:
set(VCPKG_BUILD_TYPE release)
set(VCPKG_OSX_DEPLOYMENT_TARGET "11")
set(VCPKG_OSX_ARCHITECTURES "x86_64")
set(VCPKG_OSX_SYSROOT iphonesimulator)

#set(VCPKG_CMAKE_FORCE_GENERATOR "Xcode")
#set(VCPKG_CHAINLOAD_TOOLCHAIN_FILE ${CMAKE_CURRENT_SOURCE_DIR}/../ios.toolchain.cmake)
#list(APPEND VCPKG_CMAKE_CONFIGURE_OPTIONS -DPLATFORM=SIMULATOR64)

include(${CMAKE_CURRENT_LIST_DIR}/feature-visibility-hidden.cmake)
