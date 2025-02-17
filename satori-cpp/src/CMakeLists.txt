project(satori-client-cpp-base LANGUAGES CXX)

file(
	GLOB_RECURSE SRCS
	LIST_DIRECTORIES false
	"${CMAKE_CURRENT_SOURCE_DIR}/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

include_directories("${CMAKE_CURRENT_SOURCE_DIR}")

add_library(satori-client OBJECT ${SRCS})
add_library(satori::client ALIAS satori-client)

target_link_libraries(satori-client
	PRIVATE
		nakama::sdk-core-common
		nakama::sdk-interface
		satori::api-proto
)