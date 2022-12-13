#!/bin/bash
set -uex

brew install ninja
brew install --cask android-ndk
ANDROID_NDK_HOME="$(brew --prefix)/share/android-ndk"
export ANDROID_NDK_HOME