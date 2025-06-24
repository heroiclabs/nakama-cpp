import? 'submodules/private/JUSTFILE'

[private]
default:
  @just --list

build-win-x64 config="MinSizeRel":
  rm -rf out/win-x64
  cmake --preset=win-x64 -DBUILD_EXAMPLES=OFF
  cmake --build build/win-x64 --target clean
  cmake --build build/win-x64 --target install --config {{config}} --verbose


