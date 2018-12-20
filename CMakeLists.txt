cmake_minimum_required(VERSION 3.13)

project(nakama-cpp-all)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)

include(windows_support)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(INSTALL_LIB_DIR ${PROJECT_SOURCE_DIR}/lib)

add_subdirectory(src)
add_subdirectory(third_party/grpc)
add_subdirectory(test)
