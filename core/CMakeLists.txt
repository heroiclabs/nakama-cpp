cmake_minimum_required(VERSION 3.22)
project(nakama-sdk-cpp DESCRIPTION "Nakama C++ Client" LANGUAGES CXX)

set(NAKAMA_GIT_TAG "master" CACHE STRING "nakama Git tag to fetch Proto files from")
set(NAKAMA_COMMON_GIT_TAG "master" CACHE STRING "nakama-common Git tag to fetch Proto files from")
option(BUILD_GRPC_CLIENT "Build GRPC client" ON)
option(BUILD_REST_CLIENT "Builds REST client" ON)

# unconditional
find_package(RapidJSON CONFIG REQUIRED)

# unconditional, because core-rt is always built and it uses nakama-api-proto
# TODO: split buildProtoFiles into grpc and api-proto
find_package(Protobuf CONFIG REQUIRED)
include(buildProtoFiles.cmake)

add_subdirectory(common EXCLUDE_FROM_ALL)

add_subdirectory(src)

# TODO: Currently full RtClient is always built, regardless if websocket implementation is enabled
add_subdirectory(core-rt EXCLUDE_FROM_ALL)
#target_link_libraries(nakama-sdk-core
#        PUBLIC nakama-sdk-core-rt
#        )

if (BUILD_REST_CLIENT)
    add_subdirectory(core-rest EXCLUDE_FROM_ALL)
#    target_link_libraries(nakama-sdk-core
#            PUBLIC nakama-sdk-core-rest
#            )
endif()

if (BUILD_GRPC_CLIENT)
    add_subdirectory(core-grpc EXCLUDE_FROM_ALL)
#    target_link_libraries(nakama-sdk-core
#                PUBLIC nakama-sdk-core-grpc
#                )
endif()

