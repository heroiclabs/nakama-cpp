#### API proto ####

file(GLOB_RECURSE SATORI_API_PROTO_FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/api/*.proto
)

add_library(satori-api-proto OBJECT ${SATORI_API_PROTO_FILES})
add_library(satori::api-proto ALIAS satori-api-proto)

target_link_libraries(satori-api-proto PRIVATE protobuf::libprotobuf)
target_include_directories(satori-api-proto PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

protobuf_generate(TARGET satori-api-proto
        LANGUAGE cpp
        IMPORT_DIRS "${SATORI}"
        PROTOC_OUT_DIR ${CMAKE_CURRENT_BINARY_DIR}
        )

message("done generating satori api")
