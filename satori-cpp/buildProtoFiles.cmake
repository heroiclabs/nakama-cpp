#### API proto ####
find_package(Protobuf CONFIG REQUIRED)
file(GLOB_RECURSE SATORI_API_PROTO_FILES
        ${CMAKE_CURRENT_LIST_DIR}/api/*.proto
		${NAKAMA}/build/grpc-gateway-v2.3.0/third_party/googleapis/*.proto
		${NAKAMA}/vendor/github.com/grpc-ecosystem/grpc-gateway/v2/*.proto
)
add_library(satori-api-proto OBJECT ${SATORI_API_PROTO_FILES})
add_library(satori::api-proto ALIAS satori-api-proto)

target_link_libraries(satori-api-proto
		PRIVATE protobuf::libprotobuf
				nakama::api-proto
#		PRIVATE -Wl,--exclude-libs=ALL
#				gRPC::grpc++
)

target_include_directories(satori-api-proto PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

protobuf_generate(TARGET satori-api-proto
		LANGUAGE cpp
		IMPORT_DIRS "${CMAKE_CURRENT_LIST_DIR}/api"
		PROTOC_OUT_DIR ${CMAKE_CURRENT_BINARY_DIR}
)

install(TARGETS
		satori-api-proto
		EXPORT satori-export
		LIBRARY ARCHIVE RUNTIME
)

message("done generating satori proto")

## Order of import directories is VERY important. It is because generated .pb.c files import generated .pb.h
## using relative path calculated from import directories. We want internally generated identifier names
## to match import path. Docs unhelpfully doesn't mention any of it, except for a single vague sentence
## about canonical names and IMPORT_PATH at the bottom of https://developers.google.com/protocol-buffers/docs/proto3#generating
##
## TLDR; put most specific import paths first
#set(SATORI_GRPC_PROTO_IMPORT_DIRS
#        ${CACHED_NAKAMA_BINARY_DIR}/build/grpc-gateway-v2.3.0/third_party/googleapis  # dep of apigrpc.proto
#        ${CACHED_NAKAMA_BINARY_DIR}/vendor/github.com/grpc-ecosystem/grpc-gateway/v2  # dep of apigrpc.proto
#        ${CMAKE_CURRENT_BINARY_DIR}
#)
#
#find_package(gRPC CONFIG REQUIRED)
#get_target_property(grpc_cpp_plugin_location gRPC::grpc_cpp_plugin LOCATION)
#message("SATORI_GRPC_PROTO_IMPORT_DIRS: " ${SATORI_GRPC_PROTO_IMPORT_DIRS})
#
#protobuf_generate(TARGET satori-api-proto
#        LANGUAGE cpp
#        IMPORT_DIRS "${SATORI_GRPC_PROTO_IMPORT_DIRS}"
#        PLUGIN protoc-gen-grpc=${grpc_cpp_plugin_location}
#        GENERATE_EXTENSIONS .grpc.pb.h .grpc.pb.cc
#        PROTOC_OUT_DIR ${CMAKE_CURRENT_BINARY_DIR}
#        PROTOC_OPTIONS "--grpc_out=${CMAKE_CURRENT_BINARY_DIR}"
#)
#
#message("done generating satori api")