set(VERSION 1.1.1)
message("wslay portfile")
vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO tatsuhiro-t/wslay
    REF release-${VERSION}
    SHA512 b42c66c738a3f33bc7de30e8975f4fb2dc60a8baef44be8d254110c8915e14cdaa4cbdd6b29184a66061fe387ec0948e896cb174a1dd8c85a97b5feedfde162e
    HEAD_REF master
)

string(COMPARE EQUAL "${VCPKG_LIBRARY_LINKAGE}" "static" BUILD_STATIC)
string(COMPARE EQUAL "${VCPKG_LIBRARY_LINKAGE}" "dynamic" BUILD_SHARED)

vcpkg_cmake_configure(
    SOURCE_PATH ${SOURCE_PATH}
    OPTIONS
        -DWSLAY_STATIC=${BUILD_STATIC}
        -DWSLAY_SHARED=${BUILD_SHARED}

        # On consoles system header files are using GNU extensions
        -DCMAKE_C_FLAGS_INIT="-Wno-gnu-statement-expression"
    OPTIONS_DEBUG
        # vcpkg wants debug not to install headers, only binaries
        # so in debug mode we install them outside of CMAKE_INSTALL_PREFIX
        -DCMAKE_INSTALL_INCLUDEDIR=/tmp/ignoreme
)
vcpkg_cmake_install()

vcpkg_cmake_config_fixup(CONFIG_PATH lib/cmake/${PORT})

file(REMOVE_RECURSE
        ${CURRENT_PACKAGES_DIR}/debug/share
)
