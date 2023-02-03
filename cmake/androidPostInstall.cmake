# move to our gradle tree

set(GRADLE_MAIN ${GRADLE_ROOT}/app/src/main)
set(GRADLE_JNI ${GRADLE_MAIN}/libs/${ANDROID_ABI})
set(GRADLE_HEADERS ${GRADLE_MAIN}/headers)

message("-- Cleaning: ${GRADLE_JNI}")
message("-- Cleaning: ${GRADLE_HEADERS}")

file(REMOVE_RECURSE ${GRADLE_JNI})
file(REMOVE_RECURSE ${GRADLE_HEADERS})

message("-- Copying installed library to: ${GRADLE_JNI}")
message("-- Copying installed headers to: ${GRADLE_HEADERS}")

file(COPY ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}/libnakama-sdk.so DESTINATION ${GRADLE_JNI})
file(COPY ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_INCLUDEDIR}/nakama-cpp DESTINATION ${GRADLE_HEADERS})

message("-- Bundling Java with C++ code in Gradle inside: ${GRADLE_ROOT}")
message("-- Android ABI is ${ANDROID_ABI}")

execute_process(COMMAND ./gradlew assemble -Pabi=${ANDROID_ABI}
    WORKING_DIRECTORY ${GRADLE_ROOT}
    OUTPUT_VARIABLE GRADLE_OUTPUT
    ERROR_VARIABLE GRADLE_ERROR
    COMMAND_ERROR_IS_FATAL ANY
    COMMAND_ECHO STDOUT
)

message("-- Gradle Output: ${GRADLE_OUTPUT}")
message("-- Moving artifact back to CMake installation directory.")

# put resulting .aar back in our ./out directory