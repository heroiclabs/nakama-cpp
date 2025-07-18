Nakama C/C++ Client SDK
=============

> C/C++ client for Nakama server.

[Nakama](https://github.com/heroiclabs/nakama) is an open-source server designed to power modern games and apps. Features include user accounts, chat, social, matchmaker, realtime multiplayer, and much [more](https://heroiclabs.com).

This client implements the full API and socket options with the server. It's written in C and C++11 with minimal dependencies to support Unreal, Cocos2d-x, Oculus, and other custom engines and frameworks.

We also support various game consoles. Please reach out to support@heroiclabs.com to discuss console support for your project.

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
rtClient = client->createRtClient();
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

Note: to use logging macros you have to define `NLOGS_ENABLED`.

### Websockets transport

Nakama C++ client has built-in support for WebSocket. This is available on all supported platforms.

You can use a custom Websocket transport by implementing the [NRtTransportInterface](https://github.com/heroiclabs/nakama-cpp/blob/master/include/nakama-cpp/realtime/NRtTransportInterface.h):

```cpp
rtClient = client->createRtClient(websockets_transport);
```

#### Activity timeout

Built-in websocket transport supports "Activity timeout" feature - if no any message received from server during "Activity timeout" then connection will be closed. Set 0 to disable this feature (default value).

```cpp
rtClient->getTransport()->setActivityTimeout(20000); // 20 sec
```

You can change ping period on server - `ping_period_ms` parameter:

https://heroiclabs.com/docs/install-configuration/#socket

#### Android

To use our native C++ library in your Android application, you will need to include an additional .aar file that we ship for SSL support.

For example, in Gradle:

```
implementation files("<path/to/libnakama-sdk.aar>")
```

Then you will need to load our native library from Java by calling `System.loadLibrary("nakama-sdk")` when your activity is created.

# How to build

First, clone the repository. It uses some submodules that are not publicly available and are needed to build
`nakama-sdk` on NDAed platforms. It is not an error if `./submodules/devkits` and `./submodules/private` are
not available. To clone cleanly use following commands:

```
git clone --filter=blob:none https://github.com/heroiclabs/nakama-cpp 
cd nakama-cpp
git submodule update --filter=blob:none --init ':!submodules/devkits' ':!submodules/private'
```

### Windows

Use following command from powershell to install minimal set of required dependencies
```
winget install Git.Git Ninja-build.Ninja Kitware.CMake

winget install Microsoft.VisualStudio.2022.BuildTools --override ("--wait " +
   "--passive " +
   "--add Microsoft.VisualStudio.Component.VC.Tools.ARM " +
   "--add Microsoft.VisualStudio.Component.VC.Tools.ARM64 " +
   "--add Microsoft.VisualStudio.Component.VC.Tools.x86.x64 " +
   "--add Microsoft.VisualStudio.Component.Windows11SDK.22621 ")
```


- Git
- [CMake >= 3.29](https://cmake.org/download/)
- [Ninja](https://ninja-build.org/)
- [Build Tools for Visual Studio 2022](https://visualstudio.microsoft.com/downloads/)
- Windows 11 SDK v.22000


### Linux

- CMake
- Ninja
- gcc-c++/clang++
- libcurl-dev (Debian/Ubuntu) or libcurl-devel (RPM based distros)

Fedora:
```
sudo dnf install cmake ninja-build libcurl-devel gcc g++
```

#### Ubuntu 18.04

Fresh Ubuntu 18.04 setup:

```
sudo apt update
sudo apt install git pkg-config g++ curl zip unzip tar make
mkdir -p ~/opt; curl -L https://github.com/Kitware/CMake/releases/download/v3.23.1/cmake-3.23.1-linux-x86_64.tar.gz  | tar -C ~/opt -xzf -
mkdir -p ~/bin; ln -s ~/opt/cmake-3.23.1-linux-x86_64/bin/cmake ~/bin/
cd /tmp; curl -L -O https://github.com/ninja-build/ninja/releases/download/v1.10.2/ninja-linux.zip; unzip ninja-linux.zip; mv ninja ~/bin
exec /bin/bash -l   # make ~/bin available on PATH
git clone /mnt/z/repos/nakama-cpp ~/localrepos/nakama-cpp
cd ~/localrepos/nakama-cpp
${VCPKG_ROOT}/bootstrap-vcpkg.sh
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

The configuration step builds all necessary dependencies and installs them under `./build/*/vcpkg_installed`.


Next, build the SDK:

```
cmake --build build/linux-amd64 --config MinSizeRel --target install
```

### Linux

To build Linux release you can use provided Docker image like following:

```
docker buildx build -f scripts/Dockerfile --progress=plain --output=./out .
```

### Android

To build for Android set your `ANDROID_NDK_HOME` environment variable to your NDK before building.

Your NDK is typically located within your SDK:`<sdk>/ndk/<ndk-version>`

Our prebuilt libraries target Android NDK 25.1.8937393.

### Mac OSX Universal binary

Mac OSX preset is `macosx-universal` builds Framework with universal binaries
for x86_64 and arm64 architectures.

### Build modifiers

Presets mostly represent platforms SDK can be built for. Sometimes
within platforms build configuration need to be modified, but if we create
preset for each build configuration we'd have too many of them. We have
a way to alter build behaviour of any preset with a build modifiers mechanism.

Supported build modifiers are:

- `ADDRESS_SANITIZER`: instrument library with [AddressSanitizer](https://github.com/google/sanitizers/wiki/AddressSanitizer)

Build modifiers are CMake variables passed at configure time using `-D` switch.
Example use:

```
cmake --preset linux-amd64 -DADDRESS_SANITIZER=ON
```

## Release

```
cmake --install --preset linux-amd64 --config MinSizeRel
```

You should see dynamic library and headers in the `./out` directory. This is your release.

It is safe to combine (overlay) multiple platforms releases in the same directory structure, because
binary artifacts paths won't clash and include files are identical on all platforms.


## Transports

Platforms vary in their implementation of transports for HTTP and WS. One of the
transports, `libhttpclient` itself can use different implementations depending on the platform.

HTTP:

Platform | Transport                |
  --- |--------------------------|
Windows | libhttpclient -> winhttp |
Android | libcurl                  |
Linux | libcurl (system from OS) |
MacOS | libcurl                  |
iOS   | libcurl                  |

Websockets:

Platform | Transport
--- |------------------------------|
Windows | libhttpclient -> winhttp |
Android | wslay                    |
Linux | wslay                      |
MacOS | wslay                      |
iOS   | wslay                      |


# How to integrate the SDK

There are three ways of integrating the SDK into your build.

## Github Release
We provide headers and binaries for all supported platforms in our releases section.

## CMake
After downloading it to a folder you've configured CMake to look for targets in, you can import our package via the `find_package` command in CMake: `find_package(nakama-sdk)`.

## vcpkg

Our SDK integrates with vcpkg by providing itself and a few dependencies through a git registry. To include it in your vcpkg manifest, create a `vcpkg-configuration.json` in your root directory.
```
{
    "registries":
    [
        {
            "kind": "git",
            "repository": "https://github.com/heroiclabs/nakama-vcpkg-registry",
            "baseline": "<commit>",
            "reference": "<branch>",
            "packages": ["nakama-sdk"]
        }
    ]
}
```
Then you can add it as you would any other vcpkg port in your `vcpkg.json`:
```
    "dependencies": [
      {
        "name": "nakama-sdk"
      }]
```
