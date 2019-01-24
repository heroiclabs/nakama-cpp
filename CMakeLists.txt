cmake_minimum_required(VERSION 3.10)

project(nakama-cpp-all)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)

include(windows_support)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

option(BUILD_DEFAULT_WEBSOCKETS "Build default websockets transport" ON)

set(gRPC_BUILD_CSHARP_EXT OFF)
set(protobuf_BUILD_TESTS OFF)
set(BENCHMARK_ENABLE_LTO OFF)
set(BENCHMARK_ENABLE_TESTING OFF)
set(BUILD_STATIC_LIBS ON)
set(BUILD_SHARED_LIBS OFF)
set(CMAKE_CROSSCOMPILING 1)
set(RUN_HAVE_STD_REGEX 0)
set(RUN_HAVE_POSIX_REGEX 0)
set(RUN_HAVE_GNU_POSIX_REGEX 0)
set(RUN_HAVE_STEADY_CLOCK 0)

set(INSTALL_LIB_DIR ${PROJECT_SOURCE_DIR}/lib)
set(PUBLIC_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include)

add_subdirectory(src)
add_subdirectory(third_party/grpc)

if(WIN32)
add_subdirectory(test)
endif()
