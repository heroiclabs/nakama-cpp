file(
		GLOB_RECURSE srcs
		LIST_DIRECTORIES false
		"${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/*.h"
)

add_library(android-ca OBJECT ${srcs})
target_include_directories(android-ca PUBLIC
		"$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
		"$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>")

target_link_libraries(android-ca
		PRIVATE android nakama-sdk-core-misc
)
