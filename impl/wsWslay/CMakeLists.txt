file(
        GLOB_RECURSE srcs
        LIST_DIRECTORIES false
        "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
)


find_package(wslay CONFIG REQUIRED)
add_library(nakama-impl-ws-wslay INTERFACE ${srcs})

target_link_libraries(nakama-impl-ws-wslay INTERFACE
        nakama-api-proto nakama::sdk-interface
        wslay nakama-sdk-core-common
        )

target_include_directories(nakama-impl-ws-wslay INTERFACE .)

if (BUILD_CURL_IO)
    find_package(CURL CONFIG REQUIRED)
    target_link_libraries(nakama-impl-ws-wslay INTERFACE CURL::libcurl)
endif()

target_compile_definitions(nakama-impl-ws-wslay INTERFACE "WSLAY_NETIO_${WSLAY_NETIO}")
