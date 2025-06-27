file(
        GLOB_RECURSE srcs
        LIST_DIRECTORIES false
        "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
find_package(cpprestsdk CONFIG REQUIRED)

add_library(nakama-impl-http-cppRest OBJECT ${srcs})
target_link_libraries(nakama-impl-http-cppRest
        PUBLIC nakama-api-proto nakama::sdk-interface
        PRIVATE cpprestsdk::cpprest
)
add_library(nakama::impl-http-cppRest ALIAS nakama-impl-http-cppRest)
set(HTTP_IMPL_LIB nakama-impl-http-cppRest PARENT_SCOPE)
