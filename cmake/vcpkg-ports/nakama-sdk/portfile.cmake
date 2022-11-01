vcpkg_from_git(
    OUT_SOURCE_PATH SOURCE_PATH
    URL https://github.com/heroiclabs/nakama-cpp.git
    HEAD_REF master
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
