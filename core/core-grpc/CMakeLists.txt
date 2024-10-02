project(nakama-sdk-cpp-core-grpc LANGUAGES CXX)

file(
        GLOB_RECURSE srcs
        LIST_DIRECTORIES false
        "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(gRPC CONFIG REQUIRED)

add_library(nakama-sdk-core-grpc OBJECT ${srcs})
add_library(nakama::sdk-core-grpc ALIAS nakama-sdk-core-grpc)
target_link_libraries(nakama-sdk-core-grpc
        PUBLIC
                nakama::sdk-core-rt
                nakama::grpc-proto
                nakama::sdk-core-common
                nakama::sdk-interface
        )
