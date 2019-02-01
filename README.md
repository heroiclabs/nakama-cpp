Nakama C++ SDK
=============

> General C++ client for Nakama server 2.3.x.

[Nakama](https://github.com/heroiclabs/nakama) is an open-source server designed to power modern games and apps. Features include user accounts, chat, social, matchmaker, realtime multiplayer, and much [more](https://heroiclabs.com).

This client implements the full API and socket options with the server. It's written in C++ with minimal dependencies to support Unity, Cocos2d-x and other engines and frameworks.

If you encounter any issues with the server you can generate diagnostics for us with `nakama doctor`. Send these to support@heroiclabs.com or [open an issue](https://github.com/heroiclabs/nakama/issues). If you experience any issues with the client, it can be useful to enable debug logs (see [Logging](#Logging) section) and [open an issue](https://github.com/heroiclabs/nakama-cpp/issues).

## Getting Started

You'll need to setup the server and database before you can connect with the client. The simplest way is to use Docker but have a look at the [server documentation](https://github.com/heroiclabs/nakama#getting-started) for other options.

1. Install and run the servers. Follow these [instructions](https://heroiclabs.com/docs/install-docker-quickstart).

2. Choose your engine:

    2.1. [Unreal](https://github.com/heroiclabs/nakama-unreal)

    2.2. [Cocos2d-x](https://github.com/heroiclabs/nakama-cocos2d-x)

    or download the client from the [releases page](https://github.com/heroiclabs/nakama-cpp/releases) and import it into your project. You can also [build from source](#Building).

    See [Integration](#Integration) section.

## Supported platforms

Nakama C++ are released for:

* Windows - x86, Visual Studio 2015, 2017
* Android - armeabi-v7a, arm64-v8a, x86
* iOS, Mac, Linux - coming soon

In theory any platform is supported which `grpc` and `boost` support.

## Usage

The client object has many methods to execute various features in the server or open realtime socket connections with the server.

Include nakama header.

```cpp
#include "nakama-cpp/Nakama.h"
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

To allow the client execute your callbacks in your thread you have to call `tick` method periodically in your thread. Each 50 ms is ok.
```cpp
client->tick();
if (rtClient)
    rtClient->tick();
```

Without this the default client and realtime client will not work.

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

The client can create one or more realtime clients with the server. Each realtime client can have it's own event listeners registered for responses received from the server.

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

Don't forget to call `tick`. See [tick](#Tick) section.

### For Android

Android uses a permissions system which determines which platform services the application will request to use and ask permission for from the user. The client uses the network to communicate with the server so you must add the "INTERNET" permission.

```xml
<uses-permission android:name="android.permission.INTERNET"/>
```

#### Logging

By default logs are off.

To enable logs output to console with debug logging level: `NLogger::initWithConsoleSink(NLogLevel::Debug);`

To enable logs output to custom sink with debug logging level: `NLogger::init(sink, NLogLevel::Debug);`

## Integration

To integrate SDK into your project, follow instructions:

add defines: `NAKAMA_API=`, `NLOGS_ENABLED`

add include directory: `$(NAKAMA_CPP_SDK)/include`

add link directory: `$(NAKAMA_CPP_SDK)/libs/{platform}/{ABI}`

add link libraries:

    nakama-cpp
    grpc++
    libprotobuf
    gpr
    grpc
    cares
    crypto
    ssl
    address_sorting

For Windows add extension `.lib` to libs names e.g. `nakama-cpp.lib`
For Windows Debug you must add `d` suffix to libs names e.g. `nakama-cppd.lib`

#### Websockets transport

Nakama C++ has built in websockets transport.
Currently it is available only on Windows platform.
Possible to support Android if use android port of boost (websocketpp uses boost).

Default client will create and use default websockets transport if it's available on the platform.

#### Custom websockets transport

To use custom websockets transport, implement `NRtTransportInterface`, create it and pass pointer to:
```cpp
rtClient = client->createRtClient(port, websockets_transport);
```

For example cocos2d-x has built-in websockets.

Nakama cocos2d-x client uses it.

Look at `NWebSocket` class in: [Cocos2d-x](https://github.com/heroiclabs/nakama-cocos2d-x/blob/master/Classes/NakamaCocos2d/NWebSocket.h)

### Clonning

Clone the Nakama C++ sources:

`git clone --recurse-submodules -j8 git://github.com/heroiclabs/nakama-cpp.git`

To update all submodules:

`git submodule update --init --recursive`

## Building

The build is primarily intended for SDK developers.

### Build Prerequisites

* git
* python 2.7
* cmake 3.10+
* go
* perl
* Visual Studio 2015 or 2017 - for Windows only
* boost - for Windows and Mac, used by websocketpp library

Third party libries:

* grpc - added as git submodule
* boost - must be installed in system and set path to `BOOST_ROOT` system variable.
* optional-lite - added to git
* websocketpp - added to git

### Building for Windows

```bash
cd build\win32
python build_win32.py -m Mode
```
Where `Mode` is build mode: Debug or Release

### Building for Android

At first build Nakama C++ for your desktop OS because android build will use `protoc` and `grpc_cpp_plugin` executables.
Set `ANDROID_NDK` or `NDK_ROOT` system variable to Android NDK folder.

```bash
cd build/android
python build_android.py ABI
```
Where `ABI` is Android ABI e.g. armeabi-v7a, arm64-v8a or x86

It builds for Andoid API level 16 in Release mode.

## Tests

Tests are built when you build Nakama C++ SDK for desktop OS (Windows or Mac).
By default tests try to connect to local server by `127.0.0.1`.
To use another IP of your server, edit `test/test_server_config.h` file.

Run tests (console application):

```bash
build/{platform}/build/test/Debug/nakama-test
```

### License

This project is licensed under the [Apache-2 License](https://github.com/heroiclabs/nakama-cpp/blob/master/LICENSE).
