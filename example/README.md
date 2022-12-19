This is an example project that integrates nakama-cpp via CMake. You'll need an architecture-specific `nakama-sdk` release
installed in `out`.

If you are building for Mac/iOS, you'll need to set your NAKAMA_TEST_DEVELOPMENT_TEAM environment variable to the team ID printed in parentheses here: `xcrun security find-identity -v -p codesigning`

In-tree example:
```
cd example
cmake --list-presets
cmake --preset <configure-preset>
cmake --build --preset <build-preset> --target install
```