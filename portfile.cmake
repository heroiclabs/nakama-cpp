vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO heroiclabs/nakama-cpp
    REF 9f984507326a1e60a9adb785cbf455abc8202bf9
)

vcpkg_cmake_configure(SOURCE_PATH ${SOURCE_PATH})
vcpkg_cmake_install()
