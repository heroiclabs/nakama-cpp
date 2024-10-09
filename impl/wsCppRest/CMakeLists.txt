file(
        GLOB_RECURSE srcs
        LIST_DIRECTORIES false
        "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
find_package(cpprestsdk CONFIG REQUIRED)
find_package(OpenSSL REQUIRED)

add_library(nakama-impl-ws-cppRest OBJECT ${srcs})
target_include_directories(nakama-impl-ws-cppRest INTERFACE .)
target_link_libraries(nakama-impl-ws-cppRest
        PUBLIC nakama-api-proto nakama::sdk-interface
        PRIVATE cpprestsdk::cpprest OpenSSL::SSL
        )
add_library(nakama::impl-ws-cppRest ALIAS nakama-impl-ws-cppRest)
