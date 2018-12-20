cmake_minimum_required(VERSION 3.13)

project(nakama-test CXX)

include(windows_support)

set(SOURCE_FILES
    test.cpp
)

include_directories(
    ../include
)

add_executable(nakama-test ${SOURCE_FILES})
target_link_libraries(nakama-test PRIVATE nakama-cpp)
