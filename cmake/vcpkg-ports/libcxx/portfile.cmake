set(LLVM_VERSION "14.0.3")

vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO llvm/llvm-project
    REF llvmorg-${LLVM_VERSION}
    SHA512 511e93fd9b1c414c38fe9e2649679ac0b16cb04f7f7838569d187b04c542a185e364d6db73e96465026e3b2533649eb75ac95507d12514af32b28bdfb66f2646
    HEAD_REF master
)

string(COMPARE EQUAL "${VCPKG_LIBRARY_LINKAGE}" "static" BUILD_STATIC)
string(COMPARE EQUAL "${VCPKG_LIBRARY_LINKAGE}" "dynamic" BUILD_SHARED)


vcpkg_cmake_configure(
    SOURCE_PATH ${SOURCE_PATH}/runtimes
    OPTIONS
        -DLIBCXX_INCLUDE_TESTS=OFF
        -DLIBCXX_INCLUDE_BENCHMARKS=OFF
        -DLIBCXX_ENABLE_EXPERIMENTAL_LIBRARY=OFF
        -DLIBCXX_ENABLE_SHARED=${BUILD_SHARED}
        -DLIBCXX_ENABLE_STATIC=${BUILD_STATIC}
        -DLIBCXXABI_ENABLE_SHARED=${BUILD_SHARED}
        -DLIBCXXABI_ENABLE_STATIC=${BUILD_STATIC}
        # if we link libc++ statically don't expose it
        # a.k.a -fvisibility=hidden for the static libc++
        -DLIBCXX_HERMETIC_STATIC_LIBRARY=ON

        # But DO expose libc++abi, because it is used by binaries (like nakama-test)
        # and symbols from libc++abi need to be made avaialable to underlinked libnakama-sdk.so
        -DLIBCXXABI_HERMETIC_STATIC_LIBRARY=OFF

        # libc++abi include operator new/delete and we definitely
        # don't want them to be in our lib, but to be taken from
        # the main executable, even if we happen to link libc++ statically
        -DLIBCXX_ENABLE_STATIC_ABI_LIBRARY=OFF
        -DLIBCXX_ENABLE_EXPERIMENTAL_LIBRARY=OFF
        -DLIBCXX_CXX_ABI=libcxxabi
        "-DLLVM_ENABLE_RUNTIMES=libcxx;libcxxabi"
)

vcpkg_cmake_install()


if(NOT DEFINED VCPKG_BUILD_TYPE OR VCPKG_BUILD_TYPE STREQUAL "debug")
    file(REMOVE_RECURSE ${CURRENT_PACKAGES_DIR}/debug/include)
    file(REMOVE_RECURSE ${CURRENT_PACKAGES_DIR}/debug/share)
    file(REMOVE_RECURSE ${CURRENT_PACKAGES_DIR}/debug/tools)
endif()
