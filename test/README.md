This is a project for running tests on nakama-cpp via CMake. You'll need an architecture-specific `nakama-sdk` release
installed in `out`.

If you are building for Mac/iOS, you'll need to set your NAKAMA_TEST_DEVELOPMENT_TEAM environment variable to the team ID printed in parentheses here: `xcrun security find-identity -v -p codesigning`

In-tree example:
```
cd example
cmake --list-presets
cmake --preset <configure-preset>
cmake --build --preset <build-preset> --target install
```

For Android, inside the `android` folder:

`./gradlew assemble -PandroidABI=<ANDROID_ABI>` where ANDROD_ABI is one of those defined in `CMakePresets.json`.

For example:
`./gradlew assemble -PandroidABI=arm64-v8a`

To run on an Android device, also inside the `android` folder:

(1) `adb install ./build/outputs/apk/debug/nakamatest-debug.apk`
(2) `adb shell`
(3) `am start -a android.intent.action.MAIN -n com.heroiclabs.nakamatest/android.app.NativeActivity`
(4) `logcat | grep "com.heroiclabs.nakamatest"`