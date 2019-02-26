## Release Instructions

### New releases

Download `libs_x.x.x.7z` from [libs](https://drive.google.com/drive/folders/1KHnSy28Og8uzanMPNRYSKlI0mbRUBFhH)

Extract `libs_x.x.x.7z` to `release/nakama-cpp-sdk`.

Copy `include` to `release/nakama-cpp-sdk`.

Copy `third_party/nonstd` to `release/nakama-cpp-sdk/include`.

Copy `LICENSE` to `release/nakama-cpp-sdk/LICENSE`.

Copy `README.md` to `release/nakama-cpp-sdk/README.md`.

Release folder structure:
```
nakama-cpp-sdk/
├── include/
│   ├── nakama-cpp/
│   └── nonstd/
├── libs/
├── nakama-cpp-android/
├── CMakeLists.txt
├── LICENSE
└── README.md
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

4. Make archive from `nakama-cpp-sdk` folder and name it `nakama-cpp-sdk_x.x.x.zip`

5. Do `RELEASEINST_Cocos2d-x.md`

6. Login and create a [new draft release](https://github.com/heroiclabs/nakama-cpp/releases/new) on GitHub.

7. Repeat the changelog in the release description.

8. Attach `nakama-cpp-sdk_x.x.x.zip` and `nakama-cocos2d-x-sdk_x.x.x.zip`

9. Publish the release.
