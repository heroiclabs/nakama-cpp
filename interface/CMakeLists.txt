include(GNUInstallDirs)

file(GLOB_RECURSE CORE_HEADERS
        include/*.h
)

add_library(nakama-sdk-interface INTERFACE ${CORE_HEADERS})
add_library(nakama::sdk-interface ALIAS nakama-sdk-interface)

# Create configure file with all
configure_file(config.h.in nakama-cpp/config.h)

target_sources(nakama-sdk-interface INTERFACE
        FILE_SET HEADERS FILES BASE_DIRS ${CMAKE_CURRENT_BINARY_DIR} FILES ${CMAKE_CURRENT_BINARY_DIR}/nakama-cpp/config.h
        FILE_SET HEADERS FILES BASE_DIRS include FILES ${CORE_HEADERS}
)

target_include_directories(nakama-sdk-interface INTERFACE
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>")
