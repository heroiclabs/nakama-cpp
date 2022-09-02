# This file is meant to be included as ${CMAKE_USER_MAKE_RULES_OVERRIDE}
# set by toolchain file. It's purpose is to override flags inferred
# by the CMake on the Windows platform so that it works with XDK builds.


# XDK has own list of libraries always included in the link phase
# Here is a list I came up with by intersecting list of .lib files in
# Durango SDK files and a list of libs normally set by Windows-MSVC-CXX
set(CMAKE_C_STANDARD_LIBRARIES_INIT "kernelx.lib uuid.lib")

# Ditto for CXX
set(CMAKE_CXX_STANDARD_LIBRARIES_INIT "combase.lib kernelx.lib uuid.lib")


# we use CMAKE_SYSTEM_NAME WindowsStore and CMake, which makes CMake to add some defines implicitly.
# Delete those which are not set by XDK MSBuild platform configs
string(REGEX REPLACE "/DWIN32|/D_WINDOWS" "" CMAKE_CXX_FLAGS_INIT "${CMAKE_CXX_FLAGS_INIT}")
string(REGEX REPLACE "/DWIN32|/D_WINDOWS" "" CMAKE_C_FLAGS_INIT "${CMAKE_C_FLAGS_INIT}")

string(REPLACE "/Ob1" "/Ob2" CMAKE_CXX_FLAGS_MINSIZEREL_INIT "${CMAKE_CXX_FLAGS_MINSIZEREL_INIT}")
string(REPLACE "/Ob1" "/Ob2" CMAKE_C_FLAGS_MINSIZEREL_INIT "${CMAKE_C_FLAGS_MINSIZEREL_INIT}")
