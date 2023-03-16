This is a project for running tests on nakama-cpp via CMake. Configuring tests will automatically build (with cache awareness) the nakama-sdk for the target platform as well. Tests will automatically run on desktops after building.

You can set `-DREMOTE_NAKAMA_SDK=ON` when configuring the tests to pull in a version of the nakama-sdk published on vcpkg.

If you are building for Mac/iOS, you'll need to set your NAKAMA_TEST_DEVELOPMENT_TEAM environment variable to your team ID. Your can find your team ID at developer.apple.com.

In-tree example:
```
cd example
cmake --list-presets
cmake --preset <configure-preset>
cmake --build build/<build-preset> --target install run
```

To build and deploy for iOS, you will need to pass `-- -allowProvisioningUpdates` to the end of your cmake build command.

To build for Android, inside the `android` folder:

`./gradlew assemble -Pabi=<ANDROID_ABI> -Phost=<host>` where ANDROID_ABI and HOST is one of those defined in `CMakePresets.json`.

For example:
`./gradlew assemble -Pabi=arm64-v8a -Phost=osx_arm64`

We provide an optional helper for deploying to an Android device: `deploy.sh <device_id>`. The device ID can be obtained through `adb devices`.
