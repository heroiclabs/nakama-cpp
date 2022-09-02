include_guard()

if(NOT real_project_dir)
    set(real_project_dir "${CMAKE_CURRENT_LIST_DIR}/..")
endif()

# try_compile is invoked implicitly by CMake at various places. These calls
# are made in completely independent CMake projects, but with the same toolchain.
# Because projects are independent, they have no access to CACHE variables in the "main"
# project. CMAKE_TRY_COMPILE_PLATFORM_VARIABLES instructs cmake to pass specific vars
# to try_compile projects, making them available to our toolchain.
list(APPEND CMAKE_TRY_COMPILE_PLATFORM_VARIABLES "WITH_LIBCXX" "LIBCXX_STATIC" "LIBCXX_INSTALL_DIR")

include(${real_project_dir}/cmake/compilerFlags.cmake)

if (WITH_LIBCXX)
    if (LIBCXX_STATIC)
        set(LIBCXX_LIBS ${LIBCXX_INSTALL_DIR}/lib/libc++.a)

        # When linking libc++ statically in, we still wa libc++abi (thats where exceptions and new/delete operators)
        # to be NOT linked into our library, becuase we *really* want them to come from the "main" binary. But if
        # we compile binary, we want libc++abi linked-in too, because otherwise binary becomes underlinked and doesn't
        # work. In our case we compile helper binaries like protoc too , so we need to support both.
        set(LIBCXXABI_LIB ${LIBCXX_INSTALL_DIR}/lib/libc++abi.a)
        if (NOT DEFINED CMAKE_SCRIPT_MODE_FILE)
            # CMAKE_CXX_STANDARD_LIBRARIES doesn't support generator expressions so we use link_libraries() instead
            # Magic expression below links libc++abi.a only for binaries
            link_libraries($<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${LIBCXX_INSTALL_DIR}/lib/libc++abi.a>)
        endif()
    else()
        set(LIBCXX_LIBS "${LIBCXX_INSTALL_DIR}/lib/libc++.so.1 ${LIBCXX_INSTALL_DIR}/lib/libc++abi.so.1")
    endif()

    # set flags required to use libc++ using  cmake var rather than "proper" add_compile_options() & co,
    # because not every dependency vcpkg compiles is CMake  based (Boost!!!), because of that cmake flag
    # is more portable

    # GCC
    string(APPEND CMAKE_CXX_FLAGS " -nostdinc++ -nodefaultlibs -isystem ${LIBCXX_INSTALL_DIR}/include/c++/v1")

    # Clang version (untested)
    #string(APPEND CMAKE_CXX_FLAGS " -nostdinc++ -nostdlib++ -isystem ${LIBCXX_INSTALL_DIR}/include/c++/v1")

    set(CMAKE_INSTALL_RPATH ${LIBCXX_INSTALL_DIR}/lib)

    # GCC
    set(CMAKE_CXX_STANDARD_LIBRARIES "${LIBCXX_LIBS} -lpthread -lm -lc -lgcc_s -lgcc")
    # Clang version (untested)
    #set(CMAKE_CXX_STANDARD_LIBRARIES "${LIBCXX_LIB}")

endif()

