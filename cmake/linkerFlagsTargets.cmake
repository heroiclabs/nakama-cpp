if(MSVC)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "^Apple")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")  # TODO: change based on linker used, not compiler
    # don't reexport global symbols from static libs we are linking to.
    # technically there should be none due to -fvsibility=hidden, but sometimes they creep up
    # like when we link libc++ statically or openssl
    # NOTE: target specifically nakama-sdk (our library), because we DONT want it to be set on
    #       nakama-test (test executable) in some cases (namely when linking libc++ statically)
    target_link_options(nakama-sdk PRIVATE -Wl,--exclude-libs,ALL)
endif()
