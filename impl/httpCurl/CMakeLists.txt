file(
        GLOB_RECURSE srcs
        LIST_DIRECTORIES false
        "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_library(nakama-impl-http-libcurl OBJECT ${srcs})
add_library(nakama::impl-http-libcurl ALIAS nakama-impl-http-libcurl)

target_include_directories(nakama-impl-http-libcurl PRIVATE ${CURL_INCLUDE_DIRS})

target_link_libraries(nakama-impl-http-libcurl
        PUBLIC nakama-api-proto nakama::sdk-interface
)
