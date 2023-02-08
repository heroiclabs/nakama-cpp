#!/bin/bash

adb -s "$0" uninstall com.heroiclabs.nakamatest
adb is "$0" install ./build/outputs/apk/debug/nakamatest-debug.apk
adb logcat -c
adb shell am start -a android.intent.action.MAIN -n com.heroiclabs.nakamatest/android.app.NativeActivity
adb logcat