# Automatically generated by scripts/boost/generate-ports.ps1

vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO boostorg/fiber
    REF boost-1.78.0
    SHA512 271454964e12c0006ed9ef52911cf76d004141da54d5f96e63d106a216841d9a939d3dcd802fc9e860ac6a21d0b7fab9f385e8647ebbd2fd669e48fc9b6a80c9
    HEAD_REF master
    PATCHES fix-build-numa.patch
)

file(READ "${SOURCE_PATH}/build/Jamfile.v2" _contents)
string(REPLACE "import ../../config/checks/config" "import ../config/checks/config" _contents "${_contents}")
file(WRITE "${SOURCE_PATH}/build/Jamfile.v2" "${_contents}")
file(COPY "${CURRENT_INSTALLED_DIR}/share/boost-config/checks" DESTINATION "${SOURCE_PATH}/config")
if(NOT DEFINED CURRENT_HOST_INSTALLED_DIR)
    message(FATAL_ERROR "boost-fiber requires a newer version of vcpkg in order to build.")
endif()
include(${CURRENT_HOST_INSTALLED_DIR}/share/boost-build/boost-modular-build.cmake)
boost_modular_build(
    SOURCE_PATH ${SOURCE_PATH}
    BOOST_CMAKE_FRAGMENT "${CMAKE_CURRENT_LIST_DIR}/b2-options.cmake"
)
include(${CURRENT_INSTALLED_DIR}/share/boost-vcpkg-helpers/boost-modular-headers.cmake)
boost_modular_headers(SOURCE_PATH ${SOURCE_PATH})