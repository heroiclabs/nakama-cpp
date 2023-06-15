file(
        GLOB_RECURSE srcs
        LIST_DIRECTORIES false
        "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

find_package(wslay CONFIG REQUIRED)
add_library(nakama-impl-ws-wslay OBJECT ${srcs})

target_link_libraries(nakama-impl-ws-wslay PUBLIC
        nakama-api-proto nakama::sdk-interface
        nakama-sdk-core-common nakama-sdk-core-misc
        wslay
    )

target_include_directories(nakama-impl-ws-wslay INTERFACE
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>")

if (BUILD_CURL_IO AND NOT BUILD_CURL_IO_SYSTEM)
    find_package(CURL CONFIG REQUIRED)
    target_link_libraries(nakama-impl-ws-wslay PRIVATE CURL::libcurl)
endif()

target_compile_definitions(nakama-impl-ws-wslay PUBLIC "WSLAY_NETIO_${WSLAY_NETIO}")
