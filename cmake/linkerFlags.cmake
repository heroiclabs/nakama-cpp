if(MSVC)
    add_link_options(
            /debug # always create PDB files
            /OPT:REF /OPT:ICF  # restore linked optimizations disabled by /debug
    )
elseif(CMAKE_CXX_COMPILER_ID MATCHES "^Apple")
    add_link_options(-Wl,-dead_strip)
elseif(CMAKE_SYSTEM_NAME STREQUAL "ORBIS")
    add_link_options(-Wl,--strip-unused,--strip-unused-data,--strip-duplicates,--strip-report=stripreport.txt)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Prospero")
    add_link_options(-Wl,--as-needed,--icf=all,--gc-sections)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")  # TODO: change based on linker used, not compiler
    add_link_options(
        -Wl,--gc-sections
    )
endif()
