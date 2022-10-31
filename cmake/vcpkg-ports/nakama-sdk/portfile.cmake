vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO heroiclabs/nakama-cpp
    REF 9f984507326a1e60a9adb785cbf455abc8202bf9
    SHA512 05446f2a59947afcfd029be62c7639566765f2dc1564982b6d5366ebf60f4be6a4539d1caaa0ab20d27bb8b4a806862ddcb1d5bd0007b7a28eb26e0f10bf0164
)

vcpkg_check_features(
    FEATURES
        wslay BUILD_WSLAY
        grpc  BUILD_GRPC_CLIENT
)

vcpkg_cmake_configure(SOURCE_PATH ${SOURCE_PATH}
  OPTIONS
    --DINCLUDE_OPTIONAL_LITE=OFF # user will obtain optional-lite via vcpkg dependency management
    ${FEATURE_OPTIONS} ### created by vcpkg_check_features
)

vcpkg_cmake_install()
