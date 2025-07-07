file(
        GLOB_RECURSE srcs
        LIST_DIRECTORIES false
        "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
)

find_library(log-lib log)

add_library(android-ca OBJECT ${srcs})
add_library(nakama::android-ca ALIAS android-ca)
target_include_directories(android-ca PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(android-ca
        PRIVATE ${log-lib} nakama-sdk-core-misc
)
