cmake_minimum_required(VERSION 3.6)
project(hotjoin_bench_v241)

set(CMAKE_CXX_STANDARD 14)

set(NAKAMA_SHARED_LIBRARY ON)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../nakama-cpp-sdk ${CMAKE_CURRENT_BINARY_DIR}/nakama-cpp-sdk)

add_executable(hotjoin_bench hotjoin_bench.cpp)
target_link_libraries(hotjoin_bench PRIVATE ext_nakama-cpp)

CopyNakamaSharedLib(hotjoin_bench)
