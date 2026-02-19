Nakama C/C++ Client SDK
=============

[![GitHub release](https://img.shields.io/github/release/heroiclabs/nakama-cpp.svg)](https://github.com/heroiclabs/nakama-cpp/releases/latest)
[![Forum](https://img.shields.io/badge/forum-online-success.svg)](https://forum.heroiclabs.com)
[![Client guide](https://img.shields.io/badge/client_guide-online-brightgreen)](https://heroiclabs.com/docs/cpp-client-guide)
[![Reference](https://img.shields.io/badge/reference-online-brightgreen)](https://heroiclabs.github.io/nakama-cpp/html/index.html)
[![Github all releases](https://img.shields.io/github/downloads/heroiclabs/nakama-cpp/total.svg)](https://github.com/heroiclabs/nakama-cpp/releases/)
[![License](https://img.shields.io/github/license/heroiclabs/nakama.svg)](https://github.com/heroiclabs/nakama/blob/master/LICENSE)

[![Build & tests on Ubuntu 18.04](https://github.com/heroiclabs/nakama-cpp/workflows/Ubuntu%2018.04/badge.svg)](https://github.com/heroiclabs/nakama-cpp/actions?query=workflow%3A%22Ubuntu+18.04%22)

> General C/C++ client for Nakama server.

[Nakama](https://github.com/heroiclabs/nakama) is an open-source server designed to power modern games and apps. Features include user accounts, chat, social, matchmaker, realtime multiplayer, and much [more](https://heroiclabs.com).

This client implements the full API and socket options with the server. It's written in C and C++11 with minimal dependencies to support Cocos2d-x, Unreal and other custom engines and frameworks.

If you experience any issues with the client, it can be useful to enable debug logs (see [Logging](#logging) section) and [open an issue](https://github.com/heroiclabs/nakama-cpp/issues).

Full documentation is online - https://heroiclabs.com/docs

## Getting Started

You'll need to setup the server and database before you can connect with the client. The simplest way is to use Docker but have a look at the [server documentation](https://github.com/heroiclabs/nakama#getting-started) for other options.

1. Install and run the servers. Follow these [instructions](https://heroiclabs.com/docs/install-docker-quickstart).

2. Nakama C/C++ SDK is released with prebuilt libraries for following platforms and architectures:

- Windows - 7+, Visual Studio 2015, 2017, 2019 (x86, x64, Debug, Release)
- Android - Android 4.1 (armeabi-v7a, arm64-v8a, x86, x86_64)
- Linux - Ubuntu 18.04 x64
- Mac - 10.10+
- iOS - 5.0+ (arm64, armv7, armv7s, x86_64), Bitcode is off

In theory any platform that meets the requirement for `cpprest` and `boost` is also supported. The client is compiled with C++11.

3. Download the client from the [releases page](https://github.com/heroiclabs/nakama-cpp/releases). You can also [build from source](#source-builds).

<!-- You can download clients that are tailored for [Cocos2d-x](https://github.com/heroiclabs/nakama-cocos2d-x) or [Unreal 4](https://github.com/heroiclabs/nakama-unreal). -->

4. Integrate the client library into your project:

When you've downloaded the Nakama C++ archive and extracted it to `NAKAMA_CPP_SDK` folder, you should include it in your project.

We don't recommend to copy Nakama C++ SDK to your project because it's quite big in size (~1 Gb).

### Setup for Mac and iOS projects

1. Add `NAKAMA_CPP_SDK/include` in `Build Settings > Header Search Paths`

2. Add libs folder in `Build Settings > Library Search Paths`:
    - `NAKAMA_CPP_SDK/libs/ios` - for iOS
    - `NAKAMA_CPP_SDK/libs/mac` - for Mac

3. In `General > Frameworks, Libraries, and Embedded Content` add following:
    - all `.a` files located in libs folder
    - `foundation` and `security` frameworks

### Setup for Android projects

If you use `CMake` then see [Setup for CMake projects](#setup-for-cmake-projects) section.

If you use `ndk-build` then add following to your `Android.mk` file:

```makefile
# add this to your module
LOCAL_STATIC_LIBRARIES += nakama-cpp

# add this at bottom of Android.mk file
$(call import-add-path, NAKAMA_CPP_SDK)
$(call import-module, nakama-cpp-android)
```

#### Initialize Nakama SDK

For most NativeActivity projects, if you have an entry point like:

```cpp
void android_main(struct android_app* state) {
```

Add include:

```cpp
#include "nakama-cpp/platform/android/android.h"
```

Add the following code at the top of the `android_main` function:

```cpp
Nakama::init(state->activity->vm);
```

#### Shared library size

Don't be afraid that nakama shared library are near 100 Mb in size. After building final apk it will be just few Mb.

#### Android permissions

Android uses a permissions system which determines which platform services the application will request to use and ask permission for from the user. The client uses the network to communicate with the server so you must add the "INTERNET" permission.

```xml
<uses-permission android:name="android.permission.INTERNET"/>
```

### Setup for CMake projects

To link Nakama's static lib add following to your `CMakeLists.txt` file:

```cmake
add_subdirectory(NAKAMA_CPP_SDK ${CMAKE_CURRENT_BINARY_DIR}/nakama-cpp)
target_link_libraries(${APP_NAME} ext_nakama-cpp)
```

To link Nakama's shared lib add following to your `CMakeLists.txt` file:

```cmake
set(NAKAMA_SHARED_LIBRARY TRUE)
add_subdirectory(NAKAMA_CPP_SDK ${CMAKE_CURRENT_BINARY_DIR}/nakama-cpp)
target_link_libraries(${APP_NAME} ext_nakama-cpp)
CopyNakamaSharedLib(${APP_NAME})
```

### Setup for Visual Studio projects

In `Project Settings` add following:

1. Add `NAKAMA_CPP_SDK/include` in `C/C++ > General > Additional Include Directories`
2. Add libs folder in `Linker > General > Additional Library Directories`:
    - `NAKAMA_CPP_SDK/libs/win32/v140` - for VS 2015 x86
    - `NAKAMA_CPP_SDK/libs/win64/v140` - for VS 2015 x64
    - `NAKAMA_CPP_SDK/libs/win32/v141` - for VS 2017 x86
    - `NAKAMA_CPP_SDK/libs/win64/v141` - for VS 2017 x64
    - `NAKAMA_CPP_SDK/libs/win32/v142` - for VS 2019 x86
    - `NAKAMA_CPP_SDK/libs/win64/v142` - for VS 2019 x64
3. Add all `.lib` files located in libs folder in `Linker > Input > Additional Dependencies`

### Custom setup

- add define:
  * `NLOGS_ENABLED` - define it if you want to use Nakama logger. See [Logging](#logging) section
- add include directory: `$(NAKAMA_CPP_SDK)/include`
- add link directory: `$(NAKAMA_CPP_SDK)/libs/{platform}/{ABI}`
- add all libraries for linking from link directory

## Threading model

Nakama C++ is designed to use in one thread only.

## Usage

### Use as static library

Include nakama header and use nakama namespace.

```cpp
#include "nakama-cpp/Nakama.h"

using namespace NAKAMA_NAMESPACE;
```

### Use as dynamic library

Include nakama header and use nakama namespace.

```cpp
#include "nakama-cpp-c-wrapper/NakamaWrapper.h"

using namespace NAKAMA_NAMESPACE;
```

Include following header only once in some source file e.g. in `main.cpp`:
```cpp
#include "nakama-cpp-c-wrapper/NakamaWrapperImpl.h"
```

This header includes implementation of Nakama C++ wrapper. It uses C interface to communicate with Nakama shared library (DLL).

### Client

The client object has many methods to execute various features in the server or open realtime socket connections with the server.

Use the connection credentials to build a client object.

```cpp
NClientParameters parameters;
parameters.serverKey = "defaultkey";
parameters.host = "127.0.0.1";
parameters.port = DEFAULT_PORT;
NClientPtr client = createDefaultClient(parameters);
```

The `createDefaultClient` will create HTTP/1.1 client to use REST API.

### Tick

The `tick` method pumps requests queue and executes callbacks in your thread. You must call it periodically (recommended every 50ms) in your thread.

```cpp
client->tick();
if (rtClient)
    rtClient->tick();
```

Without this the default client and realtime client will not work, and you will not receive responses from the server.

### Authenticate

There's a variety of ways to [authenticate](https://heroiclabs.com/docs/authentication) with the server. Authentication can create a user if they don't already exist with those credentials. It's also easy to authenticate with a social profile from Google Play Games, Facebook, Game Center, etc.

```cpp
string email = "super@heroes.com";
string password = "batsignal";

auto successCallback = [](NSessionPtr session)
{
    std::cout << "session token: " << session->getAuthToken() << std::endl;
};

auto errorCallback = [](const NError& error)
{
};

client->authenticateEmail(email, password, "", false, {}, successCallback, errorCallback);
```

### Sessions

When authenticated the server responds with an auth token (JWT) which contains useful properties and gets deserialized into a `NSession` object.

```cpp
std::cout << session->getAuthToken() << std::endl; // raw JWT token
std::cout << session->getUserId() << std::endl;
std::cout << session->getUsername() << std::endl;
std::cout << "Session has expired: " << session->isExpired() << std::endl;
std::cout << "Session expires at: " << session->getExpireTime() << std::endl;
```

It is recommended to store the auth token from the session and check at startup if it has expired. If the token has expired you must reauthenticate. The expiry time of the token can be changed as a setting in the server.

```cpp
string authtoken = "restored from somewhere";
NSessionPtr session = restoreSession(authtoken);
if (session->isExpired()) {
    std::cout << "Session has expired. Must reauthenticate!" << std::endl;
}
```

### Requests

The client includes lots of builtin APIs for various features of the game server. These can be accessed with the async methods. It can also call custom logic as RPC functions on the server. These can also be executed with a socket object.

All requests are sent with a session object which authorizes the client.

```cpp
auto successCallback = [](const NAccount& account)
{
    std::cout << "user id : " << account.user.id << std::endl;
    std::cout << "username: " << account.user.username << std::endl;
    std::cout << "wallet  : " << account.wallet << std::endl;
};

client->getAccount(session, successCallback, errorCallback);
```

### Realtime client

The client can create one or more realtime clients with the server. Each realtime client can have it's own events listener registered for responses received from the server.

```cpp
bool createStatus = true; // if the socket should show the user as online to others.
// define realtime client in your class as NRtClientPtr rtClient;
rtClient = client->createRtClient(DEFAULT_PORT);
// define listener in your class as NRtDefaultClientListener listener;
listener.setConnectCallback([]()
{
    std::cout << "Socket connected" << std::endl;
});
rtClient->setListener(&listener);
rtClient->connect(session, createStatus);
```

Don't forget to call `tick` method. See [Tick](#tick) section for details.

### Logging

#### Initializing Logger

Client logging is off by default.

To enable logs output to console with debug logging level:

```cpp
NLogger::initWithConsoleSink(NLogLevel::Debug);
```

To enable logs output to custom sink with debug logging level:

```cpp
NLogger::init(sink, NLogLevel::Debug);
```

#### Using Logger

To log string with debug logging level:

```
NLOG_DEBUG("debug log");
```

formatted log:

```
NLOG(NLogLevel::Info, "This is string: %s", "yup I'm string");
NLOG(NLogLevel::Info, "This is int: %d", 5);
```

Changing logging level boundary:

```
NLogger::setLevel(NLogLevel::Debug);
```

`NLogger` behaviour depending on logging level boundary:

- `Debug` writes all logs.

- `Info` writes logs with `Info`, `Warn`, `Error` and `Fatal` logging level.

- `Warn` writes logs with `Warn`, `Error` and `Fatal` logging level.

- `Error` writes logs with `Error` and `Fatal` logging level.

- `Fatal` writes only logs with `Fatal` logging level.

Note: to use logging macroses you have to define `NLOGS_ENABLED`.

### Websockets transport

Nakama C++ client has built-in support for WebSocket. This is available on all supported platforms.

Client will default to use the Websocket transport provided by [C++ REST SDK](https://github.com/microsoft/cpprestsdk).

You can use a custom Websocket transport by implementing the [NRtTransportInterface](https://github.com/heroiclabs/nakama-cpp/blob/master/include/nakama-cpp/realtime/NRtTransportInterface.h):

```cpp
rtClient = client->createRtClient(port, websockets_transport);
```

For more code examples, have a look at:

* [NWebsocketCppRest](https://github.com/heroiclabs/nakama-cpp/blob/master/src/realtime/NWebsocketCppRest.h)
* [NCocosWebSocket](https://github.com/heroiclabs/nakama-cocos2d-x/blob/master/example/Classes/NakamaCocos2d/NCocosWebSocket.h)

#### Activity timeout

Built-in websocket transport supports "Activity timeout" feature - if no any message received from server during "Activity timeout" then connection will be closed. Set 0 to disable this feature (default value).

```cpp
rtClient->getTransport()->setActivityTimeout(20000); // 20 sec
```

You can change ping period on server - `ping_period_ms` parameter:

https://heroiclabs.com/docs/install-configuration/#socket

## Contribute

The development roadmap is managed as GitHub issues and pull requests are welcome. If you're interested to enhance the code please open an issue to discuss the changes or drop in and discuss it in the [community forum](https://forum.heroiclabs.com).

## Source Builds

Clone the Nakama C++ repository:

`git clone --recurse-submodules -j8 git://github.com/heroiclabs/nakama-cpp.git`

To update all submodules:

`git submodule update --init --recursive`

Change submodule branch:

- edit `.gitmodules`

- `git submodule update --remote`

### Build Prerequisites

- git
- python 2.7 or 3.x
- CMake 3.15+
- go
- perl
- Visual Studio 2015, 2017 or 2019 - for Windows only

Third party libraries:

- boost 1.69 - must be installed in system and set path to `BOOST_ROOT` system variable.
  Used by `cpprest` library on Windows, Mac and Linux.
- grpc - in source control as git submodule
- optional-lite - in source control
- cpprestsdk - in source control
- rapidjson - in source control

### Build Configuration

Build configuration is located here:

```bash
build/build_config.py
```

In the build configuration you can enable components depending on your needs.

There are following components:

* REST client (HTTP/1.1)
* gRPC client
* HTTP transport using C++ REST SDK
* Websocket transport using C++ REST SDK
* C API
* Tests

In the config you can also set whenever you need to build `nakama-cpp` as static, dynamic library or both version.

### Building for Windows

```bash
cd build\windows
python build_windows.py -m Mode -a Arch -t Toolset --dll
```
Where `Mode` is build mode: `Debug` or `Release`

Where `Arch` is architecture: `x86` or `x64`

Where `Toolset` is platform toolset:

- `v140` - Visual Studio 2015
- `v141` - Visual Studio 2017
- `v142` - Visual Studio 2019

`--dll` is optional parameter. If set then Nakama will be built as DLL otherwise as static library.

It builds and copies nakama lib to release folder.

To build for all modes, architectures and toolsets:

```bash
cd build\windows
python build_windows_all.py
```

### Building for Mac

Prerequisites:
```bash
sudo xcode-select --install
brew install autoconf automake libtool shtool
brew install gflags
brew install cmake ninja boost
```

Build:

```bash
cd build/mac
python build_mac.py --dylib
```

`--dylib` is optional parameter. If set then Nakama will be built as dynamic library otherwise as static.

It builds in `Release` mode and copies nakama lib to release folder.

### Building for iOS

To build for one architecture:

```bash
cd build/ios
python build_ios.py Arch --dylib
```
Where `Arch` is architecture: `arm64`, `armv7`, `armv7s` or `x86_64`.

`--dylib` is optional parameter. If set then Nakama will be built as dynamic library otherwise as static.

It builds in `Release` mode.

To build for all architectures `arm64`, `armv7`, `armv7s` and `x86_64`:

```bash
cd build/ios
python build_ios_all.py
```

It builds in `Release` mode, creates universal libraries and copies them to release folder.

### Building for Linux

We use Ubuntu 18.04 amd64.

Prerequisites:

To automatically install all dependencies call:

```bash
build/linux/install_deps.sh
```

It will install all needed packages, will download and build boost and CMake.

Execute command which will be printed at end.

To manually install dependencies:

- `sudo apt-get install build-essential autoconf libtool pkg-config`
- `sudo apt-get install libgflags-dev libgtest-dev`
- `sudo apt-get install clang libc++-dev golang perl`
- download `boost` 1.69 sources and build them:

  `./bootstrap.sh --with-libraries=system,chrono,thread`

  `./b2`

  set `BOOST_ROOT` env var to `boost` folder:

  `export BOOST_ROOT={path to boost}`

- download `CMake` 3.15+ sources and build them:

  `./bootstrap && make && make install`

To build as static or shared object library:

```bash
cd build/linux
python build_linux.py --so
```

`--so` is optional parameter. If set then Nakama will be built as shared library otherwise as static.

It builds in `Release` mode and copies nakama lib to release folder.

To build both static and shared object library (see [Build Configuration](#build-configuration)):

```bash
cd build/linux
python build_linux_all.py
```

### Building for Android

Currently buid for Android is supported on Mac OS and Linux.

Set `ANDROID_NDK` or `NDK_ROOT` system variable to Android NDK folder.

To build for one ABI:

```bash
cd build/android
python build_android.py ABI --so
```
Where `ABI` is Android ABI e.g. `armeabi-v7a`, `arm64-v8a`, `x86` or `x86_64`

`--so` is optional parameter. If set then Nakama will be built as shared library otherwise as static.

It builds for Andoid API level 16 in `Release` mode.

To build for all ABIs `armeabi-v7a`, `arm64-v8a`, `x86` and `x86_64`:

```bash
cd build/android
python build_android_all.py
```

## Tests

Tests are built when you build Nakama C++ SDK for desktop OS (Windows, Mac or Linux).

By default tests connect to local server by `127.0.0.1`.
To use another IP of your server, edit `test/test_serverConfig.h` file.

### Prerequisites

Tests require lua modules: download them from https://github.com/heroiclabs/nakama/tree/master/data/modules and put to `<nakama-server>/data/modules`.

Restart server.

### Run tests

Run tests executable (console application):

```bash
build/{platform}/build/test/Debug/nakama-test
```

## Full C++ Client example

You can find the C++ Client example [here](https://github.com/heroiclabs/nakama-cpp/tree/master/examples/nakama-cmake-client-example)

## C language support

Please follow [README-C](README-C.md)

## Generating Docs

Prerequisites: `doxygen`

Generate docs:

```bash
cd docs
doxygen Doxyfile
```

## License

This project is licensed under the [Apache-2 License](https://github.com/heroiclabs/nakama-dotnet/blob/master/LICENSE).

## Special Thanks

Thanks to [@dimon4eg](https://github.com/dimon4eg) for this excellent support on developing Nakama C/C++, Cocos2d-x and Unreal client libraries.
