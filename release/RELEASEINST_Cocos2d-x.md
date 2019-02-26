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

Make archive from `nakama-cpp-sdk` folder and name it `nakama-cocos2d-x-sdk_x.x.x.zip`
