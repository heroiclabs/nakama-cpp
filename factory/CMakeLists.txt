add_library(nakama-sdk-client-factory OBJECT ClientFactory.cpp)
add_library(nakama::client-factory ALIAS nakama-sdk-client-factory)
target_link_libraries(nakama-sdk-client-factory
        PRIVATE nakama-sdk-core-rest nakama-sdk-core-grpc ${HTTP_IMPL_LIB})

if (BUILD_GRPC_CLIENT)
    target_compile_definitions(nakama-sdk-client-factory PRIVATE BUILD_GRPC_CLIENT)
endif()

if(BUILD_HTTP_LIBHTTPCLIENT)
    target_compile_definitions(nakama-sdk-client-factory PRIVATE BUILD_HTTP_LIBHTTPCLIENT)
else()
    target_compile_definitions(nakama-sdk-client-factory PRIVATE WITH_EXTERNAL_HTTP)
endif()

add_library(nakama-sdk-rtclient-factory OBJECT NWebsocketsFactory.cpp)
add_library(nakama::rtclient-factory ALIAS nakama-sdk-rtclient-factory)
target_link_libraries(nakama-sdk-rtclient-factory
        PUBLIC nakama-sdk-interface
        PRIVATE ${WS_IMPL_LIB}
        )

if (BUILD_WEBSOCKET_LIBHTTPCLIENT)
    target_compile_definitions(nakama-sdk-rtclient-factory PRIVATE BUILD_WEBSOCKET_LIBHTTPCLIENT)
elseif (BUILD_WEBSOCKET_WSLAY)
    target_compile_definitions(nakama-sdk-rtclient-factory PRIVATE BUILD_WEBSOCKET_WSLAY)
else()
    target_compile_definitions(nakama-sdk-rtclient-factory PRIVATE WITH_EXTERNAL_WS)
endif()

if (BUILD_CURL)
    target_compile_definitions(nakama-sdk-rtclient-factory PRIVATE BUILD_CURL)
endif()
