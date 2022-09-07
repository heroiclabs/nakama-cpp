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

if (WSLAY_NETIO STREQUAL "CURL")
    find_package(CURL CONFIG REQUIRED)
    target_link_libraries(nakama-impl-ws-wslay INTERFACE CURL::libcurl)
endif()

if (NOT WSLAY_NETIO)
    message(FATAL_ERROR "Error in our CMake, must be set by now")
endif()

target_compile_definitions(nakama-impl-ws-wslay INTERFACE "WSLAY_NETIO_${WSLAY_NETIO}")
