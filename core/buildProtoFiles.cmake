# Fetch proto files from nakama and nakama-common and builds them
include(FetchContent)

set(NAKAMA_COMMON ${CMAKE_CURRENT_BINARY_DIR}/nakama-common-master)
set(NAKAMA ${CMAKE_CURRENT_BINARY_DIR}/nakama-master)

set(NAKAMA_COMMON_ZIP ${NAKAMA_COMMON}.zip)
set(NAKAMA_ZIP ${NAKAMA}.zip)

file(DOWNLOAD https://github.com/heroiclabs/nakama-common/archive/refs/heads/master.zip ${NAKAMA_COMMON_ZIP} SHOW_PROGRESS)
file(DOWNLOAD https://github.com/heroiclabs/nakama/archive/refs/heads/master.zip ${NAKAMA_ZIP} SHOW_PROGRESS)

file(ARCHIVE_EXTRACT INPUT ${NAKAMA_COMMON_ZIP} DESTINATION ${CMAKE_CURRENT_BINARY_DIR} VERBOSE)
file(ARCHIVE_EXTRACT INPUT ${NAKAMA_ZIP} DESTINATION ${CMAKE_CURRENT_BINARY_DIR} VERBOSE)

#### API and RTAPI proto ####

file(GLOB_RECURSE NAKAMA_API_PROTO_FILES
        ${NAKAMA_COMMON}/api/*.proto
        ${NAKAMA_COMMON}/rtapi/*.proto
        )

message("adding library")

add_library(nakama-api-proto OBJECT ${NAKAMA_API_PROTO_FILES})
add_library(nakama::api-proto ALIAS nakama-api-proto)

target_link_libraries(nakama-api-proto PRIVATE protobuf::libprotobuf)
target_include_directories(nakama-api-proto PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

protobuf_generate(TARGET nakama-api-proto
        LANGUAGE cpp
        IMPORT_DIRS "${NAKAMA_COMMON}"
        PROTOC_OUT_DIR ${CMAKE_CURRENT_BINARY_DIR}
        )

message("done generating")

if (BUILD_GRPC_CLIENT)
#### apigrpc.proto ####

# apigrpc.proto references 'api.proto' using canonical name 'github.com/heroiclabs/nakama-common/api/api.proto'
# but very same files is used as `api/api.proto` by nakama-common itself. Protoc computes
# cannonical name using relative path from -I import dirs. As a result if same file
# is used with different imports, then -I args must be different and therefore cannonical
# name will be different. When proto files reference each other, generated code also
# references C/C++ symbols, which are derivied from canonical name. So even if we align `-I`
# perfectly so that protoc find `api.proto` under 2 different paths, compiled object files
# can't be linked together due to unresolved symbols.
#
# So here are creating stub 'github.com/heroiclabs/nakama-common/api/api.proto' file (not real one)
# which "forwards" all definitions to real `api/api.proto`, this way we make same definitions available
# under two different canonical names

set(api_compat_proto ${CMAKE_CURRENT_BINARY_DIR}/_proto/github.com/heroiclabs/nakama-common/api/api.proto)
file(WRITE ${api_compat_proto} "
syntax = \"proto3\";
package nakama.api;
import public \"api/api.proto\";
")

file(GLOB_RECURSE NAKAMA_GRPC_PROTO_FILES
        ${NAKAMA}/apigrpc/*.proto
        ${NAKAMA}/build/grpc-gateway-v2.3.0/third_party/googleapis/*.proto
        ${NAKAMA}/vendor/github.com/grpc-ecosystem/grpc-gateway/v2/*.proto
        )
add_library(nakama-grpc-proto OBJECT ${NAKAMA_GRPC_PROTO_FILES} ${api_compat_proto})
add_library(nakama::grpc-proto ALIAS nakama-grpc-proto)
target_link_libraries(nakama-grpc-proto
        PUBLIC nakama-api-proto
        PUBLIC -Wl,--exclude-libs=ALL
               gRPC::grpc++
        )
target_include_directories(nakama-grpc-proto PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
        )

install(TARGETS
        nakama-grpc-proto
        EXPORT nakama-export
        LIBRARY ARCHIVE RUNTIME
        )

# Order of import directories is VERY important. It is because generated .pb.c files import generated .pb.h
# using relative path calculated from import directories. We want internally generated identifier names
# to match import path. Docs unhelpfully doesn't mention any of it, except for a single vague sentence
# about canonical names and IMPORT_PATH at the bottom of https://developers.google.com/protocol-buffers/docs/proto3#generating
#
# TLDR; put most specific import paths first
set(NAKAMA_GRPC_PROTO_IMPORT_DIRS
        ${NAKAMA}/build/grpc-gateway-v2.3.0/third_party/googleapis  # dep of apigrpc.proto
        ${NAKAMA}/vendor/github.com/grpc-ecosystem/grpc-gateway/v2  # dep of apigrpc.proto
        ${CMAKE_CURRENT_BINARY_DIR}/_proto  # this makes 'github.com/heroiclabs/nakama-common/api/api.proto' import work
        ${NAKAMA}    # this path relative to .proto file gives us canonical name apigrpc/apigrpc.pb.h we want
        ${NAKAMA_COMMON}  # this is where real api/api.proto is
        )

find_package(gRPC CONFIG REQUIRED)
get_target_property(grpc_cpp_plugin_location gRPC::grpc_cpp_plugin LOCATION)
protobuf_generate(TARGET nakama-grpc-proto
        LANGUAGE cpp
        IMPORT_DIRS "${NAKAMA_GRPC_PROTO_IMPORT_DIRS}"
        PLUGIN protoc-gen-grpc=${grpc_cpp_plugin_location}
        GENERATE_EXTENSIONS .grpc.pb.h .grpc.pb.cc
        PROTOC_OUT_DIR ${CMAKE_CURRENT_BINARY_DIR}
        PROTOC_OPTIONS "--grpc_out=${CMAKE_CURRENT_BINARY_DIR}"
        )
endif(BUILD_GRPC_CLIENT)
