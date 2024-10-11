project(satori-client-cpp-base LANGUAGES CXX)

file(
	GLOB_RECURSE SRCS
	LIST_DIRECTORIES false
	"${CMAKE_CURRENT_SOURCE_DIR}/*.pb.cc"
	"${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/*.h"
)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Protobuf CONFIG REQUIRED)

include_directories("${CMAKE_CURRENT_SOURCE_DIR}")

add_library(satori-client OBJECT ${SRCS})
add_library(satori::client ALIAS satori-client)

target_link_libraries(satori-client
	PUBLIC
		${HTTP_IMPL_LIB} # We whould use this if this client had all the implementations that nakama has, but for now, it only has http rest
		satori-api-proto
	PRIVATE
		protobuf::libprotobuf
)