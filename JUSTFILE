import? 'submodules/private/JUSTFILE'

[private]
default:
  @just --list

build-win-x64 config="MinSizeRel":
  rm -rf out/win-x64
  cmake --preset=win-x64 -DBUILD_EXAMPLES=OFF
  cmake --build build/win-x64 --target install --config {{config}} --verbose

build-ios-arm64 config="MinSizeRel":
  rm -rf out/ios-arm64
  cmake --preset=ios-arm64 -DBUILD_EXAMPLES=OFF -DBUILD_TESTING=OFF
  cmake --build build/ios-arm64 --target install --config {{config}} --verbose
