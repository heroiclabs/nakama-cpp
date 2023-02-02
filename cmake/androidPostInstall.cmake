# move to our gradle tree
set(GRADLE_JNI ${GRADLE_ROOT}/jniLibs/${ANDROID_ABI})
set(GRADLE_HEADERS ${GRADLE_ROOT}/headers)

message("COPYING ${CMAKE_INSTALL_LIBDIR} to ${GRADLE_JNI}")

file(COPY ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR} DESTINATION ${GRADLE_JNI})

file(COPY ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_INCLUDEDIR} DESTINATION ${GRADLE_HEADERS})

# build in gradle
execute_process(COMMAND "./gradlew assemble -Pabi=${ANDROID_ABI}" WORKING_DIRECTORY ${GRADLE_ROOT})

# put resulting .aar back in our ./out directory