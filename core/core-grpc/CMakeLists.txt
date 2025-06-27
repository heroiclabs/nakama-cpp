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
target_link_libraries(nakama-sdk-core-grpc
		PUBLIC nakama-sdk-interface nakama-grpc-proto
		PUBLIC nakama-sdk-core-common
		nakama-sdk-core-rt
)