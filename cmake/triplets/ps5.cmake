set(VCPKG_TARGET_ARCHITECTURE x64)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE static)

set(VCPKG_CHAINLOAD_TOOLCHAIN_FILE "${CMAKE_CURRENT_LIST_DIR}/../ps5-toolchain.cmake")

# VCPKG_CMAKE_SYSTEM_NAME is set when evaluating manifests (dependencies)
# and port files. Linux is closest to Orbis (PS4), so use it.
# When actual build starts, toolchain will set platform to Orbis correctly
set(VCPKG_CMAKE_SYSTEM_NAME Linux)

# Set system name to what toolchain will be setting, but having it here allows us
# to enable system specific options in port files
set(CMAKE_SYSTEM_NAME "Prospero")


# Don't build debug versions of dependencies (saves build time)
set(VCPKG_BUILD_TYPE release)

# This variable is Heroic "extension" to vcpkg. When it is set, vcpkg always
# Uses Ninja generator if it is not specified in the port file directly.
# We want it because PS4 toolchain supports Ninja and it is easier to debug
# ninja builds comparing to Visual Studio.
set(VCPKG_CMAKE_FORCE_GENERATOR "Ninja")


if (PORT STREQUAL protobuf)
# protobuf's CMake incorrectly misses builtin atomics, help it "find" them
list(APPEND VCPKG_CMAKE_CONFIGURE_OPTIONS -Dprotobuf_HAVE_BUILTIN_ATOMICS=TRUE)
endif()

