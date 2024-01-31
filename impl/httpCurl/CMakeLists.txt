file(
        GLOB_RECURSE srcs
        LIST_DIRECTORIES false
        "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_library(nakama-impl-http-libcurl OBJECT ${srcs})

if (BUILD_CURL_IO_SYSTEM)
    find_package(CURL REQUIRED)
else()
    find_package(CURL CONFIG REQUIRED)
endif()

target_link_libraries(nakama-impl-http-libcurl
    PUBLIC CURL::libcurl
)

target_link_libraries(nakama-impl-http-libcurl
    PUBLIC nakama-api-proto nakama::sdk-interface
)

set(HTTP_IMPL_LIB nakama-impl-http-libcurl PARENT_SCOPE)
