file(
        GLOB srcs
        LIST_DIRECTORIES false
        "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

find_package(wslay CONFIG REQUIRED)
add_library(nakama-impl-ws-wslay OBJECT ${srcs})
add_library(nakama::impl-ws-wslay ALIAS nakama-impl-ws-wslay)

target_link_libraries(nakama-impl-ws-wslay
    PUBLIC
        nakama::sdk-interface
    PRIVATE
        nakama::sdk-core-common nakama::sdk-core-misc wslay
)

target_include_directories(nakama-impl-ws-wslay
        INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}  # for WslayIOInterface.h
)

if (CFG_WSLAY_CURL_IO)
    # Even though WslayIOCUrl is in this lib, it is a header and gets compiled
    # in the factory lib where it #include'd.
    # So we need to CMake INTERFACE anything might be required to compile
    target_link_libraries(nakama-impl-ws-wslay INTERFACE
            CURL::libcurl
            nakama::sdk-core-common # for StrUtil.h
    )
endif()
