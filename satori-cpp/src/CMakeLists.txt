project(satori-client-cpp-base LANGUAGES CXX)

file(
	GLOB_RECURSE INCLUDES
	LIST_DIRECTORIES false
	"${CMAKE_CURRENT_SOURCE_DIR}/*.h"
)

file(
	GLOB_RECURSE SRCS
	LIST_DIRECTORIES false
	"${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

include_directories("${CMAKE_CURRENT_SOURCE_DIR}")

add_library(satori-client OBJECT ${INCLUDES} ${SRCS})
add_library(satori::client ALIAS satori-client)

target_link_libraries(satori-client
	PUBLIC
		satori::sdk-interface
	PRIVATE
		${HTTP_IMPL_LIB}
		nakama::sdk-core-common
)