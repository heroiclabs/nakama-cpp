add_library(nakama-sdk-client-factory OBJECT ClientFactory.cpp SatoriClientFactory.cpp)
add_library(nakama::sdk-client-factory ALIAS nakama-sdk-client-factory)
target_link_libraries(nakama-sdk-client-factory
        PRIVATE nakama::sdk-core-rest
            $<TARGET_NAME_IF_EXISTS:nakama::sdk-core-grpc>)
target_compile_definitions(nakama-sdk-client-factory PRIVATE
        $<$<BOOL:${WITH_GRPC_CLIENT}>:WITH_GRPC_CLIENT>
        $<$<BOOL:${WITH_HTTP_CURL}>:WITH_HTTP_CURL>
        $<$<BOOL:${WITH_HTTP_LIBHTTPC}>:WITH_HTTP_LIBHTTPC>
        $<$<BOOL:${WITH_HTTP_CPPREST}>:WITH_HTTP_CPPREST>
)

add_library(nakama-sdk-rtclient-factory OBJECT NWebsocketsFactory.cpp)
add_library(nakama::sdk-rtclient-factory ALIAS nakama-sdk-rtclient-factory)
target_link_libraries(nakama-sdk-rtclient-factory
        PUBLIC nakama::sdk-interface)
target_compile_definitions(nakama-sdk-rtclient-factory PRIVATE
        $<$<BOOL:${WITH_WS_WSLAY}>:WITH_WS_WSLAY>
        $<$<BOOL:${CFG_WSLAY_CURL_IO}>:CFG_WSLAY_CURL_IO>
        $<$<BOOL:${WITH_WS_LIBHTTPC}>:WITH_WS_LIBHTTPC>
        $<$<BOOL:${WITH_WS_CPPREST}>:WITH_WS_CPPREST>
)
