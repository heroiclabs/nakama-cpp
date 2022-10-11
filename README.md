Nakama C/C++ Client SDK
=============

> C/C++ client for Nakama server.

[Nakama](https://github.com/heroiclabs/nakama) is an open-source server designed to power modern games and apps. Features include user accounts, chat, social, matchmaker, realtime multiplayer, and much [more](https://heroiclabs.com).

This client implements the full API and socket options with the server. It's written in C and C++11 with minimal dependencies to support Unreal, game consoles, Cocos2d-x, and other custom engines and frameworks.

If you experience any issues with the client, [open an issue](https://github.com/heroiclabs/nakama-cpp/issues).

Full documentation is online - https://heroiclabs.com/docs

# Usage

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

# How to build

## Prerequisite

### Windows

- [CMake >= 3.22](https://cmake.org/download/)
- [Ninja](https://ninja-build.org/)
- [Build Tools for Visual Studio 2022](https://visualstudio.microsoft.com/downloads/)

### Linux

- CMake
- Ninja
- gcc-c++/clang++

#### Ubuntu 18.04

Fresh Ubuntu 18.04 setup:

```
sudo apt update
sudo apt install git pkg-config g++ curl zip unzip tar make
mkdir -p ~/opt; curl -L https://github.com/Kitware/CMake/releases/download/v3.23.1/cmake-3.23.1-linux-x86_64.tar.gz  | tar -C ~/opt -xzf -
mkdir -p ~/bin; ln -s ~/opt/cmake-3.23.1-linux-x86_64/bin/cmake ~/bin/
cd /tmp; curl -L -O https://github.com/ninja-build/ninja/releases/download/v1.10.2/ninja-linux.zip; unzip ninja-linux.zip; mv ninja ~/bin
exec /bin/bash -l   # make ~/bin available on PATH
git clone /mnt/z/repos/nakama-cpp-mono ~/localrepos/nakama-cpp-mono
cd ~/localrepos/nakama-cpp-mono
./cmake/vcpkg/bootstrap-vcpkg.sh
```

If you plan to use `WITH_LIBCXX`, then also do following:

```
sudo add-apt-repository --yes ppa:ubuntu-toolchain-r/test
sudo apt install g++-11 python3-distutils
export CC=/usr/bin/gcc-11
export CXX=/usr/bin/g++-11
```


### OS X

- brew install ninja cmake pkg-config
- XCode or XCode command line tools

### Android
- Ensure the Android SDK is installed. Android has special build instructions -- we use Gradle and call into CMake from it:
`cd ./android && ./gradlew assemble`. The .aar artifact can be used from the build tree.

## Build

There are preconfigured presets in the [CMakePresets.json](./CMakePresets.json).
You can get list of presets for your system with:

```
cmake --list-presets
```

Then configure the build system.

```
cmake --preset linux-amd64
```

Configuration step  builds all necessary dependencies and installs them under `./build/*/vcpkg_installed`.


Next, build the SDK:

```
cmake --build --preset release-linux-amd64
```

### Linux

To build Linux release you can use provided Docker image like following:

```
docker buildx build -f scripts/Dockerfile --progress=plain --output=./out .
```

### Build modifiers

Presets mostly represent platforms SDK can be built for. Sometimes
within platforms build configuration need to be modified, but if we create
preset for each build configuration we'd have too many of them. We have
a way to alter build behaviour of any preset with a build modifiers mechanism.

Supported build modifiers are:

- `LIBHTTPCLIENT_FORCE_WEBSOCKETPP`: On Windows platforms libhttpclient always includes
  websocketpp transport and uses it if Windows doesn't support websocket natively (< Windows 8).
  You can set this build modifier to force use of websocketpp transport, so that it can be tested without
  installing Windows 7.
- `WITH_LIBCXX`: dynamically link with libc++ instead of libstdc++ on Linux platform.
  - `LIBCXX_STATIC`: link libc++ statically
- `UNREAL`: creates binaries that are compatible with Unreal Engine.
  - Using this build modifier produces blank build with no HTTP or WS transports, because Unreal module provides those.
  - When compiling on Linux, it automatically enables `WITH_LIBCXX=ON` and `LIBCXX_STATIC=ON`
- `ADDRESS_SANITIZER`: instrument library with [AddressSanitizer](https://github.com/google/sanitizers/wiki/AddressSanitizer)

Build modifiers are CMake variables passed at configure time using `-D` switch.
Example use:

```
cmake --preset linux-amd64 -DWITH_LIBCXX=ON
```

## Release

```
cmake --install --preset linux-amd64 --config MinSizeRel
```

You should see dynamic library and headers in the `./out` directory. This is your release.

It is safe to combine (overlay) multiple platforms releases in the same directory structure, because
binary artifacts paths won't clash and include files are identical on all platforms.

### MacOSX Universal binary

Currently, our dependency manager can't build non-CMake based projects as universal binary.
Watch [this PR](https://github.com/microsoft/vcpkg/pull/22898) for a proper fix. Until then
building universal binaries requires building shared libs for `arm64` and `x86_64` architectures
and gluing them together with [lipo](https://developer.apple.com/documentation/apple-silicon/building-a-universal-macos-binary/)
tool.

To build universal binary first compile individual shared lib for arm64 and x86_64. Following commands are for M1,
adjust preset names if you are on Intel CPU:

```
cmake --preset macosx-x64-host_arm64
cmake --build build/macosx-x64-host_arm64 --config MinSizeRel --target install

cmake --preset macosx-arm64-host_arm64
cmake --build build/macosx-arm64-host_arm64 --config MinSizeRel --target install
```

```
cp -r out/macosx-x64 out/macosx-universal
lipo -create -output out/macosx-universal/nakama-sdk.framework/Versions/A/nakama-sdk out/macosx-{arm64,x64}/nakama-sdk.framework/nakama-sdk
```

You can then archive and release `out/osx-universal`  directory.

## Transports

Platforms vary in their implementation of transports for HTTP and WS. One of the
transports, `libhttpclient` itself can use different implementations depending on the platform.


HTTP:

Platform | Current                  | Planned |
  --- |--------------------------| ---
 Win32 | libhttpclient -> winhttp | -
Linux | libhttpclient->curl      | -
MacOS | libhttpclient -> OS      | -
iOS   | libhttpclient -> OS      | -
XDK  | libhttpclient -> OS      | -
GDK | libhttpclient -> OS      | -
PS4/5 | OS                       |  -
Unreal | $platform              | sdk-blank + unreal

Websockets:

Platform | Current                      | Planned |
--- |------------------------------| ---
Win32 | libhttpclient -> winhttp     | -
Linux | wslay                        | -
MacOS | wslay                        | libhttpclient -> OS (min OSX 10.14)
iOS   | wslay                        | libhttpclient -> OS (min iOS xx.xx)
XDK  | libhttpclient -> OS          | -
GDK | libhttpclient -> OS          | -
PS4/5 | wslay                        |  OS (new SDK)
Unreal | $platform                  | sdk-blank + unreal


### Blank build

Transports are the messiest and hardest part when it comes to compiling on multiple platforms.
The rest of the code is "pure" and relies on nothing, but stdlib, making it fairly trivial to compile.

It is possible to make a "blank" build of the SDK with no transports included. An example use case
would be to have it shipped alongside a game executable, where game executable is responsible
for providing `NHttpTransportInterface` and `NRtTransportInterface` implementations. For instance
if the game is Unreal based, then builtin Unreal HTTP and WS clients can be used to implement
transport and passed to the blank SDK.

To make a blank build pass `-D HTTP_IMPL=OFF -D WS_IMPL=OFF` to cmake at configure time. Example:

```
cmake --preset win-x64 -D HTTP_IMPL=OFF -D WS_IMPL=OFF
cmake --build --preset release-win-x64
```

### Non-public (consoles) transports

Consoles code can't be published to the public repository and their code is hosted in
https://github.com/heroiclabs/nakama-cpp-private repository.  Private repository hosts
just source code and minimal CMake files required to build them, rest of build system
resides here.

Consoles build presets will implicitly check out nakama-cpp-private repository, so you don't
need to do anything special. You can configure commit used with `NAKAMA_PRIVATE_REPO_GIT_TAG` variable,
which can be set with `-DNAKAMA_PRIVATE_REPO_GIT_TAG=123456` cmd flag at configuration time

You can edit files in-place in `./build/${preset}/_deps/nakama-cpp-private-src` for a speedy development. Once
done commit and push by running git from that location, it is proper git clone, so normal git commands should work.

GDK is expected to be installed completely normally, per Microsoft instructions. Use the March 2022 edition for Unreal Engine.
