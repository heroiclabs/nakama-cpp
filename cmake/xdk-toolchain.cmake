cmake_minimum_required(VERSION 3.23)
include_guard()

# XDK (Durango) is closest to WindowsStore because of WinRT use
set( CMAKE_SYSTEM_NAME WindowsStore )
set( CMAKE_USER_MAKE_RULES_OVERRIDE ${CMAKE_CURRENT_LIST_DIR}/xdk-rules-override.cmake)
set (XDK ON)

# Find the latest XDK version.
set( ENV_XDK_LATEST $ENV{XboxOneXDKLatest} )
if(NOT ENV_XDK_LATEST)
    message(FATAL_ERROR "XboxOneXDKLatest env var is not found. Have you installed XDK?")
endif()

get_filename_component( XDK_LATEST_PATH "${ENV_XDK_LATEST}dummy" PATH )
get_filename_component( XDK_LATEST_VER "${XDK_LATEST_PATH}" NAME )

get_filename_component( XDK_EXTENSIONS_PATH "$ENV{XboxOneExtensionSDKLatest}dummy" PATH )

set( CMAKE_SYSTEM_VERSION ${XDK_LATEST_VER} ) ## Durango SDK is actually a Windows Kit with weird version
set( CMAKE_SYSTEM_PROCESSOR x64 )
set( CMAKE_VS_PLATFORM_TOOLSET_HOST_ARCHITECTURE x64 )

if (CMAKE_GENERATOR MATCHES ^Visual)
    set( CMAKE_GENERATOR_PLATFORM Durango)
    set( CMAKE_GENERATOR_TOOLSET  v141)
    message(FATAL_ERROR "VS support is incomplete. Use Ninja")
endif()

include(${CMAKE_CURRENT_LIST_DIR}/VSWhere.cmake)

# VS2017 only as it is latest officially supported VS for XDK (Durango)
# but we allow any Visual studio, because we don't depend XDK's MSbuild platform configs
findVisualStudio(
        VERSION "[15,)"
        PRERELEASE OFF
        PRODUCTS *
        PROPERTIES
        installationPath VS_INSTALLATION_PATH
        installationVersion VS_VERSION
)
if(NOT VS_INSTALLATION_PATH)
    message(FATAL_ERROR "Unable to find Visual Studio 15 2017" )
endif()
cmake_path(NORMAL_PATH VS_INSTALLATION_PATH)

message(STATUS "VS_INSTALLATION_PATH = ${VS_INSTALLATION_PATH}")
include(${CMAKE_CURRENT_LIST_DIR}/Windows.Kits.cmake)

set(VS_MSVC_PATH "${VS_INSTALLATION_PATH}/VC/Tools/MSVC")

