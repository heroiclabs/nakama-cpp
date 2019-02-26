## Release Instructions

### New releases

Release for Cocos2d-x should be made after the main release (see RELEASEINST.md).

Remove all `ssl` and `crypto` libs from `release/nakama-cpp-sdk/libs` folder.

Copy `NakamaCocos2d` folder from [here](https://github.com/heroiclabs/nakama-cocos2d-x/tree/master/Classes) to `release/nakama-cpp-sdk` folder.

Release folder structure:
```
nakama-cpp-sdk/
├── include/
│   ├── nakama-cpp/
│   └── nonstd/
├── libs/
├── nakama-cpp-android/
├── NakamaCocos2d/
├── CMakeLists.txt
├── LICENSE
└── README.md
```

### Full release workflow

The development team use these steps to build and upload a release.

1. Make archive from `nakama-cpp-sdk` folder and name it `nakama-cocos2d-x-sdk_x.x.x.zip`

2. Login and create a [new draft release](https://github.com/heroiclabs/nakama-cocos2d-x/releases/new) on GitHub.

3. Repeat the changelog in the release description.

4. Attach `nakama-cocos2d-x-sdk_x.x.x.zip`

5. Publish the release.
