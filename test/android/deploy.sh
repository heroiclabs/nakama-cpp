#!/bin/bash

adb -s "$1" uninstall com.heroiclabs.nakamatest
adb -s "$1" install ./build/outputs/apk/debug/nakamatest-debug.apk
adb logcat -c
adb shell am start -n com.heroiclabs.nakamatest/.MainActivity
adb logcat