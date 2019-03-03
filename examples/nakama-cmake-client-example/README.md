Nakama C++ Client Example
=============

> Nakama C++ Client Example using CMake build system

[Nakama](https://github.com/heroiclabs/nakama) is an open-source server designed to power modern games and apps. Features include user accounts, chat, social, matchmaker, realtime multiplayer, and much [more](https://heroiclabs.com).

This example uses [Nakama C++ Client SDK](https://github.com/heroiclabs/nakama-cpp).

Full documentation is online - https://heroiclabs.com/docs

## Getting Started

You'll need to setup the server and database before you can connect with the client. The simplest way is to use Docker but have a look at the [server documentation](https://github.com/heroiclabs/nakama#getting-started) for other options.

Install and run the servers. Follow these [instructions](https://heroiclabs.com/docs/install-docker-quickstart).

## Contribute

The development roadmap is managed as GitHub issues and pull requests are welcome. If you're interested to enhance the code please open an issue to discuss the changes or drop in and discuss it in the [community chat](https://gitter.im/heroiclabs/nakama).

## Build Prerequisites

- python 2.7
- cmake 3.10+
- Visual Studio 2015 or 2017 - for Windows only

### Building for Windows

```bash
cd build\windows
python build_windows.py -m Mode -a Arch
```
Where `Mode` is build mode: `Debug` or `Release`

Where `Arch` is architecture: `x86` or `x64`

### Building for Mac

Prerequisites:
```bash
sudo xcode-select --install
brew install autoconf automake libtool shtool
```

Build:

```bash
cd build\mac
python build_mac.py
```
It builds in `Release` mode.

### Building for iOS

```bash
cd build\ios
python build_ios.py Arch
```
Where `Arch` is architecture: `arm64`, `armv7`, `armv7s` or `x86_64`.

It builds in `Release` mode.

### Building for Linux

To build for x86 architecture use x86 linux distro (we use Ubuntu 14.04.5 i386)

To build for x64 architecture use x64 linux distro (we use Ubuntu 14.04.5 amd64)

```bash
cd build\linux
python build_linux.py
```
It builds in `Release` mode and copies nakama lib to release folder.

### Building for Android

Set `ANDROID_NDK` or `NDK_ROOT` system variable to Android NDK folder.

To build for one ABI:

```bash
cd build/android
python build_android.py ABI
```
Where `ABI` is Android ABI e.g. `armeabi-v7a`, `arm64-v8a`, `x86` or `x86_64`

It builds for Andoid API level 16 in `Release` mode.

## License

This project is licensed under the [Apache-2 License](https://github.com/heroiclabs/nakama-dotnet/blob/master/LICENSE).

## Special Thanks

Thanks to [@dimon4eg](https://github.com/dimon4eg) for this excellent support on developing Nakama C++, Cocos2d-x and Unreal client libraries.

Thanks to [@thewtex](https://github.com/thewtex) for his excellent work on [iOS CMake Toolchain](https://github.com/thewtex/cmake-ios-toolchain) project.
