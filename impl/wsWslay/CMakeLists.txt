file(
        GLOB_RECURSE srcs
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

target_include_directories(nakama-impl-ws-wslay INTERFACE
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>")


if (CFG_WSLAY_CUR_IO)
    target_include_directories(nakama-impl-ws-wslay PRIVATE ${CURL_INCLUDE_DIRS})
endif()
