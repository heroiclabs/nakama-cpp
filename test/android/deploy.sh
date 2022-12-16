#!/bin/bash

./gradlew assemble -PandroidABI=arm64-v8a
adb install ./build/outputs/apk/debug/nakamatest-debug.apk
adb logcat -c
adb shell am start -a android.intent.action.MAIN -n com.heroiclabs.nakamatest/android.app.NativeActivity
adb logcat