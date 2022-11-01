### sacrifice determinism for iteration speed in this section because for now we already have the registry baseline for pinning commits
set(VCPKG_USE_HEAD_VERSION ON)

vcpkg_from_git(
    OUT_SOURCE_PATH SOURCE_PATH
    URL https://github.com/heroiclabs/nakama-cpp.git
    HEAD_REF master
)

set(VCPKG_USE_HEAD_VERSION OFF)

vcpkg_check_features(
    OUT_FEATURE_OPTIONS FEATURE_OPTIONS
    FEATURES
        wslay BUILD_WSLAY
        grpc  BUILD_GRPC_CLIENT
)

vcpkg_cmake_configure(SOURCE_PATH ${SOURCE_PATH}
  OPTIONS
    -DINCLUDE_OPTIONAL_LITE=OFF # user will obtain optional-lite via vcpkg dependency management
    ${FEATURE_OPTIONS} ### created by vcpkg_check_features
)

vcpkg_cmake_install()
