file(
        GLOB_RECURSE srcs
        LIST_DIRECTORIES false
        "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_library(nakama-sdk-core-rest OBJECT ${srcs})
add_library(nakama::sdk-core-rest ALIAS nakama-sdk-core-rest)
target_compile_definitions(nakama-sdk-core-rest PRIVATE
        RAPIDJSON_HAS_STDSTRING=1

        # on consoles rapidjson fail to detect C++11 strings presence
        # we know we have them, so set them unconditionally
        RAPIDJSON_HAS_CXX11_RVALUE_REFS=1
)
target_link_libraries(nakama-sdk-core-rest
        PUBLIC
        nakama::sdk-interface
        nakama::sdk-core-common
        PRIVATE rapidjson
        nakama::sdk-core-rt
)
