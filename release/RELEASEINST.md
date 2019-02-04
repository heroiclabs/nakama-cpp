## Release Instructions

### New releases

Extract `release/libs.7z` to `release/nakama-cpp-sdk`.

Copy `include` to `release/nakama-cpp-sdk`.

Copy `third_party/nonstd` to `release/nakama-cpp-sdk/include`.

Release folder structure:
```
nakama-cpp-sdk/
├── include/
│   ├── nakama-cpp
│   └── nonstd
├── libs/
├── nakama-cpp-android/
└── CMakeLists.txt
```

### Full release workflow

The development team use these steps to build and upload a release.

1. Update the `CHANGELOG.md`.

   Make sure to add the relevant "Added", "Changed", "Deprecated", "Removed", "Fixed", and "Security" sections as suggested by [keep a changelog](http://keepachangelog.com).

2. Update version in `src/Nakama.cpp`, build and commit.

   ```
   git add Nakama.cpp CHANGELOG.md
   git commit -m "Nakama C++ 2.0.0 release."
   ```

3. Tag the release.

   __Note__ In source control good semver suggests a `"v"` prefix on a version. It helps group release tags.

   ```
   git tag -a v2.0.0 -m "v2.0.0"
   git push origin v2.0.0
   ```

4. Login and create a [new draft release](https://github.com/heroiclabs/nakama-cpp/releases/new) on GitHub. Repeat the changelog in the release description. Then publish the release.
