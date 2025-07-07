file(
        GLOB_RECURSE srcs
        LIST_DIRECTORIES false
        "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_library(nakama-sdk-core-rt OBJECT ${srcs})
add_library(nakama::sdk-core-rt ALIAS nakama-sdk-core-rt)
target_link_libraries(nakama-sdk-core-rt
        PUBLIC
        nakama::sdk-interface
        nakama::api-proto
        nakama::sdk-core-common
        protobuf::libprotobuf
        PRIVATE
        nakama::sdk-rtclient-factory  # because of BaseClient.cpp
)
