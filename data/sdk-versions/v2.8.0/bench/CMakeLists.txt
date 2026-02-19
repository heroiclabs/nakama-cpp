cmake_minimum_required(VERSION 3.14)
project(hotjoin_bench_v280)

set(CMAKE_CXX_STANDARD 17)

find_library(NAKAMA_SDK nakama-sdk
    PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../macosx-arm64/lib
    NO_DEFAULT_PATH
)

add_executable(hotjoin_bench hotjoin_bench.cpp)
target_include_directories(hotjoin_bench PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../macosx-arm64/lib/nakama-sdk.framework/Headers
)
target_link_libraries(hotjoin_bench PRIVATE ${NAKAMA_SDK})
