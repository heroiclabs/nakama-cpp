cmake_minimum_required(VERSION 3.23)
project(nakama-sdk-core LANGUAGES CXX)
include(GNUInstallDirs)

file(GLOB CORE_HEADERS
        include/nakama-cpp/*
        include/nakama-cpp/data/*
        include/nakama-cpp/data/log/*
        include/nakama-cpp/data/realtime/*
        include/nakama-cpp/data/realtime/rtdata/*
)

if (WS_IMPL STREQUAL "wslay")
    file(GLOB WSLAY_HEADERS include/nakama-cpp/data/realtime/wslay/*)
    list(APPEND CORE_HEADERS ${WSLAY_HEADERS})
endif ()


add_library(nakama-sdk-interface INTERFACE ${CORE_HEADERS})
add_library(nakama::sdk-interface ALIAS nakama-sdk-interface)

# Force nonstd::optional save our choice in config.h
set(optional_CONFIG_SELECT_OPTIONAL optional_OPTIONAL_NONSTD)
target_compile_definitions(nakama-sdk-interface INTERFACE optional_CONFIG_SELECT_OPTIONAL=${optional_CONFIG_SELECT_OPTIONAL})

if (FORCE_DLL_IMPORT_EXPORT)
    target_compile_definitions(nakama-sdk-interface INTERFACE FORCE_DLL_IMPORT_EXPORT)
endif ()

# Create configure file with all
configure_file(config.h.in nakama-cpp/config.h)

target_sources(nakama-sdk-interface INTERFACE
        FILE_SET HEADERS FILES BASE_DIRS ${CMAKE_CURRENT_BINARY_DIR} FILES ${CMAKE_CURRENT_BINARY_DIR}/nakama-cpp/config.h
        FILE_SET HEADERS FILES BASE_DIRS include FILES ${CORE_HEADERS}
)

target_include_directories(nakama-sdk-interface INTERFACE
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>")

target_link_libraries(nakama-sdk-interface INTERFACE nonstd::optional-lite)

