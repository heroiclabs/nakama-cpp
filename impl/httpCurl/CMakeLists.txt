file(
        GLOB_RECURSE srcs
        LIST_DIRECTORIES false
        "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_library(nakama-impl-http-libcurl OBJECT ${srcs})

target_include_directories(nakama-impl-http-libcurl PRIVATE ${CURL_INCLUDE_DIRS})

target_link_libraries(nakama-impl-http-libcurl
    PUBLIC nakama-api-proto nakama::sdk-interface
)

set(HTTP_IMPL_LIB nakama-impl-http-libcurl PARENT_SCOPE)
