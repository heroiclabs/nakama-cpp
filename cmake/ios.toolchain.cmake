set(CMAKE_SYSTEM_NAME Darwin)

# Set developer directory
execute_process(COMMAND /usr/bin/xcode-select -print-path
                OUTPUT_VARIABLE XCODE_DEVELOPER_DIR
                OUTPUT_STRIP_TRAILING_WHITESPACE)

# Locate gcc
execute_process(COMMAND /usr/bin/xcrun -sdk iphoneos -find gcc
                OUTPUT_VARIABLE CMAKE_C_COMPILER
                OUTPUT_STRIP_TRAILING_WHITESPACE)

# Locate g++
execute_process(COMMAND /usr/bin/xcrun -sdk iphoneos -find g++
                OUTPUT_VARIABLE CMAKE_CXX_COMPILER
                OUTPUT_STRIP_TRAILING_WHITESPACE)

# Set the CMAKE_OSX_SYSROOT to the latest SDK found
execute_process(COMMAND /usr/bin/xcrun -sdk iphoneos --show-sdk-path
                OUTPUT_VARIABLE CMAKE_OSX_SYSROOT
                OUTPUT_STRIP_TRAILING_WHITESPACE)

# If the we are on 7.0 or higher SDK we should add version min to get
# things to compile.  This is probably more properly handled with a compiler version
# check but this works for now.
#message("XCODE_DEVELOPER_DIR=${XCODE_DEVELOPER_DIR}")
#message("CMAKE_C_COMPILER=${CMAKE_C_COMPILER}")
#message("CMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}")
#message("CMAKE_OSX_SYSROOT=${CMAKE_OSX_SYSROOT}")

if(CMAKE_OSX_SYSROOT MATCHES iPhone[7-9].[0-9].sdk)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mios-version-min=5.0")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mios-version-min=5.0")
else()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__IPHONE_OS_VERSION_MIN_REQUIRED=50000")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D__PHONE_OS_VERSION_MIN_REQIORED=50000")
endif()

if(NOT CMAKE_OSX_ARCHITECTURES)
    set(CMAKE_OSX_ARCHITECTURES "armv7s")
endif()
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden -fvisibility-inlines-hidden")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fvisibility=hidden -fvisibility-inlines-hidden")

set(CMAKE_OSX_ARCHITECTURES "${CMAKE_OSX_ARCHITECTURES}" CACHE STRING "osx architectures")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}" CACHE STRING "c++ flags")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}" CACHE STRING "c flags")
set(CMAKE_OSX_SYSROOT "${CMAKE_OSX_SYSROOT}" CACHE PATH "osx sysroot")

set(CMAKE_FIND_ROOT_PATH ${CMAKE_OSX_SYSROOT})
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

set(APPLE_IOS ON)
set(TARGET_IPHONE_SIMULATOR OFF)

if (NOT CMAKE_OSX_SYSROOT)
  message(FATAL_ERROR "Could not find a usable iOS SDK in ${sdk_root}")
endif()

message(STATUS "-- gcc found at: ${CMAKE_C_COMPILER}")
message(STATUS "-- g++ found at: ${CMAKE_CXX_COMPILER}")
message(STATUS "-- Using iOS SDK: ${CMAKE_OSX_SYSROOT}")
