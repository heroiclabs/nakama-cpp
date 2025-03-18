file(GLOB_RECURSE SOURCES include/*.h src/*.cpp src/*.h main.cpp)

enable_testing()

include_directories(include src)

if (APPLE)
    add_executable(nakama-sdk-test MACOSX_BUNDLE ${SOURCES})
else ()
    add_executable(nakama-sdk-test ${SOURCES})
endif ()


target_link_libraries(nakama-sdk-test PRIVATE nakama-sdk)

if (MSVC)
    target_compile_options(nakama-sdk-test PRIVATE /W4 /wd4251 /wd4267 /utf-8)
    target_link_options(nakama-sdk-test PRIVATE /DEBUG)
else ()
    target_compile_options(nakama-sdk-test PRIVATE -Wall -Wextra -Wpedantic -Wno-unknown-pragmas)
endif ()

add_test(NAME "All" COMMAND nakama-sdk-test)
