project(satori-client-cpp-base LANGUAGES CXX)

file(
	GLOB_RECURSE SRCS
	LIST_DIRECTORIES false
	"${CMAKE_CURRENT_SOURCE_DIR}/*.pb.cc"
	"${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/*.h"
)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
#find_package(gRPC CONFIG REQUIRED)

include_directories("${CMAKE_CURRENT_SOURCE_DIR}")

add_library(satori-client OBJECT ${SRCS})
add_library(satori::client ALIAS satori-client)

target_link_libraries(satori-client
	PUBLIC
		${HTTP_IMPL_LIB}
		satori::ccp-interface   # Remove this and both absl and google include folders when the line below works commented out, together with the line find_package(gRPC CONFIG REQUIRED) above
		#gRPC::grpc++   # This is the proper way of including the protobuf grpc headers (https://github.com/grpc/grpc/tree/master/src/cpp#to-start-using-grpc-c), but I need to figure out how to properly test them. Seems like this has the answer (https://stackoverflow.com/questions/70700592/what-is-a-correct-way-to-setup-protobuf-and-grpc-for-cpp-project)
)