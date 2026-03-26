#!/bin/bash
set -eo pipefail

: "${ABI:?Set ABI}"
: "${CMAKE_PRESET:?Set CMAKE_PRESET}"
: "${VCPKG_ROOT:?Set VCPKG_ROOT}"
: "${ANDROID_NDK_HOME:?Set ANDROID_NDK_HOME}"

echo "=== Building C++ Test Library ==="
BUILD_TYPE="${BUILD_TYPE:-MinSizeRel}"
time cmake --preset "$CMAKE_PRESET" \
  -DBUILD_TESTING=ON \
  -DCMAKE_ANDROID_NDK="$ANDROID_NDK_HOME" \
  -DCMAKE_ANDROID_ARCH_ABI="$ABI" \
  -DCMAKE_MAKE_PROGRAM="$(command -v ninja)"

cmake --build "build/$CMAKE_PRESET" --config "$BUILD_TYPE" --target nakama-test

# Build the official SDK AAR first — it must exist before we extract its .so below.
echo "=== Building Official Android SDK (Release) ==="
cd android
time ./gradlew assembleRelease -Pabi="$ABI" --no-daemon
cd ..

jni_dir="integrationtests/android/jniLibs/$ABI"
mkdir -p "$jni_dir"

# Stage the test runner from the CMake build.
cp "build/$CMAKE_PRESET/integrationtests/$BUILD_TYPE/libnakama-test.so" "$jni_dir/"

# Stage the SDK .so from the Gradle AAR, NOT from the CMake build.
# The Gradle build passes flags that the direct cmake --preset path does not:
#   -DANDROID_SUPPORT_FLEXIBLE_PAGE_SIZES=ON  (16 KB page alignment, required on Android 15+)
#   -DANDROID_STL=c++_shared
#   -DINSIDE_GRADLE=ON  (vcpkg toolchain interposition workaround)
# Using the AAR's .so ensures the tested binary matches what ships to users.
AAR_PATH="android/nakama-sdk/build/outputs/aar/nakama-sdk-release.aar"
unzip -jo "$AAR_PATH" "jni/$ABI/libnakama-sdk.so" -d "$jni_dir/"

case "$ABI" in
  arm64-v8a)   triple="aarch64-linux-android";;
  armeabi-v7a) triple="arm-linux-androideabi";;
  x86_64)      triple="x86_64-linux-android";;
esac

HOST_OS=$(uname -s | tr '[:upper:]' '[:lower:]')
stl_lib="$ANDROID_NDK_HOME/toolchains/llvm/prebuilt/${HOST_OS}-x86_64/sysroot/usr/lib/$triple/libc++_shared.so"
if [ -f "$stl_lib" ]; then
  cp "$stl_lib" "$jni_dir/"
fi

echo "=== Building Integration Tests ==="
cd integrationtests/android
time ./gradlew assembleCustomDebugType -Pabi="$ABI" --no-daemon
cd ../../


 # --- Resolve device serial ---
ADB="${ADB:-adb}"
PACKAGE="com.heroiclabs.nakamatest"
        ACTIVITY="${PACKAGE}/.MainActivity"
        TIMEOUT=900
        LOG_TAG="nakama"
        APK_PATH=$(ls integrationtests/android/build/outputs/apk/customDebugType/*.apk 2>/dev/null | head -n 1)

        parse_devices() {
          devices=()
          local output
          output=$("$ADB" devices)
          while IFS= read -r line; do
            if [[ "$line" == *$'\t'device ]]; then
              devices+=("${line%%$'\t'*}")
            fi
          done <<< "$output"
        }

        serial="${DEVICE}"
        if [ -z "$serial" ]; then
          parse_devices

          if [ ${#devices[@]} -eq 0 ]; then
            echo "No connected devices/emulators found. Starting emulator..."
            EMULATOR="$ANDROID_HOME/emulator/emulator"
            if [ ! -x "$EMULATOR" ] && [ -x "${EMULATOR}.exe" ]; then
              EMULATOR="${EMULATOR}.exe"
            fi
            avd=$("$EMULATOR" -list-avds)
            avd="${avd%%$'\n'*}"
            avd="${avd//$'\r'/}"
            if [ -z "$avd" ]; then
              echo "Error: No AVDs found. Create one in Android Studio first."
              exit 1
            fi
            echo "Starting AVD: $avd"
            {{if eq OS "windows"}}powershell.exe -Command "Start-Process -FilePath '${EMULATOR}' -ArgumentList '-avd','${avd}' -WindowStyle Hidden"{{else}}"$EMULATOR" -avd "$avd" &>/dev/null &{{end}}
            echo "Waiting for device to come online..."
            "$ADB" wait-for-device
            while [[ "$("$ADB" shell getprop sys.boot_completed 2>/dev/null)" != *"1"* ]]; do
              sleep 2
            done
            echo "Emulator booted."
            parse_devices
          fi

          if [ ${#devices[@]} -gt 1 ]; then
            echo "Error: Multiple devices connected. Specify DEVICE=<serial>:"
            "$ADB" devices
            exit 1
          fi
          serial="${devices[0]}"
          echo "Auto-detected device: $serial"
        fi

        adb_cmd() {
          "$ADB" -s "$serial" "$@"
        }

        # --- Port forwarding ---
        echo "Setting up port forwarding..."
        for port in 7349 7350 7351; do
          adb_cmd reverse tcp:$port tcp:$port
        done

        # --- Install APK ---
        if [ ! -f "$APK_PATH" ]; then
          echo "Error: APK not found at $APK_PATH"
          exit 1
        fi

        echo "Installing APK..."
        adb_cmd uninstall "$PACKAGE" 2>/dev/null || true
        adb_cmd install -r "$APK_PATH"

        # --- Clear logcat and launch ---
        adb_cmd logcat -c
        echo "Launching $PACKAGE via monkey..."
        adb_cmd shell monkey -p "$PACKAGE" -c android.intent.category.LAUNCHER 1

        # --- Monitor logcat for test results ---
        echo "Waiting for test results (timeout: ${TIMEOUT}s)..."
        echo "---"

        logfile="logcat-output.tmp"
        > "$logfile"
        "$ADB" -s "$serial" logcat "libnakama-test:V" "nakama:V" "AndroidRuntime:E" "ActivityManager:E" "*:S" -v raw > "$logfile" 2>/dev/null &
        logcat_pid=$!
        trap 'rm -f "$logfile"; kill $logcat_pid 2>/dev/null || true' EXIT

        result=""
        SECONDS=0
        last_line=0

        while [ "$SECONDS" -lt "$TIMEOUT" ]; do
          line_num=0
          while IFS= read -r line; do
            line_num=$((line_num + 1))
            if [ "$line_num" -le "$last_line" ]; then
              continue
            fi
            echo "$line"
            if [[ "$line" == *"Tests failed: 0"* ]]; then
              result="passed"
            elif [[ "$line" == *"Tests failed:"* ]]; then
              result="failed"
            fi
          done < "$logfile"
          last_line=$line_num
          [ -n "$result" ] && break
          sleep 1
        done

        kill $logcat_pid 2>/dev/null || true

        echo "---"

        if [ -z "$result" ]; then
          echo "TIMEOUT: Tests did not complete within ${TIMEOUT}s."
          exit 1
        elif [ "$result" = "passed" ]; then
          echo "ALL TESTS PASSED"
          exit 0
        else
          echo "TESTS FAILED"
          exit 1
        fi



