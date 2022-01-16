## Release Instructions

### Full release workflow

The development team use these steps to build and upload a release.

1. Update the `CHANGELOG.md`.

   Make sure to add the relevant "Added", "Changed", "Deprecated", "Removed", "Fixed", and "Security" sections as suggested by [keep a changelog](http://keepachangelog.com).
   
   Copy `CHANGELOG.md` to `release/nakama-cpp-sdk/CHANGELOG.md`.

2. Update version in:

* `src/Nakama.cpp`
* `docs/Doxyfile`

3. Build and commit.

   ```
   git add Nakama.cpp CHANGELOG.md Doxyfile
   git commit -m "x.x.x release."
   ```

4. Update docs

   ```
   cd docs/
   doxygen
   ```

   Commit updated docs.

5. Tag the release.

   __Note__ In source control good semver suggests a `"v"` prefix on a version. It helps group release tags.

   ```
   git tag -a v2.0.0 -m "v2.0.0"
   git push origin v2.0.0
   ```

6. Copy libs.

   Copy built static libs of all platfroms to `release/libs`.

   Copy built shared libs of all platfroms to `release/shared-libs`.

7. Make release archives.

   ```
   cd release/
   ./release.py
   ```

   Release folder structure:

   ```
   nakama-cpp-sdk/
   ├── include/
   │   ├── nakama-c/
   │   ├── nakama-cpp/
   │   ├── nakama-cpp-c-wrapper/
   │   └── nonstd/
   ├── libs/
   ├── shared-libs/
   ├── nakama-cpp-android/
   ├── CMakeLists.txt
   ├── LICENSE
   ├── CHANGELOG.md
   └── README.md
   ```

8. Login and create a [new draft release](https://github.com/heroiclabs/nakama-cpp/releases/new) on GitHub.

9. Repeat the changelog in the release description.

10. Attach release archives produced by `release.py` script.

11. Publish the release 🎉
