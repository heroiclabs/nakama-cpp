set(NAKAMA_GIT_TAG "master" CACHE STRING "nakama Git tag to fetch Proto files from")
set(NAKAMA_COMMON_GIT_TAG "master" CACHE STRING "nakama-common Git tag to fetch Proto files from")

# unconditional, because core-rt is always built and it uses nakama-api-proto
# TODO: split buildProtoFiles into grpc and api-proto
find_package(Protobuf CONFIG REQUIRED)
include(buildProtoFiles.cmake)

add_subdirectory(common EXCLUDE_FROM_ALL)
add_subdirectory(src)
add_subdirectory(core-rt EXCLUDE_FROM_ALL)
add_subdirectory(core-rest EXCLUDE_FROM_ALL)

## for createRtClient()
target_link_libraries(nakama-sdk-core-rest PUBLIC nakama::sdk-core-rt)

if (WITH_GRPC_CLIENT)
    add_subdirectory(core-grpc EXCLUDE_FROM_ALL)
endif ()
