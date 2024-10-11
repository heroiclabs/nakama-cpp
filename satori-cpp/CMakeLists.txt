set(SATORI_GIT_TAG "master" CACHE STRING "satori Git tag to fetch Proto files from")

file(GLOB_RECURSE SATORI_CPP_HEADERS
		include/*.h
)
# TODO: split buildProtoFiles into grpc and api-proto
find_package(Protobuf CONFIG REQUIRED)
include(buildProtoFiles.cmake)
#add_library(satori-ccp-interface INTERFACE ${SATORI_CORE_HEADERS})
#add_library(satori::ccp-interface ALIAS satori-ccp-interface)

#target_include_directories(satori-ccp-interface INTERFACE
#"$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
#"$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>")

add_subdirectory(src)
add_subdirectory(test EXCLUDE_FROM_ALL)
