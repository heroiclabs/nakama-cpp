# Optional use of libc++ on Linux platforms
#
# This file declares WITH_LIBCXX config option and sets up
# build system to compile dependencies and our code with libc++.
# Here we invoke `vcpkg` directly to install libc++ using cusom
# port in vcpkg-ports/libcxx/portfile.
#
# Next we generate toolchain file based on build arguments. That
# toolchain is used both to compile SDK code (via VCKPG_CHAINLOAD_TOOLCHAIN_FILE  var)
# and to compile vcpkg dependencies (via VCPKG_CHAINLOAD_TOOLCHAIN_FILE_BAKED environment var,
# which is then used by x64-linux heroic triplet)
option(WITH_LIBCXX "Build using libc++ (has effect only when targeting Linux)" OFF)
option(LIBCXX_STATIC "When using libc++, link it statically" OFF)

if (UNREAL AND VCPKG_TARGET_TRIPLET MATCHES "linux")
    if (NOT WITH_LIBCXX OR NOT LIBCXX_STATIC)
        message(WARNING "Unreal build on Linux requires WITH_LIBCXX=ON and LIBCXX_STATIC=ON. Forcing correct values to be set.")
        set(WITH_LIBCXX ON CACHE BOOL "Build with libc++" FORCE)
        set(LIBCXX_STATIC ON CACHE BOOL "When using libc++, link it statically" FORCE)
    endif()
endif()


if (NOT (WITH_LIBCXX AND VCPKG_TARGET_TRIPLET MATCHES "linux"))
  return()
endif()


# install libc++ variants into own dirs to keep files
# separately when combining artifacts in CI scripts
string(APPEND CMAKE_INSTALL_LIBDIR "-libcxx")
if (LIBCXX_STATIC)
    string(APPEND CMAKE_INSTALL_LIBDIR "_static")
endif()

if (NOT VCPKG_TARGET_TRIPLET STREQUAL "x64-linux-heroic")
  message(FATAL_ERROR "Expected x64-linux-heroic VCPKG_TARGET_TRIPLET when requesting libc++ build")
endif()


if(LIBCXX_STATIC)
  message(STATUS "Building static libc++")
  set(TRIPLET "x64-linux-release")
else()
  message(STATUS "Building dynamic libc++")
  set(TRIPLET "x64-linux-dynamic-release")
endif()


# We can't just install into "main" vcpkg_installed tree
# because subsequent run of vcpkg in the manifest mode will
# remove it. Instead we build and install it on a side.
set(LIBCXX_INSTALL_ROOT ${CMAKE_CURRENT_BINARY_DIR}/libcxx)

execute_process(COMMAND
  ${VCPKG_ROOT_DIR}/vcpkg
  --feature-flags=-manifests
  install
  --triplet=${TRIPLET}
  --vcpkg-root ${CMAKE_CURRENT_LIST_DIR}/vcpkg
  --x-install-root=${LIBCXX_INSTALL_ROOT}
  --overlay-ports=${CMAKE_CURRENT_LIST_DIR}/vcpkg-ports
  --overlay-triplets=${CMAKE_CURRENT_LIST_DIR}/triplets
  libcxx
  COMMAND_ERROR_IS_FATAL ANY
)


# shared libc++.so.1 wont be found in a non-standard location. Copy it to
# the main vcpkg_installed tree.
if (NOT LIBCXX_STATIC)
 file( CREATE_LINK
       "${LIBCXX_INSTALL_ROOT}/${TRIPLET}/lib/libc++.so.1"
       "${CMAKE_CURRENT_BINARY_DIR}/vcpkg_installed/${VCPKG_TARGET_TRIPLET}/lib/libc++.so.1"
       COPY_ON_ERROR SYMBOLIC
 )

 file( CREATE_LINK
         "${LIBCXX_INSTALL_ROOT}/${TRIPLET}/lib/libc++abi.so.1"
         "${CMAKE_CURRENT_BINARY_DIR}/vcpkg_installed/${VCPKG_TARGET_TRIPLET}/lib/libc++abi.so.1"
         COPY_ON_ERROR SYMBOLIC
         )

 # vcpkg doesn't update rpath on tools, leaving them broken when dynamic linking is used
 # This symlink is just a workaround to make protoc work, until it is resolved properly.
 # - https://github.com/microsoft/vcpkg/issues/17607#issuecomment-1126544129
 # - https://github.com/microsoft/vcpkg/pull/15134
 file( MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/vcpkg_installed/${VCPKG_TARGET_TRIPLET}/tools")
 file( CREATE_LINK
       "../lib"
       "${CMAKE_CURRENT_BINARY_DIR}/vcpkg_installed/${VCPKG_TARGET_TRIPLET}/tools/lib"
       SYMBOLIC
 )
endif()


# vcpkg caches compiled dependencies based on toolchain file hash. Our libcxx toolchain
# changes beheaviour depending on WITH_LIBCXX and LIBCXX_STATIC params. Ie we pass these
# params via env var, then vpkg wont see them and wont invalidate cache, leading to linking
# failure at the very end of SDK build process. For this reason we bake in params into
# a  new toolchain file, so that when params change, file changes too and vcpkg can
# select appropriate cache entry or trigger dependency build if none found.
set(real_project_dir "${CMAKE_CURRENT_LIST_DIR}/..")
set(toolchain_config [=[
        set(real_project_dir @real_project_dir@)
        set(WITH_LIBCXX "@WITH_LIBCXX@")
        set(LIBCXX_STATIC "@LIBCXX_STATIC@")
        set(LIBCXX_INSTALL_DIR "@LIBCXX_INSTALL_ROOT@/@TRIPLET@")
]=])
set(baked_toolchain_file "${CMAKE_CURRENT_BINARY_DIR}/libcxx-toolchain-baked.cmake")
file(CONFIGURE OUTPUT ${baked_toolchain_file} CONTENT "${toolchain_config}" @ONLY)
file(READ "${CMAKE_CURRENT_LIST_DIR}/libcxx-toolchain.cmake" toolchain_content)
file(APPEND ${baked_toolchain_file} "${toolchain_content}")


# vcpkg uses just toolchain file itself when calculating cache key, it misses any includes.
# Run our toolchain file in trace mode (like set -x in bash) to capture changes in included
# files as well, then hash trace output and append to the toolchain file
# See: https://github.com/microsoft/vcpkg/issues/24748
execute_process(COMMAND ${CMAKE_COMMAND} --trace -P "${baked_toolchain_file}"
        OUTPUT_QUIET
        ERROR_VARIABLE toolchain_trace
        COMMAND_ERROR_IS_FATAL ANY
        )
string(SHA1 toolchain_trace_hash "${toolchain_trace}")
message(VERBOSE "Toolchain trace hash: ${toolchain_trace_hash}")
file(APPEND "${baked_toolchain_file}" "\n#tracehash: ${toolchain_trace_hash}\n")

# Setting here enables toolchain when building SDK code (but not it's vcpkg dependencies)
set(VCPKG_CHAINLOAD_TOOLCHAIN_FILE "${baked_toolchain_file}")

# there is no direct way to pass params port installed via vcpkg
# so we set env vars, which are then going to be picked by x64-linux-heroic.cmake triplet
set(ENV{VCPKG_CHAINLOAD_TOOLCHAIN_FILE_BAKED} "${VCPKG_CHAINLOAD_TOOLCHAIN_FILE}")
