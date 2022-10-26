vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO heroiclabs/nakama-cpp
    HEAD_REF master
)

vcpkg_cmake_configure(SOURCE_PATH ${SOURCE_PATH})
vcpkg_cmake_install()