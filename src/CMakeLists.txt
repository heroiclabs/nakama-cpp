cmake_minimum_required(VERSION 3.13)

project(nakama-cpp CXX)

include(windows_support)

set(_src_root_path ${CMAKE_CURRENT_LIST_DIR})
file(
    GLOB_RECURSE _source_list
    LIST_DIRECTORIES false
    "${_src_root_path}/*.c*"
    "${_src_root_path}/*.h*"
)

set(_include_root_path ${PUBLIC_INCLUDE_DIR}/nakama-cpp)

file(
    GLOB_RECURSE _public_headers_list
    LIST_DIRECTORIES false
    "${_include_root_path}/*.h*"
)

include_directories(
    api
    ../third_party/grpc/include
    ../third_party/grpc/third_party/protobuf/src
    ${PUBLIC_INCLUDE_DIR}
)

add_library(nakama-cpp SHARED STATIC ${_source_list} ${_public_headers_list})
target_compile_features(nakama-cpp PRIVATE cxx_std_11)
add_definitions(-DNAKAMA_API=)
target_link_libraries(nakama-cpp PRIVATE grpc++ libprotobuf)

foreach(_source IN ITEMS ${_source_list})
    get_filename_component(_source_path "${_source}" PATH)
    file(RELATIVE_PATH _source_path_rel "${_src_root_path}" "${_source_path}")
    string(REPLACE "/" "\\" _group_path "${_source_path_rel}")
    source_group("src\\${_group_path}" FILES "${_source}")
endforeach()

foreach(_source IN ITEMS ${_public_headers_list})
    get_filename_component(_source_path "${_source}" PATH)
    file(RELATIVE_PATH _source_path_rel "${_include_root_path}" "${_source_path}")
    string(REPLACE "/" "\\" _group_path "${_source_path_rel}")
    source_group("include\\${_group_path}" FILES "${_source}")
endforeach()

install(TARGETS nakama-cpp DESTINATION ${INSTALL_LIB_DIR})
