set(VERSION 1.1.1)

vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO tatsuhiro-t/wslay
    REF release-${VERSION}
    SHA512 b42c66c738a3f33bc7de30e8975f4fb2dc60a8baef44be8d254110c8915e14cdaa4cbdd6b29184a66061fe387ec0948e896cb174a1dd8c85a97b5feedfde162e
    HEAD_REF master
    PATCHES
        msvc_compat.patch
#        msvc_compat2.patch
)

string(COMPARE EQUAL "${VCPKG_LIBRARY_LINKAGE}" "static" BUILD_STATIC)
string(COMPARE EQUAL "${VCPKG_LIBRARY_LINKAGE}" "dynamic" BUILD_SHARED)

vcpkg_check_features(
    OUT_FEATURE_OPTIONS FEATURE_OPTIONS
    FEATURES
        windows-use-msbuild WINDOWS_USE_MSBUILD
)

if(WINDOWS_USE_MSBUILD)
    set(WINDOWS_USE_MSBUILD_OPTION "WINDOWS_USE_MSBUILD")
else()
    set(WINDOWS_USE_MSBUILD_OPTION "")
endif()

vcpkg_cmake_configure(
    SOURCE_PATH ${SOURCE_PATH}
  ${WINDOWS_USE_MSBUILD_OPTION}
    OPTIONS
        -DWSLAY_STATIC=${BUILD_STATIC}
        -DWSLAY_SHARED=${BUILD_SHARED}

        # On consoles system header files are using GNU extensions
        -DCMAKE_C_FLAGS_INIT="-Wno-gnu-statement-expression"
)
vcpkg_cmake_install()

vcpkg_cmake_config_fixup(CONFIG_PATH lib/cmake/${PORT})

file(REMOVE_RECURSE
        ${CURRENT_PACKAGES_DIR}/debug/share
)

file(INSTALL "${SOURCE_PATH}/COPYING" DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}" RENAME copyright)
