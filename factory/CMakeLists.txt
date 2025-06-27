add_library(nakama-sdk-client-factory OBJECT ClientFactory.cpp)
add_library(nakama::client-factory ALIAS nakama-sdk-client-factory)
target_link_libraries(nakama-sdk-client-factory
        PRIVATE nakama-sdk-core-rest nakama-sdk-core-grpc ${HTTP_IMPL_LIB})

if (BUILD_GRPC_CLIENT)
    target_compile_definitions(nakama-sdk-client-factory PRIVATE BUILD_GRPC_CLIENT)
endif ()

target_compile_definitions(nakama-sdk-client-factory PRIVATE HTTP_COMPILE_DEFINITION)

add_library(nakama-sdk-rtclient-factory OBJECT NWebsocketsFactory.cpp)
add_library(nakama::rtclient-factory ALIAS nakama-sdk-rtclient-factory)

target_link_libraries(nakama-sdk-rtclient-factory
        PUBLIC nakama-sdk-interface
        PRIVATE ${WS_IMPL_LIB}
)
