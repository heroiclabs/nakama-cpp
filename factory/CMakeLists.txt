add_library(nakama-sdk-client-factory OBJECT ClientFactory.cpp)
add_library(nakama::client-factory ALIAS nakama-sdk-client-factory)
target_link_libraries(nakama-sdk-client-factory
        PRIVATE nakama-sdk-core-rest nakama-sdk-core-grpc ${HTTP_IMPL_LIB})

if (BUILD_GRPC_CLIENT)
    target_compile_definitions(nakama-sdk-client-factory PRIVATE BUILD_GRPC_CLIENT)
endif()

if(WITH_LIBHTTPCLIENT_HTTP)
    target_compile_definitions(nakama-sdk-client-factory PRIVATE BUILD_HTTP_LIBHTTPCLIENT)
elseif(WITH_CPPRESTSDK)
    target_compile_definitions(nakama-sdk-client-factory PRIVATE BUILD_HTTP_CPPRESTSDK)
else()
    target_compile_definitions(nakama-sdk-client-factory PRIVATE WITH_EXTERNAL_HTTP)
endif()

add_library(nakama-sdk-rtclient-factory OBJECT NWebsocketsFactory.cpp)
add_library(nakama::rtclient-factory ALIAS nakama-sdk-rtclient-factory)

message("IMPL LIB IS ${WS_IMPL_LIB}")
target_link_libraries(nakama-sdk-rtclient-factory
        PUBLIC nakama-sdk-interface
        PRIVATE ${WS_IMPL_LIB}
        )

if (BUILD_CURL)
    target_compile_definitions(nakama-sdk-rtclient-factory PRIVATE BUILD_CURL)
endif()
