set(VCPKG_TARGET_ARCHITECTURE x64)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE static)

# enabling toolchain has an effect of disabling use of  vcvars.bat in vcpkg
# it is what we want becasue vcvars doesn't support Durango as a platform
set(VCPKG_CHAINLOAD_TOOLCHAIN_FILE ${CMAKE_CURRENT_LIST_DIR}/../xdk-toolchain.cmake)

# VCPKG_CMAKE_SYSTEM_NAME is set when evaluating manifests (dependencies)
# and port files. WindowsStore is closest to Durango, so use it
# When actual build starts, toolchain will set platform to Durango correctly
set(VCPKG_CMAKE_SYSTEM_NAME WindowsStore)

set(VCPKG_BUILD_TYPE release)

# explicitly switch off VCVARS in case future me is tempted to
# switch it back on. VCPKG ports are not entirely CMake based,
# and for those packages loading MSVC with default options
# isn't what we want
set(VCPKG_LOAD_VCVARS_ENV OFF)

# This variable is Heroic "extension" to vcpkg. When it is set, vcpkg always
# Uses Ninja generator if it is not specified in the port file directly.
# We need it because our toolchain is MSVC based, but not
# platform is "Durango", while vcpkg expects it to be x64
set(VCPKG_CMAKE_FORCE_GENERATOR "Ninja")

if ("$ENV{XboxOneXDKLatest}" STREQUAL "" AND "$ENV{VCPKG_KEEP_ENV_VARS}" STREQUAL "")
message(FATAL_ERROR "You need to set env var VCPKG_KEEP_ENV_VARS=XboxOneXDKLatest;XboxOneExtensionSDKLatest")
endif()