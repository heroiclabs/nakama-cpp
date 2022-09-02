set(VCPKG_TARGET_ARCHITECTURE x64)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE static)

set(VCPKG_BUILD_TYPE "release")

# GDK installs this "architecture" (which is just a way to configure
# global complilation options for Xbox platform, not a real arch)
set(VCPKG_CMAKE_CONFIGURE_OPTIONS "-A ${CMAKE_GENERATOR_PLATFORM}")

# explicitly switch off VCVARS in case future me is tempted to
# switch it back on. VCPKG ports are not entirely CMake based,
# and for those packages loading MSVC with default options
# isn't what we want
set(VCPKG_LOAD_VCVARS_ENV OFF)

# This variable is a Heroic "extension" to vcpkg.
# vcpkg prefers Ninja generator on all platforms, but Windows UWP,
# but we want VisualStudio, because adds Xbox configuration to it.
set(VCPKG_CMAKE_FORCE_GENERATOR ${CMAKE_GENERATOR})