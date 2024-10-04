cmake_minimum_required(VERSION 3.22)
project(satori-sdk-cpp DESCRIPTION "Satori C++ Client" LANGUAGES CXX)

add_subdirectory(src)
add_subdirectory(test EXCLUDE_FROM_ALL)
