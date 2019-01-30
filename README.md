Nakama C++ SDK
=============

> General C++ client for Nakama server 2.3.x.

Nakama is an open-source distributed server for social and realtime games. For more information have a look at the [server documentation](https://heroiclabs.com/docs/).

If you encounter any issues with the server you can generate diagnostics for us with `nakama doctor`. Send these to support@heroiclabs.com or [open an issue](https://github.com/heroiclabs/nakama/issues). If you experience any issues with the client, it can be useful to enable debug logs (`NLogger::setLevel(NLogLevel::Debug);`) and [open an issue](https://github.com/heroiclabs/nakama-cpp/issues).

### Getting Started

To get started using Nakama C++ SDK, choose your engine and go further:

1. [Unreal](https://github.com/heroiclabs/nakama-unreal)
2. [Cocos2d-x](https://github.com/heroiclabs/nakama-cocos2d-x)

#### Supported platforms

Windows, Android, iOS, Mac

### Clonning

To update all submodules:

`git submodule update --init --recursive`

### Building

The build is primarily intended for SDK developers.

#### Build Prerequisites

* git
* python 2.7
* cmake 3.10+
* go
* perl
* Visual Studio 2015 or 2017 - for Windows only
* boost - for Windows and Mac, used by websocketpp library

To build Nakama C++ SDK:

#### Building for Windows

```
cd build\win32
build_win32.py
```

#### Building for Android

```
cd build\android
build_android.py <ABI>
```
Where <ABI> is Android ABI e.g. armeabi-v7a, arm64-v8a or x86

#### Tests

Tests are built when you build Nakama C++ SDK for desktop OS (Windows or Mac).
By default tests try to connect to local server by `127.0.0.1`
To use another IP of your server, edit `test/test_server_config.h` file.

Run tests (console application):

```
build/{platform}/build/test/Debug/nakama-test
```