if(NOT VS_PLATFORM_TOOLSET_VERSION)
    file(GLOB VS_TOOLSET_VERSIONS RELATIVE ${VS_MSVC_PATH} ${VS_MSVC_PATH}/*)
    list(SORT VS_TOOLSET_VERSIONS COMPARE NATURAL ORDER DESCENDING)
    list(POP_FRONT VS_TOOLSET_VERSIONS VS_TOOLSET_VERSION)
endif()

set(VS_TOOLSET_PATH "${VS_INSTALLATION_PATH}/VC/Tools/MSVC/${VS_TOOLSET_VERSION}")
set(CMAKE_VS_PLATFORM_TOOLSET_ARCHITECTURE ${CMAKE_SYSTEM_PROCESSOR})

set(VS_TOOLSET_ARCH_PATH ${VS_TOOLSET_PATH}/bin/Host${CMAKE_VS_PLATFORM_TOOLSET_HOST_ARCHITECTURE}/${CMAKE_VS_PLATFORM_TOOLSET_ARCHITECTURE})

set(CMAKE_CXX_COMPILER "${VS_TOOLSET_ARCH_PATH}/cl.exe")
set(CMAKE_CXX_COMPILER_ID MSVC)
set(CMAKE_C_COMPILER "${VS_TOOLSET_ARCH_PATH}/cl.exe")
set(CMAKE_C_COMPILER_ID MSVC)

getMsvcVersion(${CMAKE_CXX_COMPILER} MSVC_VERSION)
if(NOT MSVC_VERSION)
    message(FATAL_ERROR "Unable to obtain the compiler version from: ${CMAKE_CXX_COMPILER}")
endif()

# Only search the XBoxOne XDK, not the remainder of the host file system
set( CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER )
set( CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY )
set( CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY )
set( CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY )

# Durango platform props files contain defines and compiler flags.
# These are picked up by VS generator automagically, but for Ninja we
# need to replicate all of them explicitly.

set(XDK_COMPILE_DEFS
        " -D_TITLE -DMONOLITHIC=1 -DWINAPI_FAMILY=WINAPI_FAMILY_TV_TITLE"

        # 180716\xdk\VS2017\flatDeployment\Common7\IDE\VC\VCTargets\Platforms\Durango\180716\Platform.Common.props
        " -D_DURANGO -D_XBOX_ONE -DWIN32_LEAN_AND_MEAN"

        # These are not straightforoward defines, but wrapped in some conditional logic which I don't understand,
        # but they ended up passed to CL.exe, hence I am adding them here
        # 180716\xdk\VS2017\flatDeployment\Common7\IDE\VC\VCTargets\Platforms\Durango\180716\Platform.Edition.Targets
        " -D_CRT_USE_WINAPI_PARTITION_APP -D_UITHREADCTXT_SUPPORT=0"

        " -D_UNICODE -DUNICODE"

        # Set in all XDK template projects
        " -D__WRL_NO_DEFAULT_LIB__"
)

# use FLAGS_INIT var rather that add_compile_definitions, because
# former is much sooner sooner, even at compiler detection phase
string(APPEND CMAKE_CXX_FLAGS_INIT ${XDK_COMPILE_DEFS})
string(APPEND CMAKE_C_FLAGS_INIT ${XDK_COMPILE_DEFS})


## If PDB debug symbols enables (/Zi) change them to be embedded into .obj files
# DOESNT WORK: https://gitlab.kitware.com/cmake/cmake/-/issues/23371
#string(REPLACE "/Zi" "/Z7" CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}")
#string(REPLACE "/Zi" "/Z7" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")

# enable WinRT (requires disablinig minimal rebuild), also see https://docs.microsoft.com/en-us/cpp/cppcx/compiler-and-linker-options-c-cx?view=msvc-170
string(APPEND CMAKE_CXX_FLAGS_INIT " /ZW /Gm- /EHsc" )

# 180716\xdk\VS2017\flatDeployment\Common7\IDE\VC\VCTargets\Platforms\Durango\180716\Platform.Common.props
string(APPEND CMAKE_CXX_FLAGS_INIT " /arch:AVX /fp:fast /favor:AMD64 /Gy" )
string(APPEND CMAKE_C_FLAGS_INIT " /arch:AVX /fp:fast /favor:AMD64 /Gy" )

# These paths contain string which makes escaping CMAKE_CXX_FLAGS string a pain,
# add_compile_options preserve spaces, but can't distinguish between
# languages, so here we use generator expression to enable flags only for C++
add_compile_options(
        # search for .winmd files here
        $<$<COMPILE_LANGUAGE:CXX>:/AI${XDK_LATEST_PATH}/xdk/VS2017/vc/platform/amd64>
        # Location of Windows.winmd
        $<$<COMPILE_LANGUAGE:CXX>:/AI${XDK_EXTENSIONS_PATH}/references/commonconfiguration/neutral/>
)

set(XDK_INCLUDES
        ${XDK_LATEST_PATH}/xdk/include/um
        ${XDK_LATEST_PATH}/xdk/include/shared
        ${XDK_LATEST_PATH}/xdk/include/winrt
        ${XDK_LATEST_PATH}/xdk/include/cppwinrt
        ${XDK_LATEST_PATH}/xdk/ucrt/inc
        # we move VC2017/vc/include to _STANDARD_INCLUDE_DIRECTORIES for CXX only
        # leave it here for C
        $<$<COMPILE_LANGUAGE:C>:${XDK_LATEST_PATH}/xdk/VS2017/vc/include>
        ${XDK_LATEST_PATH}/xdk/VS2017/vc/platform/amd64
)

include_directories(SYSTEM ${XDK_INCLUDES})

# /ZW flags makes MSVC to implicitly include vccorlib.h, therefore
# this include path must be present whenever /ZW flag is used.
# CMAKE_${LANG}_STANDARD_INCLUDE_DIRECTORIES is a CMake mechanism
# to ensure it is always passed, even in `try_compile`
list(APPEND CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES ${XDK_LATEST_PATH}/xdk/VS2017/vc/include)

set(XDK_LIBDIR
        ${XDK_LATEST_PATH}/xdk/Lib/amd64
        ${XDK_LATEST_PATH}/xdk/ucrt/lib/amd64
        ${XDK_LATEST_PATH}/xdk/VS2017/vc/lib/amd64
        ${XDK_LATEST_PATH}/xdk/VS2017/vc/platform/amd64
)

link_directories(${XDK_LIBDIR})

#180716\xdk\VS2017\flatDeployment\Common7\IDE\VC\VCTargets\Platforms\Durango\180716\Platform.Common.props
set(_IGNORE_DEFAULT_LIBS
    advapi32.lib atl.lib atls.lib atlsd.lib atlsn.lib atlsnd.lib comctl32.lib comsupp.lib dbghelp.lib gdi32.lib
    gdiplus.lib guardcfw.lib kernel32.lib mmc.lib msimg32.lib msvcole.lib msvcoled.lib mswsock.lib ntstrsafe.lib
    ole2.lib ole2autd.lib ole2auto.lib ole2d.lib ole2ui.lib ole2uid.lib ole32.lib oleacc.lib oleaut32.lib oledlg.lib
    oledlgd.lib oldnames.lib runtimeobject.lib shell32.lib shlwapi.lib strsafe.lib urlmon.lib user32.lib userenv.lib
    uuid.lib wlmole.lib wlmoled.lib ws2_32.lib
)
list(TRANSFORM _IGNORE_DEFAULT_LIBS PREPEND "/NODEFAULTLIB:")
add_link_options(${_IGNORE_DEFAULT_LIBS})
unset(_IGNORE_DEFAULT_LIBS)

# Newer MSVC enable new type of exceptions, which is not compatible with XDK. Lets disable them
# - https://devblogs.microsoft.com/cppblog/making-cpp-exception-handling-smaller-x64/
# - https://www.mbs-plugins.de/archive/2020-08-03/Moving_to_Visual_Studio_2019/monkeybreadsoftware_blog_xojo
if(MSVC_VERSION VERSION_GREATER 19.23)
    add_compile_options(/d2FH4-)
    add_link_options(-d2:-FH4-)
endif()
