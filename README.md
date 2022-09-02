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

To build Linux release you can use provider Docker image like following:

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
