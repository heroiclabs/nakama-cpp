file(GLOB SOURCES src/*.cpp src/*.h main.cpp)

enable_testing()

if (APPLE)
    add_executable(nakama-sdk-test MACOSX_BUNDLE ${SOURCES})
else ()
    add_executable(nakama-sdk-test ${SOURCES})
endif ()


target_link_libraries(nakama-sdk-test PRIVATE nakama-sdk)

if (MSVC)
    target_compile_options(nakama-sdk-test PRIVATE /W4 /WX /wd4251 /wd4267 /utf-8)
    target_link_options(nakama-sdk-test PRIVATE /WX /DEBUG)
else ()
    target_compile_options(nakama-sdk-test PRIVATE -Wall -Werror -Wextra -Wpedantic -Wno-unknown-pragmas)
endif ()

add_test(NAME "All" COMMAND nakama-sdk-test)
