This is a project for running tests on nakama-cpp via CMake. You'll need an architecture-specific `nakama-sdk` release
installed in `out`.

If you are building for Mac/iOS, you'll need to set your NAKAMA_TEST_DEVELOPMENT_TEAM environment variable to your team ID. Your can find your team ID at developer.apple.com.

In-tree example:
```
cd example
cmake --list-presets
cmake --preset <configure-preset>
cmake --build --preset <build-preset> --target install
```

To build for Android, inside the `android` folder:

`./gradlew assemble -PandroidABI=<ANDROID_ABI>` where ANDROD_ABI is one of those defined in `CMakePresets.json`.

For example:
`./gradlew assemble -PandroidABI=arm64-v8a`

To deploy to an Android device, also inside the `android` folder, run `deploy.sh`.
