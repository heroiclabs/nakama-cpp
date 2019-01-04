cmake_minimum_required(VERSION 3.13)

project(nakama-cpp CXX)

include(windows_support)

set(SOURCE_FILES
    api/github.com/heroiclabs/nakama/api/api.pb.cc
    api/github.com/heroiclabs/nakama/api/api.pb.h
    api/github.com/heroiclabs/nakama/apigrpc/apigrpc.grpc.pb.cc
    api/github.com/heroiclabs/nakama/apigrpc/apigrpc.grpc.pb.h
    api/github.com/heroiclabs/nakama/apigrpc/apigrpc.pb.cc
    api/github.com/heroiclabs/nakama/apigrpc/apigrpc.pb.h
    api/github.com/heroiclabs/nakama/rtapi/realtime.pb.cc
    api/github.com/heroiclabs/nakama/rtapi/realtime.pb.h
    api/protoc-gen-swagger/options/annotations.pb.cc
    api/protoc-gen-swagger/options/annotations.pb.h
    api/protoc-gen-swagger/options/openapiv2.pb.cc
    api/protoc-gen-swagger/options/openapiv2.pb.h
    api/google/api/annotations.pb.cc
    api/google/api/annotations.pb.h
    api/google/api/http.pb.cc
    api/google/api/http.pb.h
    api/google/rpc/status.pb.cc
    api/google/rpc/status.pb.h
    nakama.cpp
    strutil.cpp
    default_client.cpp
    default_client.h
    ${PUBLIC_INCLUDE_DIR}/nakama-cpp/strutil.h
    ${PUBLIC_INCLUDE_DIR}/nakama-cpp/nakama.h
    ${PUBLIC_INCLUDE_DIR}/nakama-cpp/client_interface.h
    ${PUBLIC_INCLUDE_DIR}/nakama-cpp/default_client.h
)

include_directories(
    api
    ../third_party/grpc/include
    ../third_party/grpc/third_party/protobuf/src
    ${PUBLIC_INCLUDE_DIR}
)

add_library(nakama-cpp SHARED STATIC ${SOURCE_FILES})
target_compile_features(nakama-cpp PRIVATE cxx_std_11)
target_link_libraries(nakama-cpp PRIVATE grpc++ libprotobuf)

install(TARGETS nakama-cpp DESTINATION ${INSTALL_LIB_DIR})
