cmake_minimum_required(VERSION 3.22)
project(satori-sdk-cpp DESCRIPTION "Satori C++ Client" LANGUAGES CXX)

file(GLOB_RECURSE SATORI_CORE_HEADERS
		include/*.h
)
#add_library(satori-ccp-interface INTERFACE ${SATORI_CORE_HEADERS})
#add_library(satori::ccp-interface ALIAS satori-ccp-interface)

#target_include_directories(satori-ccp-interface INTERFACE
#"$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
#"$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>")

add_subdirectory(src)
add_subdirectory(test EXCLUDE_FROM_ALL)
