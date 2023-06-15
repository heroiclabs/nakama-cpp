file(
        GLOB_RECURSE srcs
        LIST_DIRECTORIES false
        "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_library(nakama-impl-http-libcurl OBJECT ${srcs})

if (NOT BUILD_CURL_HTTP_SYSTEM)
    find_package(CURL CONFIG REQUIRED)

    target_link_libraries(nakama-impl-http-libcurl
        PUBLIC nakama-api-proto nakama::sdk-interface CURL::libcurl
    )
endif()

set(HTTP_IMPL_LIB nakama-impl-http-libcurl PARENT_SCOPE)
