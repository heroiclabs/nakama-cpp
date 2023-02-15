This is a project for running tests on nakama-cpp via CMake. You'll need an architecture-specific `nakama-sdk` release
installed in `out`. Make sure it was built with `--config MinSizeRel`.

If you are building for Mac/iOS, you'll need to set your NAKAMA_TEST_DEVELOPMENT_TEAM environment variable to your team ID. Your can find your team ID at developer.apple.com.

In-tree example:
```
cd example
cmake --list-presets
cmake --preset <configure-preset>
cmake --build build/<build-preset> --target install
```

To build and deploy for iOS, you will need to pass `-- -allowProvisioningUpdates` to the end of your cmake build command.


To build for Android, inside the `android` folder:

`./gradlew assemble -Pabi=<ANDROID_ABI> -Phost=<host>` where ANDROID_ABI and HOST is one of those defined in `CMakePresets.json`.

For example:
`./gradlew assemble -Pabi=arm64-v8a -Phost=osx_arm64`

We provide an optional helper for deploying to an Android device: `deploy.sh <device_id>`. The device ID can be obtained through `adb devices`.
