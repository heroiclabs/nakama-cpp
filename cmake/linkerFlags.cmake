if(MSVC)
    add_link_options(
            /debug # always create PDB files
            /OPT:REF /OPT:ICF  # restore linked optimizations disabled by /debug
    )
elseif(CMAKE_CXX_COMPILER_ID MATCHES "^Apple")
    add_link_options(-Wl,-dead_strip)
else()
    add_link_options(-Wl,--gc-sections)
endif()
