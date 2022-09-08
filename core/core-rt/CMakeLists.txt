project(nakama-cpp-sdk-core-rt LANGUAGES CXX)

file(
        GLOB_RECURSE srcs
        LIST_DIRECTORIES false
        "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_library(nakama-sdk-core-rt OBJECT ${srcs})
target_link_libraries(nakama-sdk-core-rt
        PUBLIC nakama-sdk-interface nakama-api-proto
        PUBLIC nakama-sdk-core-common
        PRIVATE nakama::rtclient-factory  # because of BaseClient.cpp
        )
