file(
        GLOB_RECURSE srcs
        LIST_DIRECTORIES false
        "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_library(nakama-impl-ws-libhttpclient OBJECT ${srcs})
target_link_libraries(nakama-impl-ws-libhttpclient
        PUBLIC nakama-api-proto nakama::sdk-interface
        # libhttpclient is OBJECT library, so this link wont create
        # link time dependency on libhttpclient, but still going to
        # configure include paths
        PUBLIC ${LIBHTTPCLIENT_TARGET}
        )

