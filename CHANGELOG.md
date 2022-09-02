# Change Log

All notable changes to this project are documented below.

The format is based on [keep a changelog](http://keepachangelog.com/) and this project uses [semantic versioning](http://semver.org/).

### [2.6.0] - [2022-09-02]

### Changed
- This is a rewrite of the C++ repository structure and build system. The user-facing remains unchanged.

### [2.5.1] - [2022-01-16]

### Fixed
- Fixed Party & Matchmaking callbacks.
- Fixed removeMatchmakerParty API.

### [2.5.0] - [2021-07-29]

### Added
- Added realtime parties support.

### Fixed
- Fixed assignment of cursor in listing of storage objects.

## [2.4.1] - [2021-01-16]
### Fixed
- fix join group chat by correcting NChannelType enum values

## [2.4.0] - [2020-11-01]
### Added
- Support server 2.13.0
- Added authentication with Apple ID: `authenticateApple`, `linkApple` and `unlinkApple`
- Added `demoteGroupUsers`
- Added `NClientInterface::rpc` with `http key`
- Added `disableTime` to `NAccount`
- Added `updateTime` to `NFriend`
- Improve future-compatibility with server changes

### Fixed
- Fix #36 listFriends failing on 2.3.0 against 2.13.0 server

## [2.3.0] - [2020-02-29]
### Added
- support Windows 10 SDK
- support build for android on linux

### Fixed
- fix crash when response is received after NClient was deleted
- fix build with CMake 3.15+ on Windows 10

### Changed
- speedup build for Linux

## [2.2.4] - 2019-11-21
### Added
- install_deps.sh for linux to install dependencies

### Fixed
- fix uint16_t is too small for our match data (#25)
- fix getUsername and getUserId in session wrapper
- fix build tests if BUILD_C_API is off

## [2.2.3] - 2019-11-13
### Added
- C API.
- Support iOS SDK 13.

### Fixed
- RPC call fails when body is empty.
- RPC payload differences between Rest and RT clients.
- optional_HAVE_INITIALIZER_LIST is undefined.

### Changed
- Release for Ubuntu 18.04 x64 only (previously was Ubuntu 14.04.5 x86, x64).

## [2.2.0] - 2019-09-14
### Added
- Support server 2.7

### Changed
- Following client API has been added/changed to support server 2.7:
  - NSessionInterface
    - getVariables
    - getVariable
  - NClientInterface
    - authenticateDevice
    - authenticateEmail
    - authenticateFacebook
    - authenticateGoogle
    - authenticateGameCenter
    - authenticateCustom
    - authenticateSteam
    - listFriends
    - createGroup
    - listGroupUsers
    - listUserGroups
  - NRtClientInterface
    - joinMatch
- Updated optional lib to v.3.2.0

## [2.1.0] - 2019-06-05
### Added
- REST client (HTTP/1.1).
- Add tournament start active time to the API response.
- Add "Activity timeout" to Websocket.

### Changed
- Now we use one C++ REST library for all platforms instead of Websocketpp and IXWebsocket.
- gRPC client is off.

### Fixed
- Fixed loading dynamic library on Mac.

## [2.0.3] - 2019-05-01
### Added
- support Visual Studio 2019.
- support build as shared lib (DLL)

### Fixed
- enable SSL for IXWebsocket.

## [2.0.2] - 2019-04-12
### Added
- SSL support.
- Added IXWebsocket 1.3.1 for Mac, iOS and Android.
- Client and Realtime Client different host/ssl config.
- Added enums for storage permissions.

### Changed
- Improved NConsoleLogSink for Windows.
- Updated grpc to 1.19.1.

### Fixed
- Fixed listUserGroups for own user.
- Fixed: success callback is not called for methods with empty response.
- Propagate transport error to realtime client listener.
- Validation of user presences in sendMatchData.
- Xcode warning on required version.

## [2.0.1] - 2019-03-04
### Added
- Initial public release.
- Current limitation - No SSL support
