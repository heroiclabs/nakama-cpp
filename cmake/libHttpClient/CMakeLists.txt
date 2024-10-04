# Upstream libHttpClient stopped offering CMake based builds
# This file recreates same experience by (pooorly) reading upstream .vcxproj
set(LHC_ROOT "${CMAKE_SOURCE_DIR}/submodules/libHttpClient")

include(CMakePrintHelpers)

# Reads ClInclude from $1 and returns list of absolute paths in $2
function(vcxproj_read_file_list)
    cmake_path(GET ARGV0 PARENT_PATH BASEDIR)
    file(READ "${ARGV0}" RAW)
    STRING(REGEX MATCHALL [=[<(ClInclude|ClCompile) Include="[^"]+"]=] CLINCLUDES_RAW "${RAW}")
    list(TRANSFORM CLINCLUDES_RAW REPLACE [=[<(ClInclude|ClCompile) Include="\$\(MSBuildThisFileDirectory\)([^"]+)"]=] "\\2" OUTPUT_VARIABLE CLINCLUDES)
    foreach (P ${CLINCLUDES})
        cmake_path(ABSOLUTE_PATH P BASE_DIRECTORY "${BASEDIR}" NORMALIZE)
        list(APPEND OUT "${P}")
    endforeach ()
    set(${ARGV1} ${OUT} PARENT_SCOPE)
endfunction()

vcxproj_read_file_list("${LHC_ROOT}/Build/libHttpClient.Common/libHttpClient.Common.vcxitems" COMMON_FILES)
vcxproj_read_file_list("${LHC_ROOT}/Build/libHttpClient.XAsync/libHttpClient.XAsync.vcxitems" XASYNC_FILES)
vcxproj_read_file_list("${LHC_ROOT}/Build/libHttpClient.Win32.Shared/libHttpClient.Win32.Shared.vcxitems" WIN32_SHARED_FILES)

if (GDK)
elseif (WIN32)
    set(FILES ${COMMON_FILES} ${XASYNC_FILES} ${WIN32_SHARED_FILES})
else()
    message(FATAL_ERROR "libHTTPClient is supported only on Windows-based platforms")
endif()

find_package(ZLIB REQUIRED)

add_library(libHttpClient OBJECT ${FILES})
target_include_directories(libHttpClient
        PUBLIC
            ${LHC_ROOT}/Include
        PRIVATE
            ${LHC_ROOT}/Source
            ${LHC_ROOT}/Source/Common
)
if (NOT WITH_WS_LIBHTTPC)
    target_compile_definitions(libHttpClient PUBLIC "HC_NOWEBSOCKETS")
endif()
target_link_libraries(libHttpClient PRIVATE
        ZLIB::ZLIB
        "Appnotify.lib"
        "winhttp.lib"
        "crypt32.lib"
)

#set_target_properties(libHttpClient PROPERTIES LINKER_LANGUAGE CXX)

