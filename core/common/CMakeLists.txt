
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

 add_library(nakama-sdk-core-common OBJECT ${srcs})
 add_library(nakama::sdk-core-common ALIAS nakama-sdk-core-common)

 target_link_libraries(nakama-sdk-core-common
         PUBLIC
         nakama-sdk-interface
         # required for datahelper
         nakama::api-proto
         $<TARGET_NAME_IF_EXISTS:nakama::grpc-proto>
         PRIVATE
         protobuf::libprotobuf   # because of StrUtil::Base64Encode
 )

 target_include_directories(nakama-sdk-core-common
         PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
 )


