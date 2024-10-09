# vcpkg doesn't allow us to select MinSizeRel, only Release and Debug are available, so we
# set -Os flag in release builds to get smaller binaries
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(CMAKE_CXX_FLAGS_RELEASE "-Os -DNDEBUG")
endif()
if (CMAKE_C_COMPILER_ID STREQUAL "GNU" OR CMAKE_C_COMPILER_ID STREQUAL "Clang")
    set(CMAKE_C_FLAGS_RELEASE "-Os -DNDEBUG")
endif()

# These embed pdb data into object files, speeds up compilation by avoiding filesystem locking on shared pdb file
string(REPLACE "/Zi" "/Z7" CMAKE_C_FLAGS_DEBUG_INIT "${CMAKE_C_FLAGS_DEBUG_INIT}")
string(REPLACE "/Zi" "/Z7" CMAKE_CXX_FLAGS_DEBUG_INIT "${CMAKE_CXX_FLAGS_DEBUG_INIT}")

# Editing non _INIT flags has effect when this file is included from CMakeLists.txt directly
string(REPLACE "/Ob1" "/Ob2" CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL}")
string(REPLACE "/Ob1" "/Ob2" CMAKE_C_FLAGS_MINSIZEREL "${CMAKE_C_FLAGS_MINSIZEREL}")



# Minimize what's visible outside of shared lib
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_C_VISIBILITY_PRESET hidden)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)

if (NOT CMAKE_BUILD_TYPE STREQUAL Debug)
	set(CMAKE_INTERPROCEDURAL_OPTIMIZATION OFF)
endif()

if (NOT MSVC)
    string(APPEND CMAKE_C_FLAGS " -ffunction-sections -fdata-sections -g")
    string(APPEND CMAKE_CXX_FLAGS " -ffunction-sections -fdata-sections -g")
else()
    string(APPEND CMAKE_C_FLAGS " /Gw /Gy")
    string(APPEND CMAKE_CXX_FLAGS " /Gw /Gy")
endif()
