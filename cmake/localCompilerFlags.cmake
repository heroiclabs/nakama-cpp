# compiler flags set for SDK only.
# Use for flags you can't/don't want to set on for vcpkg packages, like error on warnings for instance

if (MSVC)
    target_compile_options(
            nakama-sdk
            PRIVATE
                /W4 /WX  # error on warnings
                /Z7      # generate debug info so that PDB contains line numbers even in Release builds
                /utf-8   # our source code is using utf-8
    )
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    target_compile_options(
            nakama-sdk
            PRIVATE
                -Wall -Wextra -Werror  # error on warnings
                -Wno-unknown-pragmas   # ignore pragmas from other compilers
    )
endif()