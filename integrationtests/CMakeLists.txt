
if(BUILD_PRIVATE_TEST AND EXISTS "${CMAKE_SOURCE_DIR}/submodules/private/test/CMakeLists.txt")
	file(GLOB_RECURSE SOURCES include/*.h src/*.cpp src/*.h ${CMAKE_SOURCE_DIR}/submodules/private/test/main.cpp)
else()
	file(GLOB_RECURSE SOURCES include/*.h src/*.cpp src/*.h main.cpp)
endif()

enable_testing()

include_directories(include src)

set(TEST_TARGET nakama-sdk-test)

if (ANDROID)
    set(TEST_TARGET nakama-test)
    add_library(${TEST_TARGET} SHARED ${SOURCES})
elseif (APPLE)
    add_executable(${TEST_TARGET} MACOSX_BUNDLE ${SOURCES})
else ()
    add_executable(${TEST_TARGET} ${SOURCES})
endif ()

target_link_libraries(${TEST_TARGET} PRIVATE nakama-sdk rapidjson)

target_include_directories(${TEST_TARGET} PUBLIC include)

if (BUILD_PRIVATE_TEST)
    include("../submodules/private/test/CMakeLists.txt" OPTIONAL)
endif()

if (MSVC)
    target_compile_options(${TEST_TARGET} PRIVATE /W4 /wd4251 /wd4267 /utf-8)
    target_link_options(${TEST_TARGET} PRIVATE /DEBUG)
else ()
    target_compile_options(${TEST_TARGET} PRIVATE -Wall -Wextra -Wpedantic -Wno-unknown-pragmas)
endif ()

if (APPLE)
    set_target_properties(${TEST_TARGET} PROPERTIES
        XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER "com.heroiclabs.test"
        XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "Apple Development"
        XCODE_ATTRIBUTE_DEVELOPMENT_TEAM "Heroic Developer"
        XCODE_ATTRIBUTE_CODE_SIGN_STYLE Automatic
    )

endif()

if (NOT ANDROID)
    add_test(NAME "All" COMMAND ${TEST_TARGET})
endif()
