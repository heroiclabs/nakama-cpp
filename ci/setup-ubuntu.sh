#!/bin/bash
set -uex

install_ninja() {
  (
    mkdir -p ~/bin
    cd /tmp
    curl -L -O https://github.com/ninja-build/ninja/releases/download/v1.11.0/ninja-linux.zip
    unzip ninja-linux.zip
    mv ninja /usr/local/bin/
  )
  ninja --version 2>/dev/null
}

install_gcc_11() {
  eatmydata add-apt-repository --yes ppa:ubuntu-toolchain-r/test
  eatmydata apt update && apt install --no-install-recommends -y g++-11 python3-distutils git pkg-config curl zip unzip tar make
}

# vcpkg wants mono, make sure it finds one
# see: https://github.com/microsoft/vcpkg/issues/25585
ensure_mono() {
  type -p mono || {
     # mono not found, lets try dotnet
     type -p dotnet || { echo "ERROR: neither mono nor dotnet found"; exit 1; }
     ln -s "$(type -p dotnet)" ~/bin/
  }
  mono --version >/dev/null
}

setup_vcpkg() {
  ensure_mono
  eatmydata ./cmake/vcpkg/bootstrap-vcpkg.sh
}


if [[ -z "$@" ]]; then
  # default mode, do regular setup
  (( $(gcc -dumpversion) >= 11 )) || install_gcc_11
  ninja --version 2>/dev/null || install_ninja
  setup_vcpkg
else 
  # allow caller to cherry-pick what exactly to do
  for arg in "$@"; do
    $arg
  done
fi
