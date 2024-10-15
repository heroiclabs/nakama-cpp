set(SATORI_GIT_TAG "master" CACHE STRING "satori Git tag to fetch Proto files from")

file(GLOB_RECURSE SATORI_CORE_HEADERS
		include/*.h
)

add_library(satori-sdk-interface INTERFACE ${CORE_HEADERS})
add_library(satori::sdk-interface ALIAS satori-sdk-interface)

add_subdirectory(src)
add_subdirectory(test)

target_sources(satori-sdk-interface INTERFACE
	FILE_SET HEADERS FILES BASE_DIRS include FILES ${CORE_HEADERS}
)

target_compile_definitions(satori-sdk-interface INTERFACE CORE_HEADERS)

target_include_directories(satori-sdk-interface INTERFACE
	"$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
	"$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>")

target_link_libraries(satori-sdk-interface
	INTERFACE
		nakama::sdk-interface
)
