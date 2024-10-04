
cmake_minimum_required(VERSION 3.23)

project(satori-test LANGUAGES C CXX)

file(
		GLOB SRCS
		LIST_DIRECTORIES false
		"${CMAKE_CURRENT_SOURCE_DIR}/*.c*"
)

include_directories("${CMAKE_CURRENT_SOURCE_DIR}")

add_executable(satori-test MACOSX_BUNDLE ${SRCS} "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp" $<TARGET_OBJECTS:satori-client>)
target_link_libraries(satori-test PUBLIC satori-client)

target_compile_features(satori-test PRIVATE cxx_std_14)

install(TARGETS satori-test
		EXPORT satori-test-export
		DESTINATION bin
)

if (CMAKE_SYSTEM_NAME MATCHES "Windows" OR CMAKE_SYSTEM_NAME MATCHES "Linux")
	add_custom_target(run
			COMMAND ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR}/satori-test
			USES_TERMINAL
			DEPENDS install
	)
elseif (CMAKE_SYSTEM_NAME MATCHES "Darwin")
	add_custom_target(run
			COMMAND $<TARGET_BUNDLE_DIR:satori-test>/Contents/MacOS/satori-test
			USES_TERMINAL
			DEPENDS install
	)
endif()