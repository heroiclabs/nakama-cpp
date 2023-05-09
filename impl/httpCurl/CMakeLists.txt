file(
        GLOB_RECURSE srcs
        LIST_DIRECTORIES false
        "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_library(nakama-impl-http-libcurl OBJECT ${srcs})

find_package(CURL CONFIG REQUIRED)

target_link_libraries(nakama-impl-http-libcurl
    PUBLIC nakama-api-proto nakama::sdk-interface)


function(merge_transitive_dependencies target)
  set(visited_targets "")

  function(merge_dependencies current_target)
    if(NOT ${current_target} IN_LIST visited_targets)
      list(APPEND visited_targets ${current_target})

      get_property(current_dependencies TARGET ${current_target} PROPERTY LINK_LIBRARIES)
      foreach(dep IN LISTS current_dependencies)
        if(TARGET ${dep})
          merge_dependencies(${dep})
          if(NOT ${dep} IN_LIST visited_targets)
            message(STATUS "Merging objects of '${dep}' into '${target}'")
            target_sources(${target} PRIVATE $<TARGET_OBJECTS:${dep}>)
          endif()
        endif()
      endforeach()
    endif()
  endfunction()

  merge_dependencies(${target})
endfunction()

## TODO also add shared vs. static detection for wslay curl io?
if (BUILD_SHARED_LIBS)
    target_link_libraries(nakama-impl-http-libcurl
        PUBLIC CURL::libcurl
    )
else()
    merge_transitive_dependencies(CURL::libcurl)
    list(APPEND NAKAMA_SDK_DEPS $<TARGET_OBJECTS:CURL::libcurl>)
    set(NAKAMA_SDK_DEPS ${NAKAMA_SDK_DEPS} PARENT_SCOPE)
    target_include_directories(nakama-impl-http-libcurl
        PRIVATE
        $<TARGET_PROPERTY:CURL::libcurl,INTERFACE_INCLUDE_DIRECTORIES>
    )
endif()


set(HTTP_IMPL_LIB nakama-impl-http-libcurl PARENT_SCOPE)
