# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT License.

find_package(Git QUIET)
if (NOT Git_FOUND)
    message(STATUS "Unable to find git, which is needed for versioning")
endif ()

function(get_git_dir DIRECTORY OUTPUT_VAR)
    execute_process(
            COMMAND
            ${GIT_EXECUTABLE} rev-parse --git-dir
            WORKING_DIRECTORY
            ${DIRECTORY}
            RESULT_VARIABLE
            GIT_DIR_RESULT
            OUTPUT_VARIABLE
            GIT_DIR_OUTPUT
            ERROR_QUIET
            OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    # Allow to fail
    set(${OUTPUT_VAR} ${GIT_DIR_OUTPUT} PARENT_SCOPE)
endfunction()

function(get_git_current_hash DIRECTORY OUTPUT_VAR)
    execute_process(
            COMMAND
            ${GIT_EXECUTABLE} rev-parse --verify HEAD
            WORKING_DIRECTORY
            ${DIRECTORY}
            RESULT_VARIABLE
            GIT_CURRENT_HASH_RESULT
            OUTPUT_VARIABLE
            GIT_CURRENT_HASH_OUTPUT
            OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    if (NOT ("${GIT_CURRENT_HASH_RESULT}" STREQUAL "0"))
        message(STATUS ${GIT_CURRENT_HASH_OUTPUT})
        message(STATUS "Failed to get ${DIRECTORY} git hash")
    else ()
        set(${OUTPUT_VAR} ${GIT_CURRENT_HASH_OUTPUT} PARENT_SCOPE)
    endif ()
endfunction()

function(get_git_current_date DIRECTORY OUTPUT_VAR)
    execute_process(
            COMMAND
            ${CMAKE_COMMAND} -E env TZ=UTC ${GIT_EXECUTABLE} show -s --format=%cd --date=format-local:%Y%m%dT%H%M%SZ HEAD
            WORKING_DIRECTORY
            ${DIRECTORY}
            RESULT_VARIABLE
            GIT_CURRENT_DATE_RESULT
            OUTPUT_VARIABLE
            GIT_CURRENT_DATE_OUTPUT
            OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    if (NOT ("${GIT_CURRENT_DATE_RESULT}" STREQUAL "0"))
        message(STATUS ${GIT_CURRENT_DATE_OUTPUT})
        message(STATUS "Failed to get ${DIRECTORY} git date")
    else ()
        set(${OUTPUT_VAR} ${GIT_CURRENT_DATE_OUTPUT} PARENT_SCOPE)
    endif ()
endfunction()
