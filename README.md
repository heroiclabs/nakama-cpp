Nakama C++ SDK
=============

> General C++ client for Nakama server.

[Nakama](https://github.com/heroiclabs/nakama) is an open-source server designed to power modern games and apps. Features include user accounts, chat, social, matchmaker, realtime multiplayer, and much [more](https://heroiclabs.com).

This client implements the full API and socket options with the server. It's written in C++ with minimal dependencies to support Cocos2d-x, Unreal and other custom engines and frameworks.

If you experience any issues with the client, it can be useful to enable debug logs (see [Logging](#Logging) section) and [open an issue](https://github.com/heroiclabs/nakama-cpp/issues).

Full documentation is online - https://heroiclabs.com/docs

## Getting Started

You'll need to setup the server and database before you can connect with the client. The simplest way is to use Docker but have a look at the [server documentation](https://github.com/heroiclabs/nakama#getting-started) for other options.

1. Install and run the servers. Follow these [instructions](https://heroiclabs.com/docs/install-docker-quickstart).

2. Ensure that you are on one of the supported platforms:

- Windows - Visual Studio 2015, 2017 (x86)
- Android - Android 4.1 (armeabi-v7a, arm64-v8a, x86)
- Linux - Ubuntu 14.04.5 (x86, x64)
- Mac
- iOS - coming soon

In theory any platform that meets the requirement for `grpc` and `boost` is also supported. The client is compiled with C++11.

3. Download the client from the [releases page](https://github.com/heroiclabs/nakama-cpp/releases) and import it into your project. You can also [build from source](#source-builds).

<!-- You can download clients that are tailored for [Cocos2d-x](https://github.com/heroiclabs/nakama-cocos2d-x) or [Unreal 4](https://github.com/heroiclabs/nakama-unreal). -->

4. Integrate the client library into your project:

- add defines: 
  - `NAKAMA_API=`
  - `NLOGS_ENABLED`
- add include directory: `$(NAKAMA_CPP_SDK)/include`
- add link directory: `$(NAKAMA_CPP_SDK)/libs/{platform}/{ABI}`
- add link libraries:
  - `nakama-cpp`
  - `grpc++`
  - `libprotobuf`
  - `gpr`
  - `grpc`
  - `cares`
  - `crypto`
  - `ssl`
  - `address_sorting`

For Windows:

- Add extension `.lib` to libs names e.g. `nakama-cpp.lib`
- To debug you must add `d` suffix to libs names e.g. `nakama-cppd.lib`

For Android:

The client uses the network to communicate with the server so you must add the "INTERNET" permission.

```xml
<uses-permission android:name="android.permission.INTERNET"/>
```

## Usage

The client object has many methods to execute various features in the server or open realtime socket connections with the server.

Include nakama header.

```cpp
#include "nakama-cpp/Nakama.h"
```

Use nakama namespace.

```cpp
using namespace Nakama;
```

Use the connection credentials to build a client object.

```cpp
DefaultClientParameters parameters;
parameters.serverKey = "defaultkey";
parameters.host = "127.0.0.1";
parameters.port = 7349;
NClientPtr client = createDefaultClient(parameters);
```

## Tick

Allow the client to execute your callbacks in your thread. To do this you must call `tick` method periodically (recommended every 50ms) in your thread.

```cpp
client->tick();
if (rtClient)
    rtClient->tick();
```

Without this the default client and realtime client will not work, and you will not send or receives responses from the server.

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

client->authenticateEmail(email, password, "", false, successCallback, errorCallback);
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
int port = 7350; // different port to the main API port
bool createStatus = true; // if the socket should show the user as online to others.
NRtClientPtr rtClient = client->createRtClient(port);
// create listener in your class as NRtDefaultClientListener listener;
listener.setConnectCallback([]()
    {
        std::cout << "Socket connected" << std::endl;
    });
rtClient->setListener(&listener);
rtClient->connect(session, createStatus, protocol);
```

Don't forget to call `tick` method. See [Tick](#Tick) section for details.

### Logging

By default client logging is turned off.

To enable logs output to console with debug logging level:

```cpp
NLogger::initWithConsoleSink(NLogLevel::Debug);
```

To enable logs output to custom sink with debug logging level:

```cpp
NLogger::init(sink, NLogLevel::Debug);
```

#### Websockets transport

Nakama C++ client has built-in support for WebSocket. This is currently tested on Windows and Mac.

To add support for Android, you need to use the ported version of the `boost` library for Android. This is because `websocketpp` depends on `boost`.

Client will default to use the provided Websocket transport if available on the platform. You can use a custom Websocket transport if it implements the `NRtTransportInterface`:

```cpp
rtClient = client->createRtClient(port, websockets_transport);
```

For more code examples, have a look at `NWebSocket` is [Cocos2d-x client](https://github.com/heroiclabs/nakama-cocos2d-x/blob/master/Classes/NakamaCocos2d/NWebSocket.h).

## Contribute

The development roadmap is managed as GitHub issues and pull requests are welcome. If you're interested to enhance the code please open an issue to discuss the changes or drop in and discuss it in the [community chat](https://gitter.im/heroiclabs/nakama).

## Source Builds

Clone the Nakama C++ repository:

`git clone --recurse-submodules -j8 git://github.com/heroiclabs/nakama-cpp.git`

To update all submodules:

`git submodule update --init --recursive`

Change submodule branch:

- edit `.gitmodules`

- `git submodule update --remote`

## Build Prerequisites

- git
- python 2.7
- cmake 3.10+
- go
- perl
- Visual Studio 2015 or 2017 - for Windows only
- boost - for Windows and Mac, used by websocketpp library

Third party libraries:

- boost - must be installed in system and set path to `BOOST_ROOT` system variable.
- grpc - in source control as git submodule
- optional-lite - in source control
- websocketpp - in source control

### Building for Windows

```bash
cd build\windows
python build_windows.py -m Mode -a Arch
```
Where `Mode` is build mode: `Debug` or `Release`

Where `Arch` is architecture: `x86` or `x64`

It builds and copies nakama lib to release folder.

### Building for Mac

```bash
cd build\mac
python build_mac.py
```
It builds in `Release` mode and copies nakama lib to release folder.

### Building for Linux

To build for x86 architecture use x86 linux distro (we use Ubuntu 14.04.5 i386)

To build for x64 architecture use x64 linux distro (we use Ubuntu 14.04.5 amd64)

Prerequisites:

- `sudo apt-get install build-essential autoconf libtool pkg-config`
- `sudo apt-get install libgflags-dev libgtest-dev`
- `sudo apt-get install clang libc++-dev`
- `sudo apt-get install golang`
- `sudo apt-get install perl`
- download `boost` sources and build them:

  `./bootstrap.sh`

  `./b2`

  set `BOOST_ROOT` env var to `boost` folder:

  `export BOOST_ROOT={path to boost}`

- download `cmake` 3.10+ sources and build them:

  `./bootstrap && make && make install`

```bash
cd build\linux
python build_linux.py
```
It builds in `Release` mode and copies nakama lib to release folder.

### Building for Android

Set `ANDROID_NDK` or `NDK_ROOT` system variable to Android NDK folder.

```bash
cd build/android
python build_android.py ABI
```
Where `ABI` is Android ABI e.g. `armeabi-v7a`, `arm64-v8a` or `x86`

It builds for Andoid API level 16 in `Release` mode.

## Tests

Tests are built when you build Nakama C++ SDK for desktop OS (Windows or Mac).

By default tests try to connect to local server by `127.0.0.1`.
To use another IP of your server, edit `test/test_server_config.h` file.

Run tests (console application):

```bash
build/{platform}/build/test/Debug/nakama-test
```

### License

This project is licensed under the [Apache-2 License](https://github.com/heroiclabs/nakama-dotnet/blob/master/LICENSE).

### Special Thanks

Thanks to @dimon4eg for this excellent support on developing Nakama C++, Cocos2d-x and Unreal client libraries.
