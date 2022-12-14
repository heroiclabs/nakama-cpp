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

For Android:

`./gradlew assemble -PandroidABI=<ANDROID_ABI>` where ANDROD_ABI is one of those defined in `CMakePreset.json`.

For example:

`./gradlew assemble -PandroidABI=armeabi-v7a`
