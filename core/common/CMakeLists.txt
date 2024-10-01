
 execute_process(
        COMMAND git log -1 --format=%h
        WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
        OUTPUT_VARIABLE GIT_HASH
        OUTPUT_STRIP_TRAILING_WHITESPACE
        )

configure_file(Version.cpp.in Version.cpp @ONLY)

file(
        GLOB_RECURSE srcs
        LIST_DIRECTORIES false
        "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
        "${CMAKE_CURRENT_BINARY_DIR}/Version.cpp"
)

#set(CMAKE_INCLUDE_CURRENT_DIR ON)
#set(CMAKE_INCLUDE_CURRENT_DIR_IN_INTERFACE ON)
find_package(absl CONFIG REQUIRED)
find_package(Threads REQUIRED)

add_library(nakama-sdk-core-common OBJECT ${srcs})

target_link_libraries(nakama-sdk-core-common
        PUBLIC
            nakama-sdk-interface
            # required for datahelper
            nakama::api-proto
            $<$<TARGET_EXISTS:nakama::grpc-proto>:nakama::grpc-proto>
        PRIVATE
            absl::strings   # because of StrUtil::Base64Encode
        )

target_include_directories(nakama-sdk-core-common PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>
)


