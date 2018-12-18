cmake_minimum_required(VERSION 3.2)

project(nakama-cpp CXX)

IF (WIN32)
    macro(get_WIN32_WINNT version)
        if (WIN32 AND CMAKE_SYSTEM_VERSION)
            set(ver ${CMAKE_SYSTEM_VERSION})
            string(REPLACE "." "" ver ${ver})
            string(REGEX REPLACE "([0-9])" "0\\1" ver ${ver})

            set(${version} "0x${ver}")
        endif()
    endmacro()

    get_WIN32_WINNT(ver)
    add_definitions(-D_WIN32_WINNT=${ver})
ENDIF()

#add_subdirectory(thirdparty/catch)
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
)

include_directories(api)
include_directories(api/third_party/googleapis)
include_directories(../third_party/grpc/include)
include_directories(../third_party/grpc/third_party/protobuf/src)

add_library(nakama-cpp SHARED STATIC ${SOURCE_FILES})

install(TARGETS nakama-cpp DESTINATION ${INSTALL_LIB_DIR})
