cmake_minimum_required(VERSION 3.13)

project(nakama-test CXX)

include(windows_support)

set(_src_root_path ${CMAKE_CURRENT_LIST_DIR})
file(
    GLOB_RECURSE _source_list
    LIST_DIRECTORIES false
    "${_src_root_path}/*.c*"
    "${_src_root_path}/*.h*"
)

add_executable(nakama-test ${_source_list})
target_compile_features(nakama-test PRIVATE cxx_std_11)
add_definitions(-DNAKAMA_API=)
target_link_libraries(nakama-test PRIVATE nakama-cpp)

target_include_directories(nakama-test
    PRIVATE .
)

foreach(_source IN ITEMS ${_source_list})
    get_filename_component(_source_path "${_source}" PATH)
    file(RELATIVE_PATH _source_path_rel "${_src_root_path}" "${_source_path}")
    string(REPLACE "/" "\\" _group_path "${_source_path_rel}")
    source_group("src\\${_group_path}" FILES "${_source}")
endforeach()
